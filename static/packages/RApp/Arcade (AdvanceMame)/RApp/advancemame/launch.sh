#!/bin/sh
echo $0 $*
sysdir=/mnt/SDCARD/.tmp_update
progdir=`dirname "$0"`
rompath="$1"
romname=`basename "$rompath"`

cd $progdir

echo "Running game : \"$rompath\""
# set CPU performance mode
./cpufreq.sh
# Running advancemame
HOME=$progdir ./advmame "${romname%.*}" 
