#!/bin/sh

# Timer initialisation
cd /mnt/SDCARD/App/PlayActivity
./playActivity "init"

if [ -f "/mnt/SDCARD/Roms/PORTS/Binaries/Duke nukem 3D.port/FILE_HERE/duke3d.grp" ]; then
	killall audioserver
	killall audioserver.mod

	export LD_LIBRARY_PATH="/mnt/SDCARD/Roms/PORTS/Binaries/Duke nukem 3D.port/lib:$LD_LIBRARY_PATH"
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/Duke nukem 3D.port/"
	./eduke32 ./FILE_HERE/duke3d.grp &>"/mnt/SDCARD/Roms/PORTS/Binaries/Duke nukem 3D.port/eduke_output.log"

	/mnt/SDCARD/miyoo/app/audioserver &

	# Timer initialisation
	cd /mnt/SDCARD/App/PlayActivity
	./playActivity "Duke Nukem 3D"
else
	cd "/mnt/SDCARD/Roms/PORTS/Binaries/missingFile"
	./infoPanel
fi



