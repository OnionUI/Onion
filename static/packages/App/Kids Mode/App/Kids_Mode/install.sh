#!/bin/sh

progdir="$(CDPATH= cd -- "$(dirname "$0")" >/dev/null 2>&1 && pwd -P)"
cd "$progdir" || exit 1

if [ -d /mnt/SDCARD/.KidMode/backup/App ] && [ -f /mnt/SDCARD/App/.kidmode_kiosk ]; then
    cp ./data/configON.json ./config.json
else
    cp ./data/configOFF.json ./config.json
fi
