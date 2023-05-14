#!/bin/sh
. /mnt/SDCARD/App/EmuDeckCloudSync/settings.sh

sysdir=/mnt/SDCARD/.tmp_update
PATH="$sysdir/bin:$PATH"
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/mnt/SDCARD/.tmp_update/lib:$sysdir/lib/parasyte"

#Tools

emudeck_check_internet_connection(){
	  ping -q -c 1 -W 1 8.8.8.8 > /dev/null 2>&1 && echo true || echo false
}
emudeck_getSetDate(){
	# Automatically Updates System Time According to the NIST Atomic Clock in a Linux Environment
	 nistTime=$(curl -I --insecure 'https://www.google.com/' | grep "Date")
	 dateString=$(echo $nistTime | cut -d' ' -f2-7)
	 dayString=$(echo $nistTime | cut -d' ' -f2-2)
	 dateValue=$(echo $nistTime | cut -d' ' -f3-3)
	 monthValue=$(echo $nistTime | cut -d' ' -f4-4)
	 yearValue=$(echo $nistTime | cut -d' ' -f5-5)
	 timeValue=$(echo $nistTime | cut -d' ' -f6-6)
	 timeZoneValue=$(echo $nistTime | cut -d' ' -f7-7)
	#echo $dateString
	case $monthValue in
		"Jan")
			monthValue="01"
			;;
		"Feb")
			monthValue="02"
			;;
		"Mar")
			monthValue="03"
			;;
		"Apr")
			monthValue="04"
			;;
		"May")
			monthValue="05"
			;;
		"Jun")
			monthValue="06"
			;;
		"Jul")
			monthValue="07"
			;;
		"Aug")
			monthValue="08"
			;;
		"Sep")
			monthValue="09"
			;;
		"Oct")
			monthValue="10"
			;;
		"Nov")
			monthValue="11"
			;;
		"Dec")
			monthValue="12"
			;;
		*)
			continue
	esac
	date --utc --set $yearValue.$monthValue.$dateValue-$timeValue
	
			
}


emudeck_escapeSedKeyword(){
	local INPUT=$1;
	printf '%s\n' "$INPUT" | sed -e 's/[]\/$*.^[]/\\&/g'
}

emudeck_escapeSedValue(){
	local INPUT=$1
	printf '%s\n' "$INPUT" | sed -e 's/[\/&]/\\&/g'
}

# keyword replacement file. Only matches start of word
emudeck_changeLine() {
	KEYWORD=$1; shift
	REPLACE=$1; shift
	FILE=$1

	OLD=$(emudeck_escapeSedKeyword "$KEYWORD")
	NEW=$(emudeck_escapeSedValue "$REPLACE")

	echo "Updating: $FILE - $OLD to $NEW"
	#echo "Old: ""$(cat "$FILE" | grep "^$OLD")"
	sed -i "/^${OLD}/c\\${NEW}" "$FILE"
	#echo "New: ""$(cat "$FILE" | grep "^$OLD")"
}



#Cloud functions


emudeck_fail_upload(){
	time_stamp=$(cat $EmuDeckPath/.fail_upload)
	date=$(date -d @$time_stamp +'%x')
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
		-t "CloudSync conflict" \
		-m "We've detected a previously failed upload,\ndo you want us to upload your saves \nand overwrite your saves in the cloud?\n Your latest upload was on $date" \
		"Upload now to the cloud" \
		"No, download the ones I have on the cloud"
	
	retcode=$?
	if [ $retcode -eq 0 ]; then		
		emudeck_cloud_upload
	fi
	
	if [ $retcode -eq 1 ]; then			
		emudeck_cloud_download
	fi
}

emudeck_pending_upload(){
	time_stamp=$(cat $EmuDeckPath/.pending_upload)
	date=$(date -d @$time_stamp +'%x')
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
		-t "CloudSync conflict" \
		-m "We've detected a pending upload,\nthis probably was because your device \nlost connectivity at some point,\n do you want us to upload your saves\n to the cloud and overwrite them?\n This upload should have happened on $date" \
		"Upload now to the cloud" \
		"No, download the ones I have on the cloud"		
	retcode=$?
	if [ $retcode -eq 0 ]; then
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/uploading.json --persistent &
		emudeck_cloud_upload
		touch /tmp/dismiss_info_panel
	fi
	
	if [ $retcode -eq 1 ]; then			
		emudeck_cloud_download
	fi
}

emudeck_fail_download(){
	time_stamp=$(cat $EmuDeckPath/.fail_download)
	date=$(date -d @$time_stamp +'%x')
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
		-t "CloudSync conflict" \
		-m "We've detected a previously failed download,\n do you want us to download your saves\n from the cloud and overwrite your saves?\n Your latest download was on $date" \
		"Yes, download" \
		"No, upload to the cloud and overwrite my cloud saves"
	
	retcode=$?
	if [ $retcode -eq 0 ]; then
		emudeck_cloud_download
	fi
	
	if [ $retcode -eq 1 ]; then
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/uploading.json --persistent &
		emudeck_cloud_upload
		touch /tmp/dismiss_info_panel
	fi
}

emudeck_cloud_upload(){
	#folder=$(dirname "$path")
	folder=/mnt/SDCARD/Saves/CurrentProfile/
	timestamp=$(date +%s)
	
	if [ $(emudeck_check_internet_connection) = true ]; then
		#infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/uploading.json	 --persistent	&			
		#sleep 1
		echo $timestamp > $EmuDeckPath/.pending_upload
		echo $timestamp > $EmuDeckPath/.fail_upload
		/mnt/SDCARD/rclone copy -P -L --no-check-certificate $folder --exclude=/config/** --exclude=/lists/** --exclude=/romScreens/**  --exclude=/.fail_upload --exclude=/.fail_download--exclude=/.pending_upload $cloudProvider:$systemCloud/saves/retroarch/ && echo $timestamp > $EmuDeckPath/.last_upload && rm -rf $EmuDeckPath/.pending_upload && rm -rf $EmuDeckPath/.fail_upload
		
		#touch /tmp/dismiss_info_panel
	else
		echo $timestamp > $EmuDeckPath/.fail_upload
	fi
}


emudeck_cloud_download(){
	timestamp=$(date +%s)

	if [ $(emudeck_check_internet_connection) = true ]; then
	
		echo $timestamp > $EmuDeckPath/.pending_download
		echo $timestamp > $EmuDeckPath/.fail_download
																
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/downloading.json --persistent &		
		/mnt/SDCARD/rclone copy -P -L --no-check-certificate  --exclude=/.fail_upload --exclude=/.fail_download--exclude=/.pending_upload $cloudProvider:$systemCloud/saves/retroarch/  /mnt/SDCARD/Saves/CurrentProfile/ && echo $timestamp > $EmuDeckPath/.last_download && rm -rf $EmuDeckPath/.fail_download && rm -rf $EmuDeckPath/.pending_download
		touch /tmp/dismiss_info_panel

	else
		echo $timestamp > $EmuDeckPath/.fail_download	
	fi
}
