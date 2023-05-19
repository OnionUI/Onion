#!/bin/sh

sysdir=/mnt/SDCARD/.tmp_update
savedir=/mnt/SDCARD/Saves/CurrentProfile/saves

echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

# Arcade save state tutorial (played on the first launch only)

if ! [ -f "$savedir/MAME 2003-Plus/.mame2003plusTutorialDone" ]; then 
	
	mkdir "$savedir/MAME 2003-Plus"
	
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "Arcade Save States Tutorial" \
	-m "Watch a quick video to learn\nhow to use arcade save states ?" \
	"Yes" \
	"No"

	retcode=$?

	if [ $retcode -eq 0 ]; then
			cd $sysdir
			touch /tmp/stay_awake
			./bin/ffplay -autoexit -vf "hflip,vflip" -i "/mnt/SDCARD/Media/Videos/Onion/Mame2003Plus Save State Tutorial.mp4"
			rm -f /tmp/stay_awake
	fi

	touch "$savedir/MAME 2003-Plus/.mame2003plusTutorialDone"
fi





cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/mame2003_plus_libretro.so "$1"
