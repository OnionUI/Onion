#!/bin/sh
scriptlabel="Easy Netplay: Host a game"
require_networking=1
echo $0 $*
sysdir=/mnt/SDCARD/.tmp_update
cd $sysdir
/bin/sh "$sysdir/script/easynetplay/netplay_server.sh"