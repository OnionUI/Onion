#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

rompath="$1"
ext=`echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}'`

if [ "$ext" = "miyoocmd" ]; then
	chmod a+x "$rompath"
	"$rompath"
	exit
fi

echo "Running game : \"$rompath\""

# set CPU performance mode
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# disable audioserver to gain some MB of ram
killall audioserver
killall audioserver.mod

# Running retroarch (ScummVM will be launched with the argument contained in the .target file).
cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/scummvm_libretro.so "$rompath"

/mnt/SDCARD/miyoo/app/audioserver &
