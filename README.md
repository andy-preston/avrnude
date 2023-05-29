# AVRNude

AVRNude is a networked programmer for AVRDude.

It's old old network router re-flashed with OpenWRT and an stk500v1 compatible
programmer.

## Expansion / Future Plans

* Blue Pill / Black Magic Probe for an STM32 programmer
* ATTiny based programmer for SPI EEPROMS

## Hardware

### Router

It's based on an old cheap ethernet router from E-Bay. Almost anything will
do but it does need:

1. A USB port (or, in my case, some pads you can solder a USB port to.

2. To be able to run [OpenWRT](https://openwrt.org/toh/start) and have a
bit of flash left to install some more software

### USB ports and power concerns

![AVRNude Router](https://i.imgur.com/yTkyo8Z.jpg)

The router only has a single USB port and it's not up to delivering enough
power for the connected devices so I've got an externally powered USB hub to
handle both of these issues.

The router and the hub are both on the same (old ATX) PSU and so share a common
ground but the 5V line on the USB connection from the router is not connected.

The router has a 12V supply as per it's original specifications. If the router
needed (e.g.) 9V then a 7809 or similar could be used to provide that voltage.
(You can see a couple of these in the photo for other equipment).

### ATMega328 - AVR Programmer

![AVRNude Programmer](https://i.imgur.com/vZm1doT.jpg)

AVRs are programmed through a DIY minimal "Arduino" board with no PSU and a
CH340G USB to serial chip (as per the Arduino Nano).

This also has a few extra LEDs on it as per the
[Arduino ISP sketch](https://github.com/arduino/arduino-examples/blob/main/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino).

## Software

How to get OpenWRT on the router in the first place varies with what model of
router you're using... see
[The OpenWRT Table of Hardware](https://openwrt.org/toh/start) for more
details.

In the initial versions I was using `ssh`, a bunch of scripts and a copy of
`avrdude` actually installed on the router.

I've recently seen how things can be much simpler with `ser2net` as detailed
below.

### Build Machine Configuration

In the build script for my
[GPO 746 Telephone Linked to Android](https://github.com/andy-preston/gpo-746-android)
project, I call `avrdude` with:

```sh
avrdude -c stk500v1 -P net:avrnude.lan:5000 -p t2313 -V ...
```

Where `avrnude.lan` is the hostname of the old router
with the programmer attached.

### OpenWRT Configuration

I've added a few extra packages to the OpenWRT running on the old router:

```sh
opkg update
opkg install
    terminfo libncurses nano \
    ser2net \
    librt libusb-1.0 usbutils \
    kmod-usb-serial kmod-usb-serial-cp210x kmod-usb-serial-ch341
```

* `nano` is just to make editing config files easier.
* `ser2net` is to handle our network communication (see below)
* `usbutils` might come in handy to do `lsusb` etc.
* Kernel modules handle the various USB/Serial chips I'm playing with

### `ser2net` Configuration

In `/etc/config/ser2net` I've setup the following:

```text
config proxy
    option enabled 1
    option port 5000
    option protocol raw
    option timeout 0
    option device '/dev/ttyUSB0'
    option baudrate 19200
    option databits 8
    option parity 'none'
    option stopbits 1
```

## Beyond AVR - STM32 and The Black Magic Probe

Still on the "do it one day" list. The
[Black Magic Probe](https://andy-preston.github.io/blue-pill-black-magic-probe.html)
is already ready though.

## Serial Debug Monitor

This is still my old "pre-ser2net" version... updates to follow.

[Serial debug monitor](scripts/serdebug.sh)
[Notes](serdebug.md)

## USB Serial Adaptors

As well as CH340G in the programmer, I've  also got plans for a
CP2102 which can be programmed to give a unique ID:
[CP2102 based adaptors](cp2102.md)

Currently, I've no udev on the version of OpenWRT and I haven't had as
much success with "raw-hotplug" as I would have liked. I still haven't
completed implementing a script in
[/etc/hotplug.d/usb/99-programmers](99-programmers)

## Notes for further work:

* fixing usb - usb serial adaptor with dtr and cts
* the serial sniffer
* i2c eeprom programmer using attiny 2313 - needs DTR
* stm32 serial programmer - needs cts (dtr would be good but my black pill doesn't support it)
* programming the attiny2313 for i2c eeproms
* an upload script for eeproms.
* an upload script for the stm 32.
