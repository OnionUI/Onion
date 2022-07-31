#!/bin/sh
echo $0 $*
installdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

unzip_progress() {
    zipfile=$1
	msg=$2
    total=`unzip -l "$zipfile" | tail -1 | grep -Eo "([0-9]+) files" | sed "s/[^0-9]*//g"`
    unzip -o "$zipfile" | awk -v total="$total" -v out="$installdir/.update_msg" -v msg="$msg" 'BEGIN{cnt=0}{printf "%s %3.0f%%\n",msg,cnt*100/total >> out; cnt+=1}'
    echo -ne "$msg 100%\r\n"
}

version() { echo "$@" | awk -F. '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }'; }

cd $installdir

if [ ! -f /customer/lib/libpadsp.so ]; then
	./removed
	reboot
else
	is_update=0
	verb="Installing"

	if [ -d /mnt/SDCARD/.tmp_update/onionVersion ]; then
		is_update=1
		verb="Updating"
	fi

	echo "$verb core... 0%" > $installdir/.update_msg

	# Show installation progress
	./installUI &
	sleep 2

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

	install_ra=0

	# Remove stock retroarch build
	if [ ! -f /mnt/SDCARD/RetroArch/onion_ra_version.txt ]
	then
		rm -rf /mnt/SDCARD/RetroArch
		install_ra=1
	fi

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
	cd /mnt/SDCARD
	unzip_progress $installdir/onion_package.zip "$verb core..."
	# TODO: When updating, saves and configs shouldn't be overwritten!
	
	if [ $? -ne 0 ]; then
		touch $installdir/.installFailed
		echo Onion - installation failed
		exit 0
	fi

	# An existing version of RetroArch exists
	if [ install_ra -eq 0 ]; then
		current_ra_version=`cat /mnt/SDCARD/RetroArch/onion_ra_version.txt`
		new_ra_version=`cat /mnt/SDCARD/ra_package_version.txt`
		# Install if packaged RA version is greater
		if [ $(version $current_ra_version) -lt $(version $new_ra_version) ]; then
			install_ra=1
		fi
	fi

	# Install RetroArch only if necessary (time consuming)
	if [ install_ra -eq 1]; then
		cd /mnt/SDCARD
		# TODO: Backup config and patch it
		rm -rf RetroArch
		unzip_progress /mnt/SDCARD/retroarch_package.zip "$verb RetroArch..."
	fi
	
	if [ $? -ne 0 ]; then
		touch $installdir/.installFailed
		echo RetroArch - installation failed
		exit 0
	fi

	cd /mnt/SDCARD
	rm -f retroarch_package.zip ra_package_version.txt

	touch $installdir/.installed
	sleep 3

	cd /mnt/SDCARD/App/Onion_Manual/
	./launch.sh

	cd /mnt/SDCARD/.tmp_update/ 
	./freemma

	# Launch layer manager
	cd /mnt/SDCARD/App/The_Onion_Installer/ 
	./onionInstaller

	cd /mnt/SDCARD/.tmp_update/ 
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

