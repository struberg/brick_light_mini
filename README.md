# Lightning system for Bricks

This is a brick-system compatible lightning solution to control 4 LEDs.
The repo contains the controller box schematic + PCB plus all the 3d printer models to create your own.
The PCB is deliberately single sided and can easily be etched by yourself with Natriumpersulfate.
Ofc if someone likes to create a double sided PCB to made at Aisler etc then don't hesitate to ship a PR.

The schematic is based on a 0-series ATtiny-402, but a 202 will likely do fine as well.
A rechargable LiIon LIR2450 acts as power source.
The 2450 size is a good compromise between juice and size. 
Connectors are simple 2x1 Dupont types.

The only somewhat non jellybean part is the 4x220R 0603 resistor array. 
I had to use it because 4 separate resistors would have been too big to fit on the PCB.
You can get those for very cheap from aliexpress or mouser though.

Have fun! 
