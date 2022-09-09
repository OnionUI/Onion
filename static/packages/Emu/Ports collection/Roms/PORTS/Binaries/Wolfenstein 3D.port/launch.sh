#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Wolfenstein 3D.port/FILES_HERE/AUDIOHED.WL6" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/ecwolf_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Wolfenstein 3D.port/FILES_HERE/WL6.ecwolf"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Wolfenstein 3D"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
