#!/bin/sh

if [ -d /mnt/SDCARD/miyoo ] ; then
    
	
	for FILE in ./data/miyoo/*
		do
        cp -R $FILE /mnt/SDCARD/miyoo/
	done
	
fi

reboot
