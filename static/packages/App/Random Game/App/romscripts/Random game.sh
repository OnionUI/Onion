#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
emupath=`echo "$1" | awk '{st = index($0,"/../../"); print substr($0,0,st-1)}'`
emuname=`basename "$emupath"`

cd $sysdir
./bin/randomGamePicker "$emuname"

exit 0
