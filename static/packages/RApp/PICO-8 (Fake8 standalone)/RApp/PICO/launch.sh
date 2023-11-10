#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
cd $progdir
./cpufreq.sh

. /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh

HOME=$homedir $progdir/FAKE08 "$1"
