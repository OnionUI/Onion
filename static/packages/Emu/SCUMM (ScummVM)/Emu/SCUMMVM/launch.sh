#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
rompath="$1"

echo "Running game : \"$rompath\""

# set CPU performance mode
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# disable audioserver to gain some MB of ram
. /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh

# Running retroarch (ScummVM will be launched with the argument contained in the .target file).
cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/scummvm_libretro.so "$rompath"
