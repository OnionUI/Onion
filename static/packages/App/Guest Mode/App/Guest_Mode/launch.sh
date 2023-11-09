#!/bin/sh
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

cd $progdir

# Save current time
./saveTime.sh
sync

# Favourites + RecentList are saved
cp /mnt/SDCARD/Roms/*.json /mnt/SDCARD/Saves/CurrentProfile/lists
rm /mnt/SDCARD/Roms/*.json

if [ -d /mnt/SDCARD/Saves/GuestProfile ] ; then
	# The main profile is the current one
	cp ./data/configON.json ./config.json
	mv /mnt/SDCARD/Saves/CurrentProfile /mnt/SDCARD/Saves/MainProfile
	mv /mnt/SDCARD/Saves/GuestProfile /mnt/SDCARD/Saves/CurrentProfile

else
	# The guest profile is the current one
	cp ./data/configOFF.json ./config.json
	mv /mnt/SDCARD/Saves/CurrentProfile /mnt/SDCARD/Saves/GuestProfile
	mv /mnt/SDCARD/Saves/MainProfile /mnt/SDCARD/Saves/CurrentProfile
fi

# Favourites + RecentList are restored
cp /mnt/SDCARD/Saves/CurrentProfile/lists/*.json /mnt/SDCARD/Roms

# Load current time
./loadTime.sh
