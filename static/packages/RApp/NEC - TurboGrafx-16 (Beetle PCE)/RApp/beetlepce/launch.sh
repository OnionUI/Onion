#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ "$(cat /tmp/deviceModel)" = "283" ]; then
	echo -n 1600 >"/mnt/SDCARD/Saves/CurrentProfile/config/Beetle PCE/cpuclock.txt" #  Miyoo Mini
else
	echo -n 1900 >"/mnt/SDCARD/Saves/CurrentProfile/config/Beetle PCE/cpuclock.txt" #  Miyoo Mini Plus
fi

cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/mednafen_pce_libretro.so "$1"
