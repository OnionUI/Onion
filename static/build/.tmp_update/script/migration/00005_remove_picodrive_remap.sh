#!/bin/sh
# Remove faulty PicoDrive remap
pdrmp_file="/mnt/SDCARD/Saves/CurrentProfile/config/remaps/PicoDrive/PicoDrive.rmp"
if [ -f "$pdrmp_file" ] && [ $(md5hash "$pdrmp_file") == "a3895a0eab19d4ce8aad6a8f7ded57bc" ]; then
    rm -f "$pdrmp_file"
else
    echo "Skipped"
fi
