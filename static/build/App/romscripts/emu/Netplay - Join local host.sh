#!/bin/sh
echo $0 $*
sysdir=/mnt/SDCARD/.tmp_update
cd $sysdir
/bin/sh "$sysdir/script/quickplay/netplay_client.sh"