#!/bin/sh
installdir=.

main() {
	# if [ ! -d /mnt/SDCARD/.tmp_update/onionVersion ]; then
	# 	fresh_install 1
	# 	return
	# fi

	# Prompt for update or fresh install
	$installdir/prompt -m "Welcome to the Onion installer!\nPlease choose an action:" \
        "Update (keep settings)" \
        "Reinstall (reset settings)" \
        "Update OS/RetroArch only"
    retcode=$?

	if [ $retcode -eq 0 ]; then
		# Update
		update_only
	elif [ $retcode -eq 1 ]; then
		# Repair (keep settings)
		fresh_install 0
	elif [ $retcode -eq 2 ]; then
		# Fresh install
		fresh_install 1
	fi
}

fresh_install() {
    reset_configs=$1
    
    if [ $reset_configs -eq 1 ]; then
        echo "Fresh install"
    else
        echo "Repair (keep settings)"
    fi
}

update_only() {
    echo "Update"
}

main