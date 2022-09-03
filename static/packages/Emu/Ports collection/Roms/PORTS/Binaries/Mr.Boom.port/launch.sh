#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Mr.Boom.port/FILE_HERE/MrBoom.exe" ]; then

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd /mnt/SDCARD/RetroArch/
	HOME=/mnt/SDCARD/RetroArch/ /mnt/SDCARD/RetroArch/retroarch -v -L /mnt/SDCARD/RetroArch/.retroarch/cores/mrboom_libretro.so "/mnt/SDCARD/Roms/PORTS/Binaries/Mr.Boom.port/FILE_HERE/MrBoom.exe"


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Mr.Boom"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi
