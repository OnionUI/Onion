#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Quake.port/FILES_HERE/PAK0.PAK" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/tyrquake_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Quake.port/FILES_HERE/PAK0.PAK"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Quake"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
