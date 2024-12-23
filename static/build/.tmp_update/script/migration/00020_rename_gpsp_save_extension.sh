#!/bin/sh
save_dir=/mnt/SDCARD/Saves/CurrentProfile/saves

if [ ! -d $save_dir/gpSP ]; then
    exit 0
fi

# Rename gpSP save files from .sav to .srm
for file in $save_dir/gpSP/*.sav; do
    mv -n -- "$file" "${file%.sav}.srm"
done
