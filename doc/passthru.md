Active low signal passthrough
=============================

The firmware provides a way to connect to an ESC on pin C3 through pin D1 (SWIM) by translating active low signal between the two pins. In other words, if you need to tweak settings or update firmware in your ESC after you have assembled your model, and the only hook-up available is the SWIM, you can still gain access to the ESC by flashing this stub firmware and connecting to the SWIM pin.


Installation
------------

```
cmake -B build   # Add -D PROG=type to override programmer type
cd build
make
make flash-passthru
```
