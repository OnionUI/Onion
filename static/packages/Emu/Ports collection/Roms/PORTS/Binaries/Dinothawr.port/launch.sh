#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Dinothawr.port/FILES_HERE/dinothawr.game" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/dinothawr_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Dinothawr.port/FILES_HERE/dinothawr.game"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Dinothawr"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
