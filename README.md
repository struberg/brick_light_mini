# Lightning system for game bricks

This is a game brick compatible lightning solution to control 4 LEDs.
The repo contains the controller box source code, schematic and PCB plus all the 3d printer models to create your own enclousure and lamps.
The PCB is deliberately single sided and can easily be etched by yourself with Natriumpersulfate.
Ofc if someone likes to create a double sided PCB to be made at Aisler etc then don't hesitate to ship a PR.

The schematic is based on a 0-series ATtiny-402, but a 202 will likely do fine as well.
A rechargable LiIon LIR2450 acts as power source.
The 2450 size is a good compromise between juice and size. 
Connectors are simple 2x1 Dupont types.

The only somewhat non-jellybean part is the 4x220R 0603 resistor array. 
I had to use it because 4 separate resistors would have been too big to fit on the PCB.
You can get those for very cheap from aliexpress or mouser though.

## How to connect the LEDs

You can use any standard low power white or colored LEDs.
The maximum output per LED is around 15 mA.

Place the controler box so that the button is on the lower left side and the LED connectors are on the left upper side.
The top row of the pins is all ground and internally connected.
The lower row of the pins are the positive LED pins, starting with 1 on the left side.

```
  x  x  x  x
  |  |  |  | 
  1  2  3  4  
  |  |  |  |
  x  x  x  x
```
## How to use

* single click: switch LED 1 on or off
* double click: switch LED 2 on or off
* triple click: switch LED 3 on or off
* quadruple click: switch LED 4 on or off

 

The device will switch off all LEDS and go to sleep after around 5 Minutes of not using the button.
This prevents emptying the battery when one forgets to turn off all lights.
The sleep current is in the µA range, I measured from 0.2µA to 1.5µA
The device will turn on again simply by pressing the button.

Have fun! 
