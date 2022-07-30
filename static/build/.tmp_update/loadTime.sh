#!/bin/sh
#Load current time
sTime=$(cat /mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt)
sTime="@${sTime}"
$(date +%s -s ${sTime})