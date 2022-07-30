#!/bin/sh
echo $0 $*
installdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

cd $installdir

if [ ! -f /customer/lib/libpadsp.so ]; then
	./removed
	reboot
else
	# Waiting background
	./installUI &
	sleep 2

	touch $installdir/.unpacked

	if [ -d /mnt/SDCARD/RetroArch/.retroarch/system ] ; then
		mkdir -p /mnt/SDCARD/BIOS
		cp -R /mnt/SDCARD/RetroArch/.retroarch/system/. /mnt/SDCARD/BIOS/
	fi

	# Backup saves just in case
	if [ -d /mnt/SDCARD/RetroArch/.retroarch/saves ] ; then
		mkdir -p /mnt/SDCARD/Backup/saves
		cp -R /mnt/SDCARD/RetroArch/.retroarch/saves/. /mnt/SDCARD/Backup/saves/
	fi	

	# Backup states just in case
	if [ -d /mnt/SDCARD/RetroArch/.retroarch/states ] ; then
		mkdir -p /mnt/SDCARD/Backup/states
		cp -R /mnt/SDCARD/RetroArch/.retroarch/states/. /mnt/SDCARD/Backup/states/
	fi

	# Remove stock retroarch build
	rm -rf /mnt/SDCARD/RetroArch

	# Debloating the Apps
	cd /mnt/SDCARD/App
	rm -rf \
		Commander_CN \
		power \
		RetroArch \
		swapskin \
		Pal \
		OpenBor \
		Onion_Manual \
		PlayActivity \
		Retroarch \
		The_Onion_Installer
	
	# Force refresh the rom lists
	if [ -d /mnt/SDCARD/Roms ] ; then
		cd /mnt/SDCARD/Roms
		find . -type f -name "*.db" -exec rm -f {} \;
	fi

	cd /mnt/SDCARD
	rm -rf Emu/* RApp/* miyoo

	# Onion Core installation / update
	if [ -f $installdir/onion_package.zip ]
	then
		cd /mnt/SDCARD
		unzip -o $installdir/onion_package.zip
	else
		echo Onion - unzip failed
		exit 0
	fi

	touch $installdir/.coreInstalled
	sleep 3

	cd /mnt/SDCARD/App/Onion_Manual/
	./launch.sh

	cd /mnt/SDCARD/App/The_Onion_Installer/ 
	./freemma

	# Launch layer manager
	cd /mnt/SDCARD/App/The_Onion_Installer/ 
	./onionInstaller

	cd /mnt/SDCARD/App/The_Onion_Installer/ 
	./freemma

	# display turning off message
	cd /mnt/SDCARD/App/Onion_Manual
	./removed

	cd $installdir
	./boot_mod.sh 

	mv -f $installdir/system.json /appconfigs/system.json

	cd /mnt/SDCARD
	rm -rf $installdir
	
	sync
	reboot
	sleep 10
fi

