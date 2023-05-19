#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

filename=`basename "$1"`

cd /mnt/SDCARD/RetroArch/

if [ "$filename" = "~Run TI-83.8xp" ]; then 
	HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/numero_libretro.so
else
	HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/numero_libretro.so "$1"
fi
