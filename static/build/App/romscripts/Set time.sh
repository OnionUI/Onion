#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
savedir=/mnt/SDCARD/Saves/CurrentProfile/saves

cd $sysdir
HOME=/mnt/SDCARD
./bin/clock

date +%s > $savedir/currentTime.txt
