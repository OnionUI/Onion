#!/bin/sh


if [ -f /mnt/SDCARD/App/OnionView/data/.expertON ] ; then
	# Clean mode ON
	rm /mnt/SDCARD/App/OnionView/data/.expertON
	rm /mnt/SDCARD/App/OnionView/config.json
	cp /mnt/SDCARD/App/OnionView/data/configClean.json ./config.json

	rm /mnt/SDCARD/miyoo/app/MainUI
	cp /mnt/SDCARD/App/OnionView/data/bClean /mnt/SDCARD/miyoo/app/MainUI

		
else 
	# Expert mode ON
    touch /mnt/SDCARD/App/OnionView/data/.expertON
	rm /mnt/SDCARD/App/OnionView/config.json
	cp /mnt/SDCARD/App/OnionView/data/configExpert.json /mnt/SDCARD/App/OnionView/config.json
	
	rm /mnt/SDCARD/miyoo/app/MainUI
	cp /mnt/SDCARD/App/OnionView/data/bExpert /mnt/SDCARD/miyoo/app/MainUI
	
	
fi
