#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/CannonBall.port/FILES_HERE/epr-10187.88" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/cannonball_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/CannonBall.port/FILES_HERE/epr-10187.88"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "CannonBall"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
