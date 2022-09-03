#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/POWDER.port/powder" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	/mnt/SDCARD/Roms/PORTS/Binaries/POWDER.port/powder

	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "POWDER"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
