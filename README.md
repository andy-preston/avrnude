AVRNude started life as a tool to enable me to program AVR devices over my lab
network. But it's now gone beyond AVR and I'm expanding it to include STM-32
based devices and SPI EEPROMS.

Some sketchy details here may also mention PIC32 and Parallax Propeller based
devices, but this support was scrapped before completion.

# Hardware

## Router

It's based on an old cheap ethernet router from E-Bay. Almost anything will
do but it does need:

1. A USB port (or, in my case, some pads you can solder a USB port to.

2. To be able to run [OpenWRT](https://openwrt.org/toh/start) and have a
bit of flash left to install some more software

## Power concerns and wiring

![AVRNude Hardware](https://i.imgur.com/yTkyo8Z.jpg)

I don't think the small router PSU can handle all the extra hardware that's
going to be added. But I use an old ATX power supply to run a few things on
my test bench anyway and although the various components I've added share a
common ground, they each have their own supply coming from the ATX PSU.

The 5V line in the USB connection from the router to the hub is not connected.

The router has a 12V supply as per it's original specifications. If the router
needed (e.g.) 9V then a 7809 or similar could be used to provide that voltage.
(You can see a couple of these in the photo for other equipment).

The USB hub has it's own 5V supply.

## ATMega328 - AVR Programmer

AVRs are programmed through a DIY minimal "Arduino" board with no PSU or USB.
This also has a few extra LEDs on it as per the
[Arduino ISP sketch](https://github.com/arduino/arduino-examples/tree/main/examples/11.ArduinoISP).

It gets it's 5V supply from the ATX power supply and takes a serial input from
a USB/Serial adaptor mounted on the USB Hub.

## (Abandoned) Multiplexer.

My original version included a [Multiplexer](mux.md) but this has since
been abandoned and replaced with a 4-port USB Hub.

# Software

How to get OpenWRT on the router in the first place varies with what model of
router you're using... see
[The OpenWRT Table of Hardware](https://openwrt.org/toh/start) for more
details.

I needed various other packages to get up and running:
```bash
opkg update
opkg install usbutils
opkg install kmod-usb-serial kmod-usb-serial-ch341 kmod-usb-serial-cp210x \
             coreutils coreutils-stty terminfo usbutils zlib \
             libelf1 libftdi1 libncurses libreadline librt \
             libusb-1.0 libusb-compat \
             nano screen avrdude stm32flash
```

## USB Serial Adaptors

As well as CH341, I also used some CP2102 USB/Serial Adaptors which can be
programmed to give a unique ID: [CP2102 based adaptors](cp2102.md)

Currently, I've no udev on the version of OpenWRT and I haven't had as
much success with "raw-hotplug" as I would have liked. I still haven't
completed implementing a script in
[/etc/hotplug.d/usb/99-programmers](99-programmers)

## Individual Programmers

+ [AVRNude script](avrnude.sh)
+ [Serial debug monitor](serdebug.sh) - [Notes](serdebug.md)
+ [Abandoned PIC32 Programmer](pic32.md)

# TODO:

Serial programmer for STM32 "Blue pill" and "Black pill" boards

Programmer for I2S EEPROMS (Mostly so I can
easily program the Parallax Propeller - but I might want to read write EEPROMS for other purposes later)

## Notes for further work:

+ fixing usb - usb serial adaptor with dtr and cts
+ the serial sniffer
+ i2c eeprom programmer using attiny 2313 - needs DTR
+ stm32 serial programmer - needs cts (dtr would be good but my black pill doesn't support it)
+ programming the attiny2313 for i2c eeproms
+ an upload script for eeproms.
+ an upload script for the stm 32.
