#!/bin/bash

SETTINGS='9600 cs8 -cstopb -parenb -parodd -icanon'

ssh root@avrnude \
    "stty -F /dev/ttyUSB1 $SETTINGS ; cat /dev/ttyUSB1" | \
    tee logfile.txt | hexdump -C
