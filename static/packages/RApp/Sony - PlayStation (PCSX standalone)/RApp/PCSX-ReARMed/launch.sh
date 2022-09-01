#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
ROMNAME="$1"
BASEROMNAME=${ROMNAME##*/}
ROMNAMETMP=${BASEROMNAME%.*}

cd $progdir


if [ -f "/mnt/SDCARD/BIOS/${ROMNAMETMP}.bin" ]; then  # To swap bios depending the game :
		cp -f "/mnt/SDCARD/BIOS/${ROMNAMETMP}.bin" "${progdir}/.pcsx/bios/scph1001.bin"
else
		cp -f "/mnt/SDCARD/BIOS/scph1001.bin" "${progdir}/bios/scph1001.bin"
fi


./cpufreq.sh
export LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH"

if [ "$ROMNAMETMP" = "Gear Fighter Dendoh (Japan)" ]; then  # Some games have compatibility problems with default pcsx 
SDL_HIDE_BATTERY=1 HOME=$homedir $progdir/pcsx-fromMiyoo -cdfile "$1"
else
SDL_HIDE_BATTERY=1 HOME=$homedir $progdir/pcsx -cdfile "$1"
fi

