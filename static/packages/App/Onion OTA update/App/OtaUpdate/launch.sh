#!/bin/sh
scriptdir=/mnt/SDCARD/.tmp_update/script

touch /tmp/stay_awake

cd $scriptdir
st -q -e sh $scriptdir/ota_update.sh 
