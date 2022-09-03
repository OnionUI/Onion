#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`



if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Doom2.port/FILE_HERE/doom2.wad" ]; then
	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/prboom_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Doom2.port/FILE_HERE/doom2.wad"

	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Doom 2"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi



