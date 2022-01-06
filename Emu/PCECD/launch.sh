#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
./cpufreq.sh
cd /mnt/SDCARD/RetroArch/
#HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/mednafen_pce_fast_libretro-old.so "$1"
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/mednafen_pce_fast_libretro.so "$1"