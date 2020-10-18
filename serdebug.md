An AVR is transmitting debugging data over a UART line
to a serial USB adaptor (CP2102)
being monitored / logged on a PC over ssh.

[Script](serdebug.sh)

If you're having problems with your `stty` settings (I did)
`-icanon` was the "magic incantation" that made all the difference for me.

Also, bear in mind that you could well be sending the wrong number of
data bits of the AVR side.

On an atTiny2313 at least, the default is 5 bits and you need to
```c
UCSRC = (1 << UCSZ1) | (1 << UCSZ0)
```
to get it to the more usual 8 data bits
(1 stop bit, no parity are the other defaults and that's fine)
