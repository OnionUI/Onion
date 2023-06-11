#!/bin/sh
echo $0 $*

game=$(cat "$1")
progdir=`dirname "$0"`
scummvm_config=/mnt/SDCARD/Saves/CurrentProfile/config/scummvm_standalone/.scummvmrc


cd $progdir
HOME=$progdir LD_LIBRARY_PATH=$progdir:$LD_LIBRARY_PATH $progdir/scummvm  -c "$scummvm_config" -l "/mnt/SDCARD/.tmp_update/logs/scummvm.log" $game
