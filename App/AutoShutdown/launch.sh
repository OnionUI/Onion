#!/bin/sh

if [ ! -d /mnt/SDCARD/.tmp_update ] ; then
    mkdir /mnt/SDCARD/.tmp_update
fi

if [ -f /mnt/SDCARD/App/AutoShutdown/data/updater_backup ] ; then
    rm /mnt/SDCARD/.tmp_update/updater
else
    mv /mnt/SDCARD/.tmp_update/updater /mnt/SDCARD/App/AutoShutdown/data/updater_backup
fi
   
cp /mnt/SDCARD/App/AutoShutdown/data/updater /mnt/SDCARD/.tmp_update
chmod a+x /mnt/SDCARD/.tmp_update/updater

cd /mnt/SDCARD/App/AutoShutdown/

if [ -f /mnt/SDCARD/App/AutoShutdown/data/.enabled ] ; then
	rm /mnt/SDCARD/App/AutoShutdown/data/.enabled
	rm ./config.json
	cp ./data/configOFF.json ./config.json
	cd /mnt/SDCARD/
	rm -rf .tmp_update
else 
    if [ -f /mnt/SDCARD/App/AutoShutdown/data/.first_enabled ] ; then
		rm /mnt/SDCARD/App/AutoShutdown/data/.first_enabled
		rm ./config.json
		cp ./data/configOFF.json ./config.json
		cd /mnt/SDCARD/
		rm -rf .tmp_update
	else
		touch /mnt/SDCARD/App/AutoShutdown/data/.first_enabled
		rm ./config.json
		cp ./data/configON.json ./config.json
	fi
fi
