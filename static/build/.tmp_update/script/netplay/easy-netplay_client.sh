# GLO CLIENT
# Script to:
# Enable Wifi
# Pull the cookie file from the host
# Pull the core from the host based on what the cookie file gives us
# Start retroarch with the above passed, and -C provided to connect.

# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export hostip="192.168.100.100" # This should be the default unless the user has changed it..
peer_ip="$hostip"

logfile=easy_netplay

# Source scripts
. $sysdir/script/log.sh
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running, restore_ftp, udhcpc_control, url_encode, read_cookie, check_wifi, start_ftp
. $sysdir/script/netplay/easy-netplay_common.sh

program=$(basename "$0" .sh)

##########
##Setup.##
##########

# Read the cookie and store the paths and checksums into a var.

sync_file() {

	MAX_FILE_CHKSUM_SIZE=26214400
	MAX_FILE_DL_SIZE=104857600

	file_type="$1"            # Used in displayed message and some custom actions
	file_path="$2"            # Local file path
	file_check_size="$3"      # 0 or 1 to indicate if we have to check the file size
	remote_file_checksum="$4" # 0 to skip , real checksum value to check
	sync_type="$5"            # -o overwrite if different (require file_size or/and checksum != 0) , (if file_size & checksum = 0 then the file is never overwritted, only copied if not present)
	# -f overwrite all the time (whatever the value of file_size and checksum)  (if file_size & checksum = 0 then the file is overwritted, even if already present )
	# -b if different, backup local file before copying remote file
	# -c check only, allows to check the presence of a file, to check its CRC or size and to quit or not the script
	file_mandatory="$6" # -m , exit the script on failed sync_success

	#examples :
	# sync_file archive /mnt/SDCARD/test.zip 1 5AFC442 -b -m 			# backup and replace if different file
	# sync_file archive /mnt/SDCARD/test.zip 0 0 -f -m 					# the local file will be systematically replaced
	# sync_file archive /mnt/SDCARD/test.zip 0 0 -o -m 					# the local file will be copied if not exist
	# sync_file archive /mnt/SDCARD/test.zip 0 5AFC442 -c -m 			# exit if the file doesn't have the right checksum
	# sync_file archive /mnt/SDCARD/test.zip 1 5AFC442 -o -m 			# the local file will be replaced if the size or the checksum is different

	# some useful vars
	dir_path=$(dirname "$file_path")
	file_url="ftp://${hostip}/$(url_encode "${file_path#*/}")"
	remote_ip="$hostip"

	echo "############################ DEBUGGING #######################################"
	echo file_type $file_type
	echo file_path $file_path
	echo file_check_size $file_check_size
	echo remote_file_checksum $remote_file_checksum
	echo sync_type $sync_type
	echo file_mandatory $file_mandatory
	echo
	echo dir_path $dir_path
	echo romdirname $romdirname
	echo romName $romName
	echo romNameNoExtension $romNameNoExtension
	echo Img_path $Img_path
	echo file_url $file_url
	echo "##############################################################################"

	# state vars
	same_size=
	same_chksum=
	sync_success=
	run_sync= # tell if the sync task must be done or not

	if ! ensure_ftp_head "$file_url" "$remote_ip" "$file_mandatory"; then
		same_size=0
		run_sync=0
		sync_success=0
	fi
	RequestResult="$ftp_head_result"

	if [[ $RequestResult == *"The file does not exist"* ]]; then
		log "The remote file does not exist."
		msg="The remote file does not exist."
		build_infoPanel_and_log "Syncing" "The remote file does not exist."
		run_sync=0
		sync_success=0
		same_size=0
	else
		remote_file_size=$(echo "$RequestResult" | grep -i "Content-Length" | awk '{print $2}')
		if ! echo "$remote_file_size" | grep -q "^[0-9][0-9]*$"; then # check if the remote file size is a numeric value
			log "Non-numeric remote file size: '$remote_file_size'"
			log "HEAD response: $RequestResult"
			same_size=0
		else
			log "remote_file_size: $remote_file_size"
		fi

	fi

	if [ -e "$file_path" ]; then

		########################## File checksum check : same_chksum = 0 different, 1 identical , 2 unknown
		checksum_func "$file_path" "$remote_file_checksum"

		########################## File size check   same_size = 0 different, 1 identical , 2 unknown
		if [ "$file_check_size" -eq 1 ]; then # file_checksum=0 means skip the difference check = always replace
			checksize_func "$file_path" "$remote_file_size"
		else
			log "Skipping file size check."
			same_size=1 # fake same size for skipping
		fi

	else
		log "The local file does not exist."
		same_size=0
		same_chksum=0
	fi

	########################## exception : max file size check on the remote
	if echo "$remote_file_size" | grep -q "^[0-9][0-9]*$"; then
		if [ "$remote_file_size" -le "$MAX_FILE_DL_SIZE" ]; then
			log "Remote file size ok: $remote_file_size bytes  (<= $MAX_FILE_DL_SIZE bytes)"
		else
			log "Remote file size too big: $remote_file_size bytes (> $MAX_FILE_DL_SIZE bytes)"
			run_sync=0
		fi
	else
		log "Skipping max file size check due to non-numeric remote size: '$remote_file_size'"
	fi

	##########################  We have all the required information, depending the choosen option we run the copy or not

	if [ "$sync_type" == "-o" ]; then # we overwrite the file if different
		log "option -o selected : we overwrite the file if different."
		if [ "$same_size" -ne 1 ] || [ "$same_chksum" -ne 1 ]; then
			[ -z "$run_sync" ] && run_sync=1
		fi
	fi

	if [ "$sync_type" == "-b" ]; then # backup
		log "option -b selected : we backup before overwrite."
		if { [ "$remote_file_checksum" != "0" ] && [ "$same_chksum" -ne 1 ]; } || [ "$remote_file_checksum" == "0" ]; then # if (we don't skip CRC and CRC is not identical) or (if we skip CRC) then we sync -> in other words we only skip the copy when we have an identical CRC
			[ -z "$run_sync" ] && run_sync=1
		fi
	fi

	if [ "$sync_type" == "-f" ]; then # we overwrite the file if different
		log "option -f selected : forced file syncing."
		if { [ "$remote_file_checksum" != "0" ] && [ "$same_chksum" -ne 1 ]; } || [ "$remote_file_checksum" == "0" ]; then # if (we don't skip CRC and CRC is not identical) or (if we skip CRC) then we sync -> in other words we only skip the copy when we have an identical CRC
			[ -z "$run_sync" ] && run_sync=1
		fi

	fi

	if [ "$sync_type" == "-c" ]; then # we overwrite the file if different
		log "option -c selected : no file copy, only check."
		run_sync=0

		if [ "$same_size" -ne 1 ]; then
			msg="Files doesn't have the same size."
			sync_success=0
			[ "$file_mandatory" = "-m" ] && cleanup

		fi
		if [ "$same_chksum" -ne 1 ]; then
			msg="$msg\nFiles doesn't have the same checksum."
			sync_success=0
			[ "$file_mandatory" = "-m" ] && cleanup

		fi
		if [ "$same_size" -eq 1 ] && [ "$same_chksum" -eq 1 ]; then
			msg="Remote and local files are identical."
			sync_success=1
		fi
		build_infoPanel_and_log "File check" "$msg"

	fi

	log "############################ DEBUGGING #######################################"
	echo sync_type $sync_type
	echo remote_file_checksum $remote_file_checksum
	echo same_chksum $same_chksum
	echo run_sync $run_sync
	echo file_type $file_type
	echo
	echo
	echo "##############################################################################"

	########################## COPY Operation ##########################

	if [ "$run_sync" -eq 1 ]; then

		if [ $file_type == "Cookie" ]; then # exception for cookies : we don't download with the same target name
			file_path="${file_path}.client"
		fi
		# let's make a backup first whatever the case
		if [ -e "$file_path" ]; then
			if [ $file_type == "Rom" ]; then
				# if rom already here and different file then we create a rom_neplay to avoid to override user games

				# oldpath="$file_path"
				# file_path_without_extension="${file_path%.*}"
				# file_path="${file_path_without_extension}_netplay.${file_path##*.}"
				# if [ -e "$Img_path" ]; then
				# 	cp "$Img_path" "/mnt/SDCARD/Roms/$romdirname/Imgs/${romNameNoExtension}_netplay.png"
				# 	Img_path="/mnt/SDCARD/Roms/$romdirname/Imgs/${romNameNoExtension}_netplay.png"
				# fi

				Netplay_Rom_Folder="$(dirname "$file_path")/.netplay"
				mkdir -p "$Netplay_Rom_Folder"
				file_path="$Netplay_Rom_Folder/$(basename "$file_path")"
				rom=$file_path

			elif [ $file_type == "Core" ]; then
				# oldpath="$file_path"
				# file_path_without_extension="${file_path%.*}"
				# file_path="${file_path_without_extension}_netplay.${file_path##*.}"

				Netplay_Core_Folder="$(dirname "$file_path")/.netplay"
				mkdir -p "$Netplay_Core_Folder"
				file_path="$Netplay_Core_Folder/$(basename "$file_path")"
				core=$file_path
			else
				mv "$file_path" "${file_path}_old"
				log "Existing $file_type file moved to ${file_path}_old"
			fi
		fi

		if [ ! -d "$dir_path" ]; then
			mkdir -p "$dir_path"
		fi

		log "Starting to download $file_type from $file_url"
		download_attempt=1
		while [ $download_attempt -le $NETPLAY_FTP_DOWNLOAD_RETRIES ]; do
			curl_output=$(curl -S -o "$file_path" "$file_url" 2>&1)
			curl_exit=$?

			if [ $curl_exit -eq 0 ]; then
				log "$file_type download completed"
				break
			fi

			log "$file_type download failed (curl exit=$curl_exit, attempt $download_attempt/$NETPLAY_FTP_DOWNLOAD_RETRIES)"
			log "curl error: $curl_output"
			if [ $curl_exit -eq 9 ] || [ $curl_exit -eq 78 ]; then
				log "FTP path denied for $file_url"
				break
			fi

			download_attempt=$((download_attempt + 1))
			sleep "$NETPLAY_FTP_DOWNLOAD_DELAY"
		done

	fi

	###################### FINAL CHECK RESULT #########################

	if [ ! -e "$file_path" ]; then
		if [ "$file_type" = "Img" ]; then
			log "Image not found on peer; continuing without image."
		else
			build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
		fi

		# copy has failed : restoring the original file
		if [ $file_type != "Rom" ] && [ $file_type != "Core" ]; then
			mv "${file_path}_old" "$file_path"
			log "backup restored"
		fi
		sleep 2
		if [ "$file_mandatory" = "-m" ]; then
			if [ "$file_type" != "Img" ]; then
				cleanup
			fi
		fi

	else

		checksum_func "$file_path" "$remote_file_checksum"

		if [ "$file_check_size" -eq 1 ]; then # file_checksum=0 means skip the difference check = always replace
			checksize_func "$file_path" "$remote_file_size"
		else
			log "Skipping file size check."
			same_size=1 # fake same size for skipping
		fi

		if [ "$same_size" -eq 1 ] && [ "$same_chksum" -eq 1 ]; then
			build_infoPanel_and_log "Syncing" "$file_type synced."
			if [ $file_type == "Rom" ]; then
				log "Refreshing roms list ${rom%/*}/${romdirname}_cache6.db"
				rm "${rom%/*}/${romdirname}_cache6.db"
			fi

		else
			build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."

			# copy has failed : restoring the original file
			if [ $file_type != "Rom" ] && [ $file_type != "Core" ]; then
				mv "${file_path}_old" "$file_path"
				log "backup restored"
			fi
			sleep 2
			[ "$file_mandatory" = "-m" ] && cleanup
		fi
		#####"

	fi
	###################### FINAL RESULT DISPLAY#########################

	# if [ "$sync_success" -ne 1 ]; then
	# 	build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
	# 	sleep 2
	# 	cleanup
	# else
	# 	build_infoPanel_and_log "Syncing" "$file_type synced."
	# fi

	# build_infoPanel_and_log "Syncing" "$file_type checksums don't match, syncing"
	# build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
	# build_infoPanel_and_log "Syncing" "$file_type synced."
	# build_infoPanel_and_log "Syncing" "$file_type doesn't exist locally; syncing with host."
	# build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
	# build_infoPanel_and_log "Syncing" "$file_type synced."

	################### END ##########################

}

# Start retroarch with -C in client mode if everything's gone to plan
start_retroarch() {
	build_infoPanel_and_log "Starting RA" "Starting RetroArch"

	log "RetroArch" "Starting RetroArch..."
	echo "*****************************************"
	echo "romfullpath: $rom"
	echo "platform: ${platform}"
	echo "netplaycore: $netplaycore"
	echo "core_config_folder: $core_config_folder"
	echo "cpuspeed: $cpuspeed"
	echo "*****************************************"

	if [ -n "$cpuspeed" ]; then
		log "We set core CPU speed for Netplay: $cpuspeed"
		PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
		echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi

	cd /mnt/SDCARD/RetroArch
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -C $hostip -L "$core" "$rom"

	if [ -n "$PreviousCPUspeed" ]; then
		log "We restore previous core CPU speed: $PreviousCPUspeed"
		echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
}

###########
#Utilities#
###########

# URL encode helper

wifi_disabled() {
	[ $(/customer/app/jsonval wifi) -eq 0 ]
}

confirm_join_panel() {
	local title="$1"
	local message="$2"

	if [ -e "$Img_path" ]; then
		pngScale "$Img_path" "/tmp/CurrentNetplay.png" 150 150
		sync
		imgpop 2 1 "/tmp/CurrentNetplay.png" 245 265 >/dev/null 2>&1 &
	fi
	infoPanel -t "$title" -m "$message"
	retcode=$?

	echo "retcode: $retcode"

	if [ $retcode -ne 0 ]; then
		build_infoPanel_and_log "Cancelled" "User cancelled, exiting."
		cleanup
		exit 1
	fi
}

stripped_game_name() {
	game_name=$(awk -F'/' '/\[rom\]:/ {print $NF}' /mnt/SDCARD/RetroArch/retroarch.cookie.client | sed 's/\(.*\)\..*/\1/')
}

cleanup() {
	build_infoPanel_and_log "Cleanup" "Cleaning up after netplay session..."

	pkill -9 pressMenu2Kill

	if is_running infoPanel; then
		killall -9 infoPanel
	fi
	rm /tmp/dismiss_info_panel

	sync

	# restore_wifi_state
	. "$sysdir/script/network/hotspot_cleanup.sh"

	rm "/mnt/SDCARD/RetroArch/retroarch.cookie.client"

	log "Cleanup done"
	sync
	exit
}

#########
##Main.##
#########

lets_go() {
	# Allow user to abort via menu while setup runs
	pressMenu2Kill $(basename $0) &

	# Join host hotspot
	. "$sysdir/script/network/hotspot_join.sh"

	# Fetch cookie from host
	sync_file "Cookie" "/mnt/SDCARD/RetroArch/retroarch.cookie" 0 0 -f -m

	# Read host cookie and parse paths/checksums
	read_cookie

	# Sync required core, rom, and image
	sync_file "Core" "$core" 1 "$corechecksum" -b -m
	sync_file "Rom" "$rom" 1 "$romchecksum" -b -m
	sync_file "Img" "$Img_path" 0 0 -o

	# Build display name for confirmation prompt
	stripped_game_name

	# Stop menu watcher before launch
	pkill -9 pressMenu2Kill

	# Confirm join with host info
	confirm_join_panel "Join now?" "$game_name"

	# Launch RetroArch client session
	start_retroarch

	# Cleanup and restore state
	cleanup
}

lets_go
