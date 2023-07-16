# AVRNude

AVRNude is a networked server for
[AVRDude](https://github.com/avrdudes/avrdude)
based on
[OpenWRT](https://openwrt.org/)
and including an
[stk500v1 programmer](https://github.com/arduino/arduino-examples/blob/main/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino)
for AVRs and (currently in development) an
[ascii ICSP programmer](https://github.com/sergev/pic32prog/tree/master/bitbang)
for PIC32s

## Hardware

### Router

It's based on an old cheap ethernet router from E-Bay. Almost anything will
do as long as it has a USB port and is capable of running
[OpenWRT](https://openwrt.org/toh/start)

### ATMega328 - AVR/PIC32 Programmer

![AVRNude Programmer](https://i.imgur.com/vZm1doT.jpg)

AVRs and PIC32s are programmed through a DIY minimal "Arduino" board with a
CH340G USB to serial chip (as used on the Arduino Nano).

This also has a few extra LEDs on it as used by the
[Arduino ISP sketch](https://github.com/arduino/arduino-examples/blob/main/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino).

## Software

How to get OpenWRT on the router in the first place varies with what model of
router you're using... see
[The OpenWRT Table of Hardware](https://openwrt.org/toh/start) for more
details.

The programmer is made available on the network using
[ser2net](https://github.com/cminyard/ser2net)

### OpenWRT Configuration

I've added a few extra packages to the OpenWRT running on the old router:

```sh
opkg update
opkg install
    terminfo libncurses nano \
    ser2net \
    librt libusb-1.0 usbutils \
    kmod-usb-serial \
    kmod-usb-serial-cp210x kmod-usb-serial-ch341
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

The baud rate given is the default used by `avrdude`.

> Robert Rozee's programmer code for `pic32prog` runs at 115200,
> which might case some compatibility issues between the two.
> TODO: check running `avrdude` at 115200.
> [Issue 1](https://github.com/andy-preston/avrnude/issues/1)

### Build Machine Configuration

In the build script for my
[GPO 746 Telephone Linked to Android](https://github.com/andy-preston/gpo-746-android)
project, I call `avrdude` with:

```sh
avrdude -c stk500v1 -P net:avrnude.lan:5000 -p t2313 -V ...
```

Where `avrnude.lan` is the hostname of the old router
with the programmer attached.

> TODO: `pic32prog` doesn't have any facility to connect to networked devices.
> Can the network code from `avrdude` be factored into it?
> [Issue 2](https://github.com/andy-preston/avrnude/issues/2)

## Expansion / Future Plans

### Serial Debug Monitor

This is still my old
["pre-ser2net" version](https://github.com/andy-preston/avrnude/blob/master/serdebug.md)
it needs updating.

### STM32 and The Black Magic Probe

Still on the "do it one day" list.

The
[Black Magic Probe](https://andy-preston.github.io/blue-pill-black-magic-probe.html)
is already ready though.
