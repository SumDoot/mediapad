# Simple media control pad
My last keyboard had media controls but my current one only has them under a function layer, so i just built an external one.

It uses a rpi pico to send media commands and 4 buttons + a rotary encoder as inputs.

I added the controls i would be most likely to use:
 - volume up/down on encoder
 - play/pause on encoders button
 - next and previous song
 - mute
 - open windows sound panel

All the functions except for the last one just simply send the appropriate keyboard input.
The sound panel is opened via a combination of keys: win + ctrl + v
I think it only works for windows 11 but that may be wrong, could probably be set up anyway with a different method

I wrote and uploaded the program via Arduino IDE with <a href="https://github.com/earlephilhower/arduino-pico">Earle F. Philhower's pico core</a>.
Maybe not the most efficient or cleanest code but it works

There is also a rgb strip because why not? In my frame i have the LEDs placed behind thinner parts of the body so that it shines through and shows the symbol on a flat surface.

Bellow is a poorly made paint schematic that matches the pinout in the code

![alt text](https://github.com/SumDoot/mediapad/blob/main/AudioControllerLayout.png "Paint schematic")
