#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Flashback.port/FILES_HERE/level1.map" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/reminiscence_libretro.so --menu


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Flashback"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
