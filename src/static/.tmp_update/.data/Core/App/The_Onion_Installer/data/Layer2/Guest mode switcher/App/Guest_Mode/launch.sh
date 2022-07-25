#!/bin/sh

# Save current time
cd /mnt/SDCARD/.tmp_update/
./saveTime.sh
sync

if [ -d /mnt/SDCARD/Saves/GuestProfile ] ; then
	# The main profile is the current one
	cd /mnt/SDCARD/App/Guest_Mode
	cp ./data/configON.json ./config.json
	mv /mnt/SDCARD/Saves/CurrentProfile /mnt/SDCARD/Saves/MainProfile
	mv /mnt/SDCARD/Saves/GuestProfile /mnt/SDCARD/Saves/CurrentProfile

else
	# The guest profile is the current one
	cd /mnt/SDCARD/App/Guest_Mode
	cp ./data/configOFF.json ./config.json
	mv /mnt/SDCARD/Saves/CurrentProfile /mnt/SDCARD/Saves/GuestProfile
	mv /mnt/SDCARD/Saves/MainProfile /mnt/SDCARD/Saves/CurrentProfile

fi

# Load current time
sTime=$(cat /mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt)
Add 4 hours to the current time
sTime=$(($sTime + 14400))
sTime="@${sTime}"
$(date +%s -s ${sTime})