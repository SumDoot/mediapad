#include <Adafruit_NeoPixel.h>

#include <EZButton.h>
#include <Keyboard.h>

// --------------------
// Pins
// --------------------
#define ENC_A 2
#define ENC_B 1

#define ENC_SW 7

#define MUTE 3
#define PREV 4
#define NEXT 5
#define MENU 6

#define LED_PIN   14
#define LED_COUNT 4
#define LED_CDIFF 2048

volatile int32_t encoderPosition = 0;
volatile int8_t  lastEncoded     = 0;
volatile int8_t  pulseCount      = 0;  // Tracks sub-detent pulses

static const int8_t TRANSITION_TABLE[16] = {
    0, -1, +1,  0,
   +1,  0,  0, -1,
   -1,  0,  0, +1,
    0, +1, -1,  0,
};

void encoderISR() {
    uint8_t a = digitalRead(ENC_A);
    uint8_t b = digitalRead(ENC_B);

    int8_t currentEncoded = (a << 1) | b;
    int8_t index          = (lastEncoded << 2) | currentEncoded;
    int8_t delta          = TRANSITION_TABLE[index];
    lastEncoded           = currentEncoded;

    if (delta == 0) return;  // Glitch, ignore

    pulseCount += delta;

    // Only register a full detent (4 pulses in the same direction)
    if (pulseCount >= 4) {
        encoderPosition++;
        pulseCount = 0;
    } else if (pulseCount <= -4) {
        encoderPosition--;
        pulseCount = 0;
    }
    // Partial movements (1-3 pulses) are held in pulseCount and only
    // count if they continue in the same direction to complete a detent
}

void ReadButtons(bool *states, int num) {
  states[0] = !digitalRead(MUTE);
  states[1] = !digitalRead(PREV);
  states[2] = !digitalRead(NEXT);
  states[3] = !digitalRead(MENU);
  states[4] = !digitalRead(ENC_SW);
}

EZButton buttons(5, ReadButtons, 0, 0, 500);
Adafruit_NeoPixel strip(LED_COUNT , LED_PIN, NEO_GRB + NEO_KHZ800);

int ledHSV = 0;
uint8_t saturation[4] = {255, 255, 255, 255};

void ledLoop(){
  for(int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(ledHSV + (LED_CDIFF * i), saturation[i])));

    if(saturation[i] < 255) saturation[i]++;
  }
  strip.show();

  ledHSV += 64;
  if(ledHSV >= 65536) ledHSV = 0;
}

void sendMedia(uint16_t key) {
  Keyboard.consumerPress(key);
  Keyboard.consumerRelease();
}

void encoderLoop() {
  noInterrupts();
  int32_t pos = encoderPosition;
  encoderPosition = 0;
  interrupts();

  // Now work entirely off the local snapshot — ISR is free to run again
  if (pos != 0) {
    Serial.println(pos);
    
    uint16_t key = pos > 0 ? KEY_VOLUME_INCREMENT : KEY_VOLUME_DECREMENT;
    for (int i = 0; i < abs(pos); i++) {
      sendMedia(key);
    }
  }
}

// --------------------
// Setup
// --------------------
void setup() {
  Serial.begin(115200);
  

  // USB keyboard + media
  Keyboard.begin();

  // Pins
  pinMode(MUTE, INPUT_PULLUP);
  pinMode(NEXT, INPUT_PULLUP);
  pinMode(PREV, INPUT_PULLUP);
  pinMode(MENU, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  // Read initial state so the first interrupt isn't treated as movement
  lastEncoded = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);

  attachInterrupt(digitalPinToInterrupt(ENC_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), encoderISR, CHANGE);

  // Buttons
  buttons.Subscribe(0, [](){
    saturation[0] = 0;
    sendMedia(KEY_MUTE);
  }, PRESSED);

  buttons.Subscribe(1, [](){
    saturation[1] = 0;
    sendMedia(KEY_SCAN_PREVIOUS);
  }, PRESSED);

  buttons.Subscribe(2, [](){
    saturation[2] = 0;
    sendMedia(KEY_SCAN_NEXT);
  }, PRESSED);

  buttons.Subscribe(3, [](){
    saturation[3] = 0;
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.print("v");
    Keyboard.releaseAll();
  }, PRESSED);

  buttons.Subscribe(4, [](){
    sendMedia(KEY_PLAY_PAUSE);
  }, PRESSED);

  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'
}

// --------------------
// Loop
// --------------------
void loop() {
  buttons.Loop();
  encoderLoop();
  ledLoop();
  delay(5);
}




