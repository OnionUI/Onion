# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

logfile=pokemon_link
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

rm /tmp/stop_now
host_rom="$cookie_rom_path"
host_rom_filename=$(basename "$host_rom")
host_rom_filename_NoExt="${host_rom_filename%.*}"

netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/tgbdual_libretro.so"
SaveFromGambatte=0
export CurDate=$(date +%Y%m%d_%H%M%S)
log "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Easy Netplay Pokemon Host -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\ndate : $CurDate"
##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..
check_wifi() {
	if ifconfig wlan0 &>/dev/null; then
		build_infoPanel_and_log "WIFI" "Wifi up"
	else
		build_infoPanel_and_log "WIFI" "Wifi disabled, starting..."

		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		sleep 1
		$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
		udhcpc_control

		if is_running wpa_supplicant && ifconfig wlan0 >/dev/null 2>&1; then
			build_infoPanel_and_log "WIFI" "Wifi started."
		else
			build_infoPanel_and_log "WIFI" "Unable to start WiFi\n unable to continue."
			notify_stop
		fi

		sleep 2
	fi
}

# We'll need hotspot to host the local connection
start_hotspot() {
	build_infoPanel_and_log "Hotspot" "Starting hotspot..."
	if is_running hostapd; then
		killall -9 hostapd
	fi
	if is_running dnsmasq; then
		killall -9 dnsmasq
	fi

	enable_flag hotspotState
	$sysdir/script/network/update_networking.sh hotspot toggle
}

# We'll need FTP to host the cookie to the client - use the built in FTP, it allows us to curl (errors on bftpd re: path)
start_ftp() {
	check_stop
	if is_running bftpd; then
		log "FTP already running, killing to rebind"
		bftpd_p=$(ps | grep bftpd | grep -v grep | awk '{for(i=4;i<=NF;++i) printf $i" "}')
		killall -9 bftpd
		killall -9 tcpsvd
		tcpsvd -E 0.0.0.0 21 ftpd -w / &
	else
		tcpsvd -E 0.0.0.0 21 ftpd -w / &
		log "Starting FTP server"
	fi
}

# Create a cookie with all the required info for the client. (client will use this cookie)
create_cookie_info() {
	COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"
	MAX_FILE_SIZE_BYTES=26214400

	echo "[core]: $netplaycore" >"$COOKIE_FILE"
	echo "[rom]: $cookie_rom_path" >>"$COOKIE_FILE"

	if [ -s "$netplaycore" ]; then
		log "Writing core size"
		core_size=$(stat -c%s "$netplaycore")
		if [ "$core_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
			echo "[corechksum]: 0" >>"$COOKIE_FILE"
		else
			echo "[corechksum]: $(xcrc "$netplaycore")" >>"$COOKIE_FILE"
		fi
	fi

	if [ -s "$cookie_rom_path" ]; then
		rom_size=$(stat -c%s "$cookie_rom_path")
		log "Cookie local rom size : $rom_size"
		if [ "$rom_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
			echo "[romchksum]: 0" >>"$COOKIE_FILE"
		else
			echo "[romchksum]: $(xcrc "$cookie_rom_path")" >>"$COOKIE_FILE"
		fi
	fi

	if [ -s "$cpuspeed" ]; then
		echo "[cpuspeed]: $cpuspeed" >>"$COOKIE_FILE"
		log "Writing custom cpuspeed: $cpuspeed"
	fi

}

# Wait for a hit on the sta list for someone joining the hotspot
wait_for_client() {
	check_stop
	build_infoPanel_and_log "Hotspot" "Waiting for a client to connect..."

	client_ip=""
	client_mac=""
	counter=0

	killall -9 wpa_supplicant
	killall -9 udhcpc

	sleep 1

	while true; do
		sta_list=$($sysdir/bin/hostapd_cli all_sta 2>/dev/null)
		$sysdir/bin/hostapd_cli all_sta flush

		if [ $? -ne 0 ]; then
			build_infoPanel_and_log "Hotspot" "Hostapd hook failing, retrying."
			counter=$((counter + 1))
		fi

		if [ ! -z "$sta_list" ]; then
			client_mac=$(echo "$sta_list" | awk 'NR==2{print $1; exit}')
			client_ip=$(arp -an | awk '/'"$client_mac"'/ {gsub(/[\(\)]/,""); print $2}')

			if [ ! -z "$client_ip" ]; then
				case "$client_ip" in
				192.168.100.*)
					log "$sta_list"
					log "A client has connected. IP: $client_ip"
					build_infoPanel_and_log "Hotspot" "A client has connected! \n IP: $client_ip"
					break
					;;
				esac
			fi
		fi

		sleep 1
		counter=$((counter + 1))

		if [ $counter -ge 30 ]; then
			log "No client has connected"
			build_infoPanel_and_log "Hotspot error" "No client has connected. Exiting..."
			cleanup
		fi
	done

	sleep 1
	log "$client_ip has joined the hotspot"
}

# Backup the save we're going to use before we do anythign else
host_save_backup() {

	check_stop
	mkdir -p "/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual"
	save_gambatte="/mnt/SDCARD/Saves/CurrentProfile/saves/Gambatte/$host_rom_filename_NoExt.srm"
	save_tgbdual="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/$host_rom_filename_NoExt.srm"

	if [ -f "$save_gambatte" ]; then
		log "Gambatte save file detected."
		log "Backing up Gambatte save file to: $host_rom_filename_NoExt.srm_$Curdate"
		cp -f "$save_gambatte" "${save_gambatte}_$CurDate"
		SaveFromGambatte=1
		if [ -f "$save_tgbdual" ]; then
			confirm_join_panel "Continue ?" "There is a local save for\n$host_rom_filename_NoExt\nfor TGB Dual and for Gambatte.\n Gambatte save will be used by default."
			log "Backing up the existing TGB Dual save file to: $host_rom_filename_NoExt.srm_$Curdate"
			cp -f "$save_tgbdual" "${save_tgbdual}_$CurDate"
		fi
		# copy save from Gambatte to TGB Dual
		cp -f "$save_gambatte" "$save_tgbdual"
	elif [ -f "$save_tgbdual" ]; then
		log "No Gambatte save file detected, using TGB Dual save file instead."
		log "Backing up current TGB Dual save file to: $host_rom_filename_NoExt.srm_$Curdate"
		cp -f "$save_tgbdual" "${save_tgbdual}_$CurDate"
	fi

	sync
}

# The client will send us a save file, we'll pull the name from this, find it on the host and call duplicate_rename_rom - send to tmp
client_save_get() {
	check_stop
	build_infoPanel_and_log "Setting up" "Setting up session \n Waiting for save files."

	client_save_file=""
	counter=0

	while true; do
		sync
		for file in /tmp/*.srm; do
			if [ -f "$file" ]; then
				if [ "$(basename "$file")" = "MISSING.srm" ]; then
					build_infoPanel_and_log "Sync error" "Client advises they don't have a save file. \n Cannot continue."
					notify_stop
				else
					client_save_file=$file
				fi
				build_infoPanel_and_log "Synced!" "Received save from client"
				break 2
			fi
		done

		sleep 1
		counter=$((counter + 1))

		if [ $counter -ge 25 ]; then
			build_infoPanel_and_log "Sync error" "No save file was received. Exiting..."
			notify_stop
		fi
	done
}

# Prep the clients save file
client_save_rename() {
	check_stop
	if [ ! -z "$client_save_file" ]; then
		save_base_name=$(basename "$client_save_file" .srm)
		save_new_name="${save_base_name}_client.srm"
		save_new_path="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/$save_new_name"
		mv "$client_save_file" "$save_new_path"
		log "Save file found and processed - old save path:$client_save_file, new save path:$save_new_path "
		sync
	else
		build_infoPanel_and_log "Syncing" "Save file not found, cannot continue"
		cleanup
	fi
}

# Read the cookie and store the paths and checksums into a var.
client_read_cookie() {
	sync
	while IFS= read -r line; do
		case $line in
		"[core]: "*)
			core="${line##"[core]: "}"
			;;
		"[rom]: "*)
			client_rom="${line##"[rom]: "}"
			;;
		"[coresize]: "*)
			corechecksum="${line##"[coresize]: "}"
			;;
		"[corechksum]: "*)
			corechecksum="${line##"[corechksum]: "}"
			;;
		"[romsize]: "*)
			romchecksum="${line##"[romsize]: "}"
			;;
		"[romchksum]: "*)
			romchecksum="${line##"[romchksum]: "}"
			;;
		"[cpuspeed]: "*)
			cpuspeed="${line##"[cpuspeed]: "}"
			;;
		esac
		log "$core $rom $coresize $corechksum $romsize $romchksum"
	done <"/mnt/SDCARD/RetroArch/retroarch.cookie.client"

	#url encode or curl complains
	export core_url=$(url_encode "$core")
	export rom_url=$(url_encode "$rom")

	romdirname=$(echo "$client_rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
	romName=$(basename "$client_rom")
	romNameNoExtension=${romName%.*}
	client_Img_path="/mnt/SDCARD/Roms/$romdirname/Imgs/$romNameNoExtension.png"
	log "Cookie file read :"
	log "client romdirname $romdirname"
	log "client romName $romName"
	log "client romNameNoExtension $romNameNoExtension"
	log "client Img_path $Img_path"
}

# Duplicate the rom to spoof the save loaded in on the host
client_rom_rename() {
	check_stop

	rom_extension="${client_rom##*.}"
	client_rom_clone="${client_rom%.*}_client.$rom_extension"
	cp "$client_rom" "$client_rom_clone"
	if [ $? -eq 0 ]; then
		log "Successfully copied $client_rom to $client_rom_clone"
	else
		log "Failed to copy $client_rom to $client_rom_clone"
	fi
}

# Tell the client we're ready to accept connections
ready_up() {
	check_stop
	ping -c 5 $client_ip >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		notify_peer "host_ready"
	else
		build_infoPanel_and_log "Error" "No connectivity to $client_ip, \n is the client still connected?"
		notify_stop
	fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {

	log "\n############################ RETROARCH DEBUGGING ############################"
	log "host_rom: $host_rom"
	log "client_rom_clone: ${client_rom_clone}"
	log "netplaycore: $netplaycore"
	log "cpuspeed: $cpuspeed"
	log "###############################################################################"

	if [ -n "$cpuspeed" ]; then
		log "We set core CPU speed for Netplay: $cpuspeed"
		PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
		echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi

	cd /mnt/SDCARD/RetroArch
	log "Starting RetroArch loaded with $host_rom and $client_rom_clone"
	(sleep 2 && notify_peer "host_ready" &) &
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -H -v -L .retroarch/cores/tgbdual_libretro.so --subsystem "gb_link_2p" "$host_rom" "$client_rom_clone"

	if [ -n "$PreviousCPUspeed" ]; then
		log "We restore previous core CPU speed: $PreviousCPUspeed"
		echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
}

# Restore gambatte save from TGB Dual
host_save_overwrite() {
	if [ $SaveFromGambatte -eq 1 ]; then
		mv -f "$save_tgbdual" "$save_gambatte"
		mv -f "/mnt/SDCARD/Saves/CurrentProfile/states/Gambatte/$host_rom_filename_NoExt.state.auto" "/mnt/SDCARD/Saves/CurrentProfile/states/Gambatte/$host_rom_filename_NoExt.state.auto_$CurDate"
		if [ -f "${save_tgbdual}_$CurDate" ]; then # We restore the previous tgbdual save to keep it intact.
			mv -f "${save_tgbdual}_$CurDate" "$save_tgbdual"
		fi
	else
		mv -f "/mnt/SDCARD/Saves/CurrentProfile/states/TGB Dual/$host_rom_filename_NoExt.state.auto" "/mnt/SDCARD/Saves/CurrentProfile/states/TGB Dual/$host_rom_filename_NoExt.state.auto_$CurDate"
	fi
}

# Go into a waiting state for the client to be ready to accept the save
client_wait_for_save_return() {
	check_stop
	local counter=0

	build_infoPanel_and_log "Syncing" "Waiting for client to be ready for save sync"
	notify_peer "ready_to_send"
	sync

	while true; do
		for file in /tmp/ready_to_receive; do
			if [ -f "$file" ]; then
				build_infoPanel_and_log "Message from client" "Client is ready for the save"
				break 2
			fi
		done

		sleep 1
		counter=$((counter + 1))

		if [ $counter -ge 30 ]; then
			build_infoPanel_and_log "Error" "The client didn't ready up, cannot continue..."
			sleep 1
			cleanup
			break
		fi
	done
}

# Push the clients save file back
client_save_send() {
	check_stop
	build_infoPanel_and_log "Syncing" "Returning client save..."
	received_save="tmp/$(basename "${save_new_path/_client/}")"
	encoded_path=$(url_encode "${received_save}")
	log "Returning client save: $save_new_path to ftp://$client_ip/${encoded_path}_rcvd"
	curl --connect-timeout 20 -T "$save_new_path" "ftp://$client_ip/${encoded_path}_rcvd" # the first / after the IP must be not encoded

	curl_exit_status=$?

	if [ $curl_exit_status -eq 0 ]; then
		build_infoPanel_and_log "Syncing" "Client save returned!"
	else
		build_infoPanel_and_log "Syncing" "Failed to return the client save \n Progress has likely been lost \n Curl exit code: $curl_exit_status"
	fi
}

# Cleanup. If you don't call this you don't retransfer the saves - Users cannot under any circumstances miss this function.
cleanup() {
	build_infoPanel_and_log "Cleanup" "Cleaning up after Pokemon session\n Do not power off!"

	pkill -9 pressMenu2Kill

	. "$sysdir/script/network/hotspot_cleanup.sh"

	restore_ftp

	# Remove some files we prepared and received
	rm "/tmp/host_ready"
	rm "$save_new_path"
	rm "$client_rom_clone"
	rm "/tmp/ready_to_send"
	rm "/tmp/ready_to_receive"
	rm "/tmp/MISSING.srm"
	rm "/tmp/stop_now"
	disable_flag hotspotState
	rm "/tmp/dismiss_info_panel"
	sync
	log "Cleanup done"
	exit
}

###########
#Utilities#
###########

# URL encode helper
url_encode() {
	encoded_str=$(echo "$*" | awk '
    BEGIN {
	split ("1 2 3 4 5 6 7 8 9 A B C D E F", hextab, " ")
	hextab [0] = 0
	for ( i=1; i<=255; ++i ) ord [ sprintf ("%c", i) "" ] = i + 0
    }
    {
	encoded = ""
	for ( i=1; i<=length ($0); ++i ) {
	    c = substr ($0, i, 1)
	    if ( c ~ /[a-zA-Z0-9.-]/ ) {
		encoded = encoded c		# safe character
	    } else if ( c == " " ) {
		encoded = encoded "%20"	# special handling
	    } else {
		# unsafe character, encode it as a two-digit hex-number
		lo = ord [c] % 16
		hi = int (ord [c] / 16);
		encoded = encoded "%" hextab [hi] hextab [lo]
	    }
	}
	    print encoded
    }
')
	echo "$encoded_str"
}

# Use the safe word
notify_stop() {
	notify_peer "stop_now"
	sleep 2
	cleanup
}

# Check stop, if the client tells us to stop we will.
check_stop() {
	sync
	if [ -e "/tmp/stop_now" ]; then
		build_infoPanel_and_log "Message from client" "The client has had a problem joining the session."
		sleep 2
		cleanup
	fi
}

notify_peer() {
	local notify_file="/tmp/$1"
	touch "$notify_file"
	sync
	curl -T "$notify_file" "ftp://${client_ip}/${notify_file}" >/dev/null 2>&1 # the first / after the IP must be not encoded

	if [ $? -eq 0 ]; then
		log "Successfully transferred $notify_file to ftp://${client_ip}/${notify_file}"
	else
		log "Failed to transfer $notify_file to ftp://${client_ip}/${notify_file}"
	fi
}

# Rename the new save back to the original one ready to be re-transferred
remove_client_save_suffix() {
	if [ ! -z "$save_new_path" ]; then
		save_base_name=$(basename "$save_new_path" _client.srm)
		original_name="${save_base_name}.srm"
		original_path="/tmp/$original_name"
		mv "$save_new_path" "$original_path"
	fi
}

confirm_join_panel() {
	local title="$1"
	local message="$2"

	sync

	if [ -e "/tmp/stop_now" ]; then
		build_infoPanel_and_log "Message from client" "The client advises they don't have this rom \n Can't continue."
		sleep 2
		cleanup
	fi

	if [ "$title" = "Host now?" ]; then
		if [ -e "$client_Img_path" ]; then # local rom image
			pngScale "$client_Img_path" "/tmp/CurrentNetplay.png" 100 100
			sync
			imgpop 3 2 "/tmp/CurrentNetplay.png" 10 10 >/dev/null 2>&1 &
		fi

		if [ -e "/mnt/SDCARD/Roms/$romdirname/Imgs/$host_rom_filename_NoExt.png" ]; then # remote rom image
			pngScale "/mnt/SDCARD/Roms/$romdirname/Imgs/$host_rom_filename_NoExt.png" "/tmp/CurrentNetplay2.png" 100 100
			sync
			imgpop 2 1 "/tmp/CurrentNetplay2.png" 530 300 >/dev/null 2>&1 &
		fi
	fi

	infoPanel -t "$title" -m "$message"
	retcode=$?

	if [ $retcode -ne 0 ]; then
		build_infoPanel_and_log "Cancelled" "User cancelled, exiting."
		cleanup
		exit 1
	fi
}

stripped_game_names() {
	host_game_name="$(basename "${host_rom%.*}")"
	host_game_name="$(echo "$host_game_name" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
	host_game_name="Host (me): \n$host_game_name"

	client_rom_trimmed="$(basename "${client_rom%.*}" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
	client_game_name="\n Client: \n$client_rom_trimmed"
}

# Function to sync files

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
	file_url="ftp://${client_ip}/$(url_encode "${file_path#*/}")"

	log "\n############################ SYNC_FILE DEBUGGING ############################"
	log file_type $file_type
	log file_path $file_path
	log file_check_size $file_check_size
	log remote_file_checksum $remote_file_checksum
	log sync_type $sync_type
	log file_mandatory $file_mandatory
	log file_url $file_url
	log dir_path $dir_path
	log "#############################################################################"

	# state vars
	same_size=
	same_chksum=
	sync_success=
	run_sync= # tell if the sync task must be done or not

	RequestResult=$(curl -I "$file_url" 2>&1)

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
			log "Impossible to get remote file size."
			same_size=0
			run_sync=0
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
	if [ "$remote_file_size" -le "$MAX_FILE_DL_SIZE" ]; then
		log "Remote file size ok: $remote_file_size bytes  (<= $MAX_FILE_DL_SIZE bytes)"
	else
		log "Remote file size too big: $remote_file_size bytes (> $MAX_FILE_DL_SIZE bytes)"
		run_sync=0
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
	log sync_type $sync_type
	log remote_file_checksum $remote_file_checksum
	log same_chksum $same_chksum
	log run_sync $run_sync
	log file_type $file_type
	log
	log
	log "##############################################################################"

	########################## COPY Operation ##########################

	if [ "$run_sync" -eq 1 ]; then

		# let's make a backup first whatever the case
		if [ -e "$file_path" ]; then
			if [ $file_type == "Rom" ]; then
				# if rom already here and different file then we create a rom_neplay to avoid to override user games

				# oldpath="$file_path"
				# file_path_without_extension="${file_path%.*}"
				# file_path="${file_path_without_extension}_netplay.${file_path##*.}"
				# if [ -e "$client_Img_path" ]; then
				# 	cp "$client_Img_path" "/mnt/SDCARD/Roms/$romdirname/Imgs/${romNameNoExtension}_netplay.png"
				# 	client_Img_path="/mnt/SDCARD/Roms/$romdirname/Imgs/${romNameNoExtension}_netplay.png"
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
		if [ $file_type == "Cookie" ]; then # exception for cookies : we don't download with the same target name
			file_path="${file_path}.client"
		fi

		if [ ! -d "$dir_path" ]; then
			mkdir -p "$dir_path"
		fi

		log "Starting to download $file_type from $file_url"
		curl -o "$file_path" "$file_url" >/dev/null 2>&1

		if [ $? -eq 0 ]; then
			log "$file_type download completed"
		else
			log "$file_type download failed"
		fi

	fi

	###################### FINAL CHECK RESULT #########################

	if [ ! -e "$file_path" ]; then
		build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."

		# copy has failed : restoring the original file
		if [ $file_type != "Rom" ] && [ $file_type != "Core" ]; then
			mv "${file_path}_old" "$file_path"
			log "backup restored"
		fi
		sleep 2
		[ "$file_mandatory" = "-m" ] && cleanup

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
			if [ "$file_mandatory" = "-m" ]; then
				notify_peer "stop_now"
				cleanup
			fi
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

checksum_func() {
	local_file_size=$(stat -c%s "$file_path")
	local func_file_path="$1"
	local CRC="$2"

	########################## File checksum check : same_chksum = 0 different, 1 identical , 2 unknown

	if [ "$CRC" != "0" ]; then # file_checksum=0 means skip the difference check = always replace
		local_file_checksum=$(xcrc "$func_file_path")

		if [ "$local_file_size" -gt "$MAX_FILE_CHKSUM_SIZE" ]; then
			log "File size too big for checksum: it would be too long"
			same_chksum=2
		else
			if [ "$CRC" == "$local_file_checksum" ]; then
				same_chksum=1
			else
				same_chksum=0
			fi
		fi
	else
		log "Skipping checksum check."
		same_chksum=1 # fake same size for skipping
	fi
}

checksize_func() {

	local func_file_path="$1"
	local filesize_tocheck="$2"
	local_file_size=$(stat -c%s "$func_file_path")

	########################## File size check   same_size = 0 different, 1 identical , 2 unknown

	if echo "$filesize_tocheck" | grep -q "^[0-9][0-9]*$"; then # check if the remote file size is a numeric value
		if [ "$filesize_tocheck" -eq "$local_file_size" ]; then
			log "Same size as remote"
			same_size=1
		else
			log "Files size are different"
			same_size=0
		fi
	else
		log "Impossible to get file size : wrong parameter."
		same_size=2
	fi
}

unpack_rom() {
	file="$1"
	folder=$(dirname "$file")
	extension="${file##*.}"

	if [ -f "$file" ]; then
		7z x "$file" -o"$folder" >/dev/null 2>&1
	else
		log "File '$file' not found - cannot continue"
	fi
}

build_infoPanel_and_log() {
	local title="$1"
	local message="$2"

	log "Info Panel: \n\tStage: $title\n\tMessage: $message"
	if is_running infoPanel; then
		killall -9 infoPanel
	fi
	infoPanel --title "$title" --message "$message" --persistent &
	sync
	touch /tmp/dismiss_info_panel
	sync
	sleep 0.5
	sync
}

restore_ftp() {
	log "Restoring original FTP server"
	killall -9 tcpsvd
	if flag_enabled ftpState; then
		if flag_enabled authftpState; then
			bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpdauth.conf &
		else
			bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf &
		fi
	fi
}

flag_enabled() {
	flag="$1"
	[ -f "$sysdir/config/.$flag" ]
}

udhcpc_control() {
	if pgrep udhcpc >/dev/null; then
		killall -9 udhcpc
	fi
	sleep 1
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
}

is_running() {
	process_name="$1"
	pgrep "$process_name" >/dev/null
}

enable_flag() {
	flag="$1"
	touch "$sysdir/config/.$flag"
}

#########
##Main.##
#########

lets_go() {
	pressMenu2Kill $(basename $0) &
	create_cookie_info
	. "$sysdir/script/network/hotspot_create.sh"
	start_ftp
	wait_for_client
	sync_file "Cookie" "/mnt/SDCARD/RetroArch/retroarch.cookie" 0 0 -f -m # will be downloaded as retroarch.cookie.client !
	client_read_cookie
	host_save_backup
	client_save_get
	client_save_rename
	sync_file "Rom" "$client_rom" 1 "$romchecksum" -b -m
	client_rom_rename
	ready_up
	sync_file "Img" "$client_Img_path" 0 0 -o
	stripped_game_names
	confirm_join_panel "Host now?" "$host_game_name \n $client_game_name"
	pkill -9 pressMenu2Kill
	start_retroarch
	host_save_overwrite
	client_wait_for_save_return
	client_save_send
	cleanup
}

lets_go
