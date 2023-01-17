DOUBLE E HOBBY Volvo EC160E Excavator
=====================================

![](/img/volvo1.jpg)


Firmware features
-----------------

+ Hydraulic pump and valve control
+ Acceleration ramping
+ LED lighting (headlights, tail light)
+ iBUS servo link with FlySky transmitter
+ iBUS telemetry (voltage, temperature)

An advanced version of this firmware can be found here: https://github.com/neoxic/STM32F1


Rationale
---------

The firmware controls response of the pump depending on input on channels 1, 2 and 3. The main goal is to make the operation smooth and fluid comparing to a simple channel mix in the transmitter. The conventional approach does not take into account the fact that there is some travel distance before a valve begins to open at `VALVE_MIN`. In this model, a valve is not immediately fully open upon reaching that point, but rather upon reaching `VALVE_MAX` which takes some more travel. A better strategy is to keep the pump off until a valve is about to start opening and then give the input reduced weight until the valve is fully open. As a bonus, there is no need to create a channel mix on the transmitter for the pump.

![](/img/pump1.jpg)

Another feature is that the digital servos of the valves are also controlled by the firmware. The main reason for that is servo trimming. Due to their design, the valve servos are not centered mechanically in this model hence they must be trimmed. Consequently, the trimming values need to be further taken into account in the firmware for proper pump control. Since there is no point in keeping the same values in two places, they are stored in the firmware only. It simplifies the model's setup on the transmitter to a great extent. As another bonus, the pump/servo refresh rate is fixed at 250Hz.


Pinout
------

| Pin | I/O | Description       |
|-----|-----|-------------------|
| D6  | IN  | iBUS servo        |
| D5  | I/O | iBUS sens / DEBUG |
| C6  | OUT | Bucket valve      |
| C7  | OUT | Boom valve        |
| C3  | OUT | Stick valve       |
| C4  | OUT | Pump              |
| C5  | OUT | Light             |
| D2  | AIN | Temperature       |
| D3  | AIN | Voltage           |
| B5  | OUT | Status LED (*)    |

(*) active low


Channel mapping
---------------

| # | Action       |
|---|--------------|
| 1 | Bucket       |
| 2 | Boom         |
| 3 | Stick        |
| 7 | Light on/off |


Installation
------------

```
cmake -B build    # Add -D PROG=type to override programmer type
cd build
make
make flash-opts   # Required only once
make flash-volvo
```


Upgrading the model
-------------------

Make sure valve channels 1, 2 and 3 are properly centered via the _Function/Subtrims_ menu on the transmitter. Transfer the values to the corresponding `CHx_TRIM` constants at the beginning of `src/volvo.c`. Don't worry about the values being shown as percentages in the menu. They are actually in microseconds! For example, `Ch1 -50%` yields `CH1_TRIM -50` and so on. You can skip this step and center the channels later by trial and error and/or consulting with debug output.

Solder a 3-row header to the side of the board covering pins C3-D3 (8 pins). The outer row of the header pins must be connected to ground. The middle row provides power and is split into two regions: C3-D1 (ESCs/servos/light) - 6V, D2-D3 (sensors) - 3.3V. Make appropriate connections on the back of the board and solder iBUS leads with header connectors.

![](/img/volvo2.jpg)

![](/img/volvo3.jpg)

Fix the board on the receiver with a double-sided tape and make appropriate wire connections. Note that the stock 10-channel FS-iA10B receiver has been replaced with a 6-channel FS-iA6B in my case. Connect the voltage sensor to the main power supply and place the temperature sensor on the transparent low-pressure pipe.

![](/img/volvo4.jpg)

Note that channels 4, 5 and 6 are not controlled by the board; they are plugged directly into the receiver.

![](/img/volvo5.jpg)

![](/img/volvo6.jpg)

![](/img/volvo7.jpg)

The temperature sensor is a TMP36 sensor. The voltage sensor is a simple voltage divider with R1=100K and R2=10K (you can pick your own values). Measure voltage on the VDD pin along with the exact values of R1 and R2 with a multimeter and update the integer constants `VOLT1` and `VOLT2` at the beginning of `src/volvo.c`. `VOLT1` must be in millivolts whereas `VOLT2` must be constructed by calculating the formula `VOLT1*(R1+R2)/R2` and then converting the value to tens of millivolts.

The LEDs on the schematics denote LED connection points rather than the actual number of diodes. For example, there are three LEDs in this model (two headlights and one tail light).

![](/img/circuit1.png)

Flash the board and give it a run. You should see two telemetry sensors on your transmitter.

![](/img/telemetry1.jpg)

The voltage sensor can be used to set up the battery charge monitor on the transmitter by tapping on the battery indicators, ticking the _"Ext"_ checkbox and updating the _"High/Ala./Low"_ values.

![](/img/telemetry2.jpg)
