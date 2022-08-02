#!/bin/sh
echo $0 $*
installdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

main() {
	if [ ! -d /mnt/SDCARD/.tmp_update/onionVersion ]; then
		fresh_install
		return
	fi

	# Prompt for update or fresh install
	update_only
}

remove_configs() {
	rm -rf \
		/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg
		/mnt/SDCARD/Saves/CurrentProfile/config/*
		/mnt/SDCARD/Saves/GuestProfile/config/*
}

fresh_install() {
	# Show installation progress
	./installUI &
	sleep 1

	# Backup important stock files
	backup_retroarch

	# Remove previous Onion files
	cd /mnt/SDCARD/.tmp_update
	remove_everything_except ".installer"

	# Debloat the apps folder
	debloat_apps
	refresh_roms

	backup_themes

	# Remove stock folders
	cd /mnt/SDCARD
	rm -rf Emu/* RApp/* miyoo

	install_core "Installing core..."	
	install_retroarch "Installing RetroArch..."
	install_configs "Copying configuration files..."
	echo "Installation complete" > $installdir/.update_msg

	touch $installdir/.installed
	sleep 1

	cd /mnt/SDCARD/App/Onion_Manual/
	./launch.sh
	free_mma

	# Launch layer manager
	cd /mnt/SDCARD/App/The_Onion_Installer/ 
	./onionInstaller
	free_mma

	# display turning off message
	cd /mnt/SDCARD/App/Onion_Manual
	./removed

	cd $installdir
	./boot_mod.sh 

	mv -f $installdir/system.json /appconfigs/system.json
}

update_only() {
	# Show installation progress
	./installUI &
	sleep 1

	install_core "Updating core..."	
	install_retroarch "Updating RetroArch..."
	restore_ra_config
	echo "Update complete" > $installdir/.update_msg

	touch $installdir/.installed
	sleep 1
}

install_core() {
	msg="$1"
	zipfile=$installdir/onion_package.zip

	if [ ! -f $zipfile ]; then
		return
	fi

	echo "$msg 0%" > $installdir/.update_msg

	# Onion Core installation / update
	cd /mnt/SDCARD
	unzip_progress $zipfile $msg

	if [ $? -ne 0 ]; then
		touch $installdir/.installFailed
		echo Onion - installation failed
		exit 0
	fi

	rm -f $zipfile
}

install_retroarch() {
	msg="$1"
	ra_zipfile="/mnt/SDCARD/RetroArch/retroarch_package.zip"
	ra_version_file="/mnt/SDCARD/RetroArch/onion_ra_version.txt"
	ra_package_version_file="/mnt/SDCARD/RetroArch/ra_package_version.txt"
	install_ra=1

	# An existing version of Onion's RetroArch exist
	if [ -f $ra_version_file ] && [ -f $ra_package_version_file ]; then
		current_ra_version=`cat $ra_version_file`
		package_ra_version=`cat $ra_package_version_file`

		# Skip installation if current version is up-to-date
		if [ $(version $current_ra_version) -ge $(version $package_ra_version) ]; then
			install_ra=0
		fi

		if [ $install_ra -eq 1 ] && [ -f $ra_zipfile ]; then
			# Backup old RA configuration
			cd /mnt/SDCARD/RetroArch
			mv .retroarch/retroarch.cfg /mnt/SDCARD/Backup/
		fi
	fi

	# Install RetroArch only if necessary
	if [ $install_ra -eq 1 ] && [ -f $ra_zipfile ]; then
		echo "$msg 0%" > $installdir/.update_msg

		# TODO: Backup config and patch it
		cd /mnt/SDCARD/RetroArch
		remove_everything_except `basename $ra_zipfile`
		unzip_progress $ra_zipfile $msg /mnt/SDCARD
	fi

	if [ $? -ne 0 ]; then
		touch $installdir/.installFailed
		echo RetroArch - installation failed
		exit 0
	fi

	rm -f $ra_zipfile
}

restore_ra_config() {
	cfg_file=/mnt/SDCARD/Backup/retroarch.cfg
	if [ -f $cfg_file ]; then
		mv -f $cfg_file /mnt/SDCARD/RetroArch/.retroarch/
	fi
}

version() {
	echo "$@" | awk -F. '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }';
}

install_configs() {
	msg="$1"
	zipfile=$installdir/configs.zip

	if [ ! -f $zipfile ]; then
		return
	fi

	echo "$msg" > $installdir/.update_msg

	# Extract config files without overwriting any existing files
	cd /mnt/SDCARD
	unzip -n $zipfile
}

check_firmware() {
	if [ ! -f /customer/lib/libpadsp.so ]; then
		./removed
		reboot
		exit 0
	fi
}

backup_retroarch() {
	old_ra_dir=/mnt/SDCARD/RetroArch/.retroarch

	# Move BIOS files from stock location
	if [ -d $old_ra_dir/system ] ; then
		mkdir -p /mnt/SDCARD/BIOS
		cp -R $old_ra_dir/system/. /mnt/SDCARD/BIOS/
	fi

	# Backup old saves
	if [ -d $old_ra_dir/saves ] ; then
		mkdir -p /mnt/SDCARD/Backup/saves
		cp -R $old_ra_dir/saves/. /mnt/SDCARD/Backup/saves/
	fi	

	# Backup old states
	if [ -d $old_ra_dir/states ] ; then
		mkdir -p /mnt/SDCARD/Backup/states
		cp -R $old_ra_dir/states/. /mnt/SDCARD/Backup/states/
	fi
}

backup_themes() {
	if [ -d /mnt/SDCARD/Themes ]; then
		cp -R /mnt/SDCARD/Themes/. /mnt/SDCARD/Backup/Themes/
	fi
}

debloat_apps() {
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
}

refresh_roms() {
	# Force refresh the rom lists
	if [ -d /mnt/SDCARD/Roms ] ; then
		cd /mnt/SDCARD/Roms
		find . -type f -name "*.db" -exec rm -f {} \;
	fi
}

remove_everything_except() {
	find * .* -maxdepth 0 -not -name "$1" -exec rm -rf {} \;
}

unzip_progress() {
    zipfile=$1
	msg=$2
    dest=$3
    total=`unzip -l "$zipfile" | tail -1 | grep -Eo "([0-9]+) files" | sed "s/[^0-9]*//g"`
    unzip -d "$dest" -o "$zipfile" | awk -v total="$total" -v out="$installdir/.update_msg" -v msg="$msg" 'BEGIN{cnt = 0; l = 0}{
        p = int(cnt * 100 / total);
        if (p != l) {
            printf "%s %3.0f%%\n", msg, p >> out;
            close(out);
            l = p;
        }
        cnt += 1
    }'
    echo "$msg 100%" >> $installdir/.update_msg
}

free_mma() {
	/mnt/SDCARD/.tmp_update/freemma
}

main
sync
reboot
sleep 10
