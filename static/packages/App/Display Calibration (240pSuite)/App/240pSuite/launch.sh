#!/bin/sh

echo $0 $*
progdir=`dirname "$0"`
cd /mnt/SDCARD/RetroArch/
./retroarch -v -L /mnt/SDCARD/App/240pSuite/snes9x2005_plus_libretro.so /mnt/SDCARD/App/240pSuite/240pSuite.sfc


	