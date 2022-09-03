#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Super 3D Noah's Ark.port/FILES_HERE/audiohed.n3d" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/ecwolf_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Super 3D Noah's Ark.port/FILES_HERE/n3d.ecwolf"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Super 3D Noah's Ark"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
