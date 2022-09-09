#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Spear of Destiny.port/FILES_HERE/AUDIOHED.SOD" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/ecwolf_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Spear of Destiny.port/FILES_HERE/SOD.ecwolf"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Spear of Destiny"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
