#!/bin/sh

if [ -d /mnt/SDCARD/Roms ] ; then

	cd /mnt/SDCARD/Roms
	find . -type f -name "*.db" -exec rm -f {} \;
	
fi