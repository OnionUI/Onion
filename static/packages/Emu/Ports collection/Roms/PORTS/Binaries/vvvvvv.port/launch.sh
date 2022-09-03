#!/bin/sh
echo $0 $*
progdir="/mnt/SDCARD/Roms/PORTS/Binaries/vvvvvv.port"

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/vvvvvv.port/data.zip" ]; then
	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "init"

	cd $progdir
	HOME=$progdir
	a=`ps | grep audioserver | grep -v grep`
	if [ "$a" == "" ] ; then
	  export LD_LIBRARY_PATH=$progdir:$LD_LIBRARY_PATH
	fi
	$progdir/vvv_sdl12


	# Timer registration
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "vvvvvv"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi

