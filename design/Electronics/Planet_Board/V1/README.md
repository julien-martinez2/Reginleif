I chose not to put mounting holes on this PCB to save as much space as possible since each one (M2) takes about 5mm on the edge of the PCB, so adding one on each side is 10mm added, quite a lot for a 16mm-wide PCB.
However, I failed to envision that since the holes allow me to place the PCB more easily in the structure, it makes it easier to design.

Same for the LEDs, I could have added one for the 3.3V, one for the VCC and one for NFAULT, but chose not to to save space.
But when I started testing, I immediately understood that not having an LED to indicate the PCB is correctly powered is annoying.

The design was also planned to be as modular as possible, but since the PCB will sit between two motors shafts, same as the angle sensors, it forces me to put them on different mechanical layers.
Putting 2 sensors directly on this board will reduce the modularity but simplify the mechanical structure by quite a lot. It could even allow me to put all the components on a single side since their number is reduced and the surface is more important.

Next, I didn't add much text on the silkscreen, again to save space, thinking that I could just refer to the drawings anyway.
Well, it works but definitively takes more time. Since it doesn't takes that much space, it would be a big improvement.

Finally, all the degradations mainly come from trying to save space, but at the end I probably have quite a lot of margin, since this PCB lenght is 48.5mm and the space between my motors shaft is about 80mm.

TODO in the next version : 
- Add mounting holes
- Add LEDs for quick debug
- Put two of the angle sensors on this board
- Comment more on silkscreen
