#!/bin/sh

cd /mnt/SDCARD/

rm -rf .Spotlight-V100 .apDisk .fseventsd .TemporaryItems .Trash .Trashes

find ./Roms ./Media -depth -type f \( -name "._*" -o -name ".DS_Store" \) -not -path "**/._state_seen/*" -delete
find ./Roms ./Media -depth -type d -name "__MACOSX" -exec rm -rf {} \;
find ./Roms -type f -name "*_cache[0-9].db" -exec rm -f {} \;
