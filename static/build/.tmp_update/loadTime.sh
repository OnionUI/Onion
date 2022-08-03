#!/bin/sh
cd /mnt/SDCARD/Saves/CurrentProfile/saves/
currentTime=0

if [ -f currentTime.txt ]; then
    currentTime=`cat currentTime.txt`
fi

date +%s -s @$currentTime
