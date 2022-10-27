#!/bin/sh
mydir=`dirname "$0"`

export LD_LIBRARY_PATH=/mnt/SDCARD/.tmp_update/lib/parasyte:$LD_LIBRARY_PATH

export SDL_VIDEODRIVER=mmiyoo
export SDL_AUDIODRIVER=mmiyoo

echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

cd $mydir


killall audioserver
killall audioserver.mod


./OpenBOR "$1"


/mnt/SDCARD/miyoo/app/audioserver &