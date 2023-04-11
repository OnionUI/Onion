#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update

if [ -f $sysdir/config/.lcdvolt ]; then
    lcdvolt=`cat $sysdir/config/.lcdvolt`
    lcdvolt_d=$((0x$lcdvolt))

    if [ $lcdvolt_d -ge $((0x09)) ] && [ $lcdvolt_d -le $((0x0e)) ]; then
        # Reduce LCD voltage from 3000 to 2800 (to remove artifacts)
        axp 21 $lcdvolt
        axp 10 +02
        axp 12 +80
    fi
fi
