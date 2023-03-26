#!/bin/sh
echo $0 $*
sysdir=/mnt/SDCARD/.tmp_update
progdir=`dirname "$0"`
rompath="$1"
romname=`basename "$rompath"`
ext=`echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}'`

cd $progdir

if [ "$ext" = "miyoocmd" ]; then
	chmod a+x "$rompath"
	"$rompath"
	exit
fi

echo "Running game : \"$rompath\""
# set CPU performance mode
./cpufreq.sh
# Running advancemame
HOME=$progdir ./advmame "${romname%.*}" 
