#!/bin/sh
echo $0 $*

set -o pipefail

cd /mnt/SDCARD/.tmp_update
./bin/randomGamePicker

if [ $? -eq 99 ]; then
    /mnt/SDCARD/.tmp_update/bin/infoPanel --title "NO GAMES FOUND" --message "It looks like you don't have\n any roms on your device!"
fi