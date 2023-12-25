#!/bin/sh

# 560p beta testers had a different res_exception file
# If the file does not contain "/mnt/SDCARD/App/" we need to reset it

RESET_CONFIGS_PAK="/mnt/SDCARD/.tmp_update/config/configs.pak"

if ! grep -qF "/mnt/SDCARD/App/" /mnt/SDCARD/.tmp_update/config/res_exceptions; then
    echo "Resetting res_exceptions"
    if 7z x -aoa "$RESET_CONFIGS_PAK" -o/mnt/SDCARD/ -ir!.tmp_update/config/res_exceptions; then
        echo "Extraction successful."
    else
        echo "Error during extraction. Exit code: $?"
    fi
else
    echo "Don't need to reset res_exceptions"
fi
