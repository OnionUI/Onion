#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
portFolder=`basename "$1"`
cd "/mnt/SDCARD/Roms/PORTS/Binaries/"
cd "$portFolder"
./launch.sh

