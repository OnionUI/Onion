#!/bin/sh


if [ -f /mnt/SDCARD/App/Clean_View_Toggle/data/.expertON ] ; then
	# Clean mode ON
	rm /mnt/SDCARD/App/Clean_View_Toggle/data/.expertON
	rm /mnt/SDCARD/App/Clean_View_Toggle/config.json
	cp /mnt/SDCARD/App/Clean_View_Toggle/data/configClean.json ./config.json

	rm /mnt/SDCARD/miyoo/app/MainUI
	cp /mnt/SDCARD/App/Clean_View_Toggle/data/bClean /mnt/SDCARD/miyoo/app/MainUI

		
else 
	# Expert mode ON
    touch /mnt/SDCARD/App/Clean_View_Toggle/data/.expertON
	rm /mnt/SDCARD/App/Clean_View_Toggle/config.json
	cp /mnt/SDCARD/App/Clean_View_Toggle/data/configExpert.json /mnt/SDCARD/App/Clean_View_Toggle/config.json
	
	rm /mnt/SDCARD/miyoo/app/MainUI
	cp /mnt/SDCARD/App/Clean_View_Toggle/data/bExpert /mnt/SDCARD/miyoo/app/MainUI
	
	
fi
