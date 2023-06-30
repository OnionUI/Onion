#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update

#Load current time
timepath=/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt
currentTime=0
# Load current time
if [ -f $timepath ]; then
    currentTime=`cat $timepath`
fi
#Add 4 hours to the current time
hours=4
if [ -f $sysdir/config/startup/addHours ]; then
    hours=`cat $sysdir/config/startup/addHours`
fi
addTime=$(($hours * 3600))
if [ ! -f $sysdir/config/.ntpState ]; then
    currentTime=$(($currentTime + $addTime))
fi
date +%s -s @$currentTime
