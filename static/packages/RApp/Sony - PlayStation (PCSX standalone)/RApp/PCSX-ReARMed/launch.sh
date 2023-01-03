#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
ROMNAME="$1"
BASEROMNAME=${ROMNAME##*/}
ROMNAMETMP=${BASEROMNAME%.*}

cd $progdir


if [ ! -f "${progdir}/bios/scph1001.bin" ]; then  # if no bios installed...
	if [ -f "/mnt/SDCARD/BIOS/PSXONPSP660.bin" ]; then
		cp -f "/mnt/SDCARD/BIOS/PSXONPSP660.bin" "${progdir}/bios/scph1001.bin"
	elif [ -f "/mnt/SDCARD/BIOS/scph101.bin" ]; then
		cp -f "/mnt/SDCARD/BIOS/scph101.bin" "${progdir}/bios/scph1001.bin"
	elif [ -f "/mnt/SDCARD/BIOS/scph7001.bin" ]; then
		cp -f "/mnt/SDCARD/BIOS/scph7001.bin" "${progdir}/bios/scph1001.bin"
	elif [ -f "/mnt/SDCARD/BIOS/scph5501.bin" ]; then
		cp -f "/mnt/SDCARD/BIOS/scph5501.bin" "${progdir}/bios/scph1001.bin"
	elif [ -f "/mnt/SDCARD/BIOS/scph1001.bin" ]; then
		cp -f "/mnt/SDCARD/BIOS/scph1001.bin" "${progdir}/bios/scph1001.bin"
	fi
fi


./cpufreq.sh
export LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH"

if [ "$ROMNAMETMP" = "xxxxxxxxx" ]; then  # Some games have compatibility problems with default pcsx 
SDL_HIDE_BATTERY=1 HOME=$homedir $progdir/pcsx-fromMiyoo -cdfile "$1"  # run a game with the stock Miyoo core
else
SDL_HIDE_BATTERY=1 HOME=$homedir $progdir/pcsx -cdfile "$1"
fi

