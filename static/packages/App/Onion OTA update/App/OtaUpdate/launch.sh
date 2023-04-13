#!/bin/sh

touch /tmp/stay_awake

cd /mnt/SDCARD/.tmp_update/script
st -q -e sh ./ota_update.sh 
