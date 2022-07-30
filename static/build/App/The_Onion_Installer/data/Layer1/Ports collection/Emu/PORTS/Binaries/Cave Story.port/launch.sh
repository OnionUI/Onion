#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`



if [ -f "/mnt/SDCARD/Emu/PORTS/Binaries/Cave Story.port/FILES_HERE/Doukutsu.exe" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/nxengine_libretro.so "/mnt/SDCARD/Emu/PORTS/Binaries/Cave Story.port/FILES_HERE/Doukutsu.exe"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Cave Story"
else
	cd "/mnt/SDCARD/Emu/PORTS/Binaries/missingFile"
	./infoPanel
fi



