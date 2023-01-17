LESU Skid Steer Loader
======================

![](/img/lesu1.jpg)

Check out this video: https://www.youtube.com/watch?v=e72fbO8cWV4


Firmware features
-----------------

+ Hydraulic pump control
+ Dual motor control (tracks)
+ Acceleration ramping
+ LED lighting (headlights, tail light, blinkers, reverse)
+ iBUS servo link with FlySky transmitter
+ iBUS telemetry (voltage, temperature)

An advanced version of this firmware can be found here: https://github.com/neoxic/STM32F0


Rationale
---------

The firmware controls response of the pump depending on input on channels 1, 2 and 5. The main goal is to make the operation smooth and fluid comparing to a simple channel mix in the transmitter. The conventional approach does not take into account the fact that there is some travel distance before a valve begins to open at `VALVE_MIN`. In this model, there is no practical need to account for half-opened state because the valves open quickly. A better strategy is to keep the pump off until a valve is open and then linearly increase output. As a bonus, there is no need to create a channel mix on the transmitter for the pump.

![](/img/pump2.jpg)

The track motors are controlled by the firmware in a differential fashion, i.e. channel 3 is forward/reverse and channel 4 is left/right. This makes it possible to use two independent ESCs (or a dual one working in independent mode) for the tracks without having to configure a channel mix on the transmitter.

The blinkers indicate the direction of turning while the model is moving forward or turning around. Additionally, there are two forced blinking modes (emergency and strobe) that are controlled by a 3-way switch on channel 7.


Pinout
------

| Pin | I/O | Description       |
|-----|-----|-------------------|
| D6  | IN  | iBUS servo        |
| D5  | I/O | iBUS sens / DEBUG |
| C6  | OUT | Left track        |
| C7  | OUT | Right track       |
| C3  | OUT | Pump              |
| C4  | OUT | Light             |
| C5  | OUT | Left blinker (*)  |
| A3  | OUT | Right blinker (*) |
| D4  | OUT | Reverse (*)       |
| D2  | AIN | Temperature       |
| D3  | AIN | Voltage           |
| B5  | OUT | Status LED (*)    |

(*) active low


Channel mapping
---------------

| # | Action           |
|---|------------------|
| 1 | Bucket           |
| 2 | Lift arm         |
| 3 | Forward/reverse  |
| 4 | Left/right       |
| 5 | Tool             |
| 6 | Light on/off     |
| 7 | Blinkers 1/2/off |


Installation
------------

```
cmake -B build    # Add -D PROG=type to override programmer type
cd build
make
make flash-opts   # Required only once
make flash-lesu
```


Upgrading the model
-------------------

I ended up replacing the provided ESCs and light/sound controller because the brushed ESCs were terribly loud and rated only 9.4V (LESU claimed they were suitable for a 3S battery for some weird reason), the brushless HobbyKing ESC was simply not suitable for a pump due to an unavoidable 300ms startup delay, and the sound controller was largely useless for me anyway. The list of the components that I used:

* FlySky FS-i6S transmitter with FS-iA10B receiver (iBUS servo/telemetry)
* Sabertooth 2x12 motor driver (tracks)
* SPEDIX 30A ESC (pump)
* 3 unprotected 18650 cells as a 3S battery pack

The receiver was trimmed off of all the housing and header pins in order to fit in the top shelve in the cab. The Sabertooth 2x12 motor driver was also stripped off of the heat sink, capacitors and headers and trimmed off on the sides in order to be placed atop the motors. Note that you can use any track drivers (or a [dual one](https://github.com/neoxic/STM8-RC-Dual-Motor-Driver) working in independent mode) and/or ESCs with this firmware that suit your needs.

![](/img/lesu2.jpg)

You might also want to replace the track motor leads and solder one 0.1uF ceramic capacitor from each terminal to the motor case to protect the internals from electrical noise since the motors are **very** noisy. I even had one of them replaced by the seller because it would otherwise constantly trip over-current protection of the driver due to the very bad commutator design that causes lots of shorting. As an additional measure to counter this problem, I ground a tiny layer off the brushes to make them a bit thinner.

The SPEDIX 30A ESC is snugly located in the cavity underneath the valve servos with its heat sink facing down. The pump motor leads are soldered to it. The servos are connected to PWM channels 1, 2 and 5 of the receiver. The STM8S103F3 board is located to the left while the 470mF capacitor is located to the right of the driver. The track motor leads are soldered to the bottom of the driver to reduce overall thickness of the front part and to help it fit snugly.

![](/img/lesu3.jpg)

If you intend to use a BlHeli ESC like I did, plug it into the receiver directly and calibrate throttle after a startup beep by moving it first to the maximum and then to the minimum endpoint. When you are done, get into BlHeliSuite and switch _"Motor Direction"_ to _"Bidirectional"_ (or _"Bidirectional Rev."_ should the motor direction be reversed). Additionally, set _"Startup Power"_ to _"1.00"_, turn off _"Low RPM Power Protect"_ and _"Demag Compensation"_, and set _"Beacon Delay"_ to _"Infinite"_. Take a look at the settings I ended up with.

In order to be able to change BlHeli settings later when the model was fully assembled, I wrote a separate piece of stub firmware called [active low signal passthrough](passthru.md) that allows to configure a BlHeli ESC via the SWIM pin.

![](/img/blheli1.png)

The temperature sensor is a TMP36 sensor. The voltage sensor is a simple voltage divider with R1=100K and R2=10K (you can pick your own values). Measure voltage on the VDD pin along with the exact values of R1 and R2 with a multimeter and update the integer constants `VOLT1` and `VOLT2` at the beginning of `src/lesu.c`. `VOLT1` must be in millivolts whereas `VOLT2` must be constructed by calculating the formula `VOLT1*(R1+R2)/R2` and then converting the value to tens of millivolts.

The LEDs on the schematics denote LED connection points rather than the actual number of diodes. For example, there are two headlights in this model.

![](/img/circuit2.png)

Flash the board and give it a run. You should see two telemetry sensors on your transmitter.

![](/img/telemetry1.jpg)

The voltage sensor can be used to set up the battery charge monitor on the transmitter by tapping on the battery indicators, ticking the _"Ext"_ checkbox and updating the _"High/Ala./Low"_ values.

![](/img/telemetry2.jpg)
