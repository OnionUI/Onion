#!/bin/sh
my_dir=$(dirname $0)
cd $my_dir

echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
. /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh

touch /tmp/stay_awake

HOME=/mnt/SDCARD/App/Gmu
LD_LIBRARY_PATH=/mnt/SDCARD/.tmp_update/lib/parasyte:./lib:$LD_LIBRARY_PATH
export LD_PRELOAD=./lib/libSDL-1.2.so.0
./gmu.bin -c gmu.miyoo.conf
unset LD_PRELOAD

rm -f /tmp/stay_awake
