#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

filename=`basename "$1"`

# Timer initialisation
cd /mnt/SDCARD/App/PlayActivity
./playActivity "init"

cd /mnt/SDCARD/RetroArch/

if [ "$filename" = "~Run TI-83.8xp" ]; then 
	HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/numero_libretro.so
else
	HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/numero_libretro.so "$1"
fi


# Timer registration
cd /mnt/SDCARD/App/PlayActivity
./playActivity "$1"