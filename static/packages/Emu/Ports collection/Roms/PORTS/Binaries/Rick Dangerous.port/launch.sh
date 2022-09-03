#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Rick Dangerous.port/FILE_HERE/data.zip" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/xrick_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Rick Dangerous.port/FILE_HERE/data.zip"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Rick Dangerous"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
