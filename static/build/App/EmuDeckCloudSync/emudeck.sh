#!/bin/sh
. /mnt/SDCARD/App/EmuDeckCloudSync/functions.sh

init(){
	
	sleep 1
	while [ "$(emudeck_check_internet_connection)" != "true" ]; do
		echo "CONTINUE" >> /mnt/SDCARD/test.txt	
		LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
			-t "No internet connection" \
			-m "It seems you are not connected to the internet, \n do you want to wait until your device\n connects or do you want to play offline?" \
			"Try again" \
			"I want to play now with no sync"
		
		retcode=$?		
		
		if [ $retcode -eq 0 ]; then			
			emudeck_changeLine 'cloudEnabled' 'cloudEnabled=true' $EmuDeckPath/settings.sh
			. /mnt/SDCARD/App/EmuDeckCloudSync/functions.sh		 												
		fi
		
		if [ $retcode -eq 1 ]; then	
			emudeck_changeLine 'cloudEnabled' 'cloudEnabled=false' $EmuDeckPath/settings.sh
			. /mnt/SDCARD/App/EmuDeckCloudSync/functions.sh
			exit
		fi				
	done
	
	#Set proper date
	emudeck_getSetDate
	
	if [ -f $EmuDeckPath/.pending_upload ]; then
		emudeck_pending_upload
	elif [ -f $EmuDeckPath/.fail_upload ]; then		
		emudeck_fail_upload		
	elif [ -f $EmuDeckPath/.fail_download ]; then		
		emudeck_fail_download
	else
		emudeck_cloud_download
	fi
	
	#emudeck_cloud_watch
	
}

init	