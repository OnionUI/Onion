#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
./clock
date +%s > /mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt
