# AVRNude Multiplexer

I abandoned the multiplexer because it was just simpler to use a USB hub and
have multiple devices for my various programmers.

But whilst I was just looking to program pic32 and AVR using a pair of
ATMega328s as programmers, a simple 2-way multiplexer seemed like a good idea.

![Multiplexer circuit](https://i.imgur.com/iPLBmtB.jpg)

Simply connecting the USB/Serial adaptor to one end of the muliplexer and the
two programmers to the other and using an unused LED on the router as a GPIO
to control the switching through a MOSFET(2N7000) 3v3 -> 5V level shifter.

```bash
# Gives about 3.3 volts on unpopulated WPS LED
echo 1 > /sys/class/leds/F@ST2704N:green\:wps/brightness
```

![Photo 1](https://i.imgur.com/gGkBz8v.jpg)
![Photo 2](https://i.imgur.com/l6Rhc5P.jpg)
