#!/bin/bash

# usage:
#    avrnude m324p plop.hex - to upload a hex file to an AVR device
#    avrnude m324p FF DF E2 - to set AVR fuses

prefix="avrdude -P /dev/ttyUSB0 -c stk500v1 -b 19200 -p $1"

if [ -f $2 ]
then
    # This leads to more network traffic but no remote storage
    for op in 'w' 'v'
    do
        cat $2 | ssh root@avrnude.lan "$prefix -V -U flash:$op:-:i"
    done
else
    cnt=0
    for f in $2 $3 $4
    do
        [[ $f =~ ^[0-9A-F]{2}$ ]] && ((cnt+=1))
    done
    if [ $cnt == 3 ]
    then
        ssh root@avrnude.lan "$prefix -U lfuse:w:0x$2:m -Uhfuse:w:0x$3:m -Uefuse:w:0x$4:m"
    else
        echo "no hex file, no 3 hex numbers, what else do you want of me?"
    fi
fi
