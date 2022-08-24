#!/bin/sh

cd /mnt/SDCARD/Saves/CurrentProfile/saves/
rm ./currentTime.txt
date +%s > currentTime.txt
