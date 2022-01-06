#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
./cpufreq.sh
cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/pcsx_rearmed_libretro.so "$1"
