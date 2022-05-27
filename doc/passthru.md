Active low signal passthrough
=============================

The firmware provides a way to configure a BlHeli ESC connected to pin C3 through pin D1 (SWIM) by translating active low signal between the two pins. In other words, if you need to tweak a couple settings in your BlHeli ESC after you have assembled your model, and the only hook-up available is the SWIM, you can still gain access to the ESC by flashing this stub firmware and connecting your BlHeli linker to the SWIM pin.


Installation
------------

```
cmake -B build   # Add -D PROG=type to override programmer type
cd build
make
make flash-passthru
```
