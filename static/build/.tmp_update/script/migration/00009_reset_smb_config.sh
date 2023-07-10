#!/bin/sh
RESET_CONFIGS_PAK="/mnt/SDCARD/.tmp_update/config/configs.pak"

if [ -f "$RESET_CONFIGS_PAK" ]; then
    7z x -aoa "$RESET_CONFIGS_PAK" -o/mnt/SDCARD/ -ir!.tmp_update/config/smb.conf
else
    echo "Skipped"
fi
