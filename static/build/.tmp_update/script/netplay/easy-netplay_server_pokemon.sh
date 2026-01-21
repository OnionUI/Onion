# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
. /mnt/SDCARD/.tmp_update/script/netplay/easy-netplay_env.sh

logfile=pokemon_link

# Source scripts
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running, restore_ftp, udhcpc_control, url_encode, strip_game_name, format_game_name, check_wifi, start_ftp, sync_file
. $sysdir/script/netplay/easy-netplay_common.sh
# easy-netplay_signalling.sh: wait_for_client, ready_up, notify_peer, check_stop, notify_stop
. $sysdir/script/netplay/easy-netplay_signalling.sh

program=$(basename "$0" .sh)

# Remove existing stop_now 
rm /tmp/stop_now

# Runtime vars
host_rom="$cookie_rom_path"
host_rom_filename=$(basename "$host_rom")
host_rom_filename_NoExt="${host_rom_filename%.*}"

netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/tgbdual_libretro.so"
SaveFromGambatte=0
export CurDate=$(date +%Y%m%d_%H%M%S)
log "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Easy Netplay Pokemon Host -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\ndate : $CurDate"

# Helper: capture quick state of a file for debugging save/rom transfer issues.
log_file_state() {
	local label="$1"
	local path="$2"
	if [ -f "$path" ]; then
		# Prefer stat, fall back to wc -c for size
		local size
		size=$(stat -c%s "$path" 2>/dev/null || wc -c <"$path")
		local md5
		md5=$(md5sum "$path" 2>/dev/null | awk '{print $1}')
		log "$label: exists size=${size} md5=${md5:-N/A} path=$path"
	else
		log "$label: missing path=$path"
	fi
}
##########
##Setup.##
##########

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

# Create a cookie with all the required info for the client. (client will use this cookie)
# Backup the save we're going to use before we do anythign else
host_save_backup() {

	# check_stop: client reported a setup/join issue
	check_stop "The client has had a problem joining the session."
	mkdir -p "/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual"
	save_gambatte="/mnt/SDCARD/Saves/CurrentProfile/saves/Gambatte/$host_rom_filename_NoExt.srm"
	save_tgbdual="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/$host_rom_filename_NoExt.srm"

	if [ -f "$save_gambatte" ]; then
		log "Gambatte save file detected."
		log "Backing up Gambatte save file to: $host_rom_filename_NoExt.srm_$Curdate"
		cp -f "$save_gambatte" "${save_gambatte}_$CurDate"
		log_file_state "Gambatte save (backed up)" "$save_gambatte"
		SaveFromGambatte=1
		if [ -f "$save_tgbdual" ]; then
			confirm_join_panel "Continue ?" "There is a local save for\n$host_rom_filename_NoExt\nfor TGB Dual and for Gambatte.\n Gambatte save will be used by default."
			log "Backing up the existing TGB Dual save file to: $host_rom_filename_NoExt.srm_$Curdate"
			cp -f "$save_tgbdual" "${save_tgbdual}_$CurDate"
			log_file_state "Existing TGB Dual save (backed up)" "$save_tgbdual"
		fi
		# copy save from Gambatte to TGB Dual
		cp -f "$save_gambatte" "$save_tgbdual"
	elif [ -f "$save_tgbdual" ]; then
		log "No Gambatte save file detected, using TGB Dual save file instead."
		log "Backing up current TGB Dual save file to: $host_rom_filename_NoExt.srm_$Curdate"
		cp -f "$save_tgbdual" "${save_tgbdual}_$CurDate"
		log_file_state "TGB Dual save (backed up)" "$save_tgbdual"
	fi

	sync
}

# The client will send us a save file, we'll pull the name from this, find it on the host and call duplicate_rename_rom - send to tmp
client_save_get() {
	# check_stop: client reported a setup/join issue
	check_stop "The client has had a problem joining the session."
	build_infoPanel_and_log "Setting up" "Setting up session \n Waiting for save files."

	client_save_file=""
	counter=0

	while true; do
		sync
		for file in /tmp/*.srm; do
			if [ -f "$file" ]; then
				log_file_state "Incoming client save candidate" "$file"
				if [ "$(basename "$file")" = "missing_save.srm" ]; then
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
	# check_stop: client reported a setup/join issue
	check_stop "The client has had a problem joining the session."
	if [ ! -z "$client_save_file" ]; then
		save_base_name=$(basename "$client_save_file" .srm)
		save_new_name="${save_base_name}_client.srm"
		save_new_path="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/$save_new_name"
		mv "$client_save_file" "$save_new_path"
		log "Save file found and processed - old save path:$client_save_file, new save path:$save_new_path "
		log_file_state "Client save after move" "$save_new_path"
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
	done <"$COOKIE_CLIENT_PATH"

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
	# check_stop: client reported a setup/join issue
	check_stop "The client has had a problem joining the session."

	rom_extension="${client_rom##*.}"
	client_rom_clone="${client_rom%.*}_client.$rom_extension"
	cp "$client_rom" "$client_rom_clone"
	if [ $? -eq 0 ]; then
		log "Successfully copied $client_rom to $client_rom_clone"
	else
		log "Failed to copy $client_rom to $client_rom_clone"
	fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {
	log "\n############################ RETROARCH DEBUGGING ############################"
	log "host_rom: $host_rom"
	log "client_rom_clone: ${client_rom_clone}"
	log "netplaycore: $netplaycore"
	log "cpuspeed: $cpuspeed"
	log_file_state "Host save (pre-RA)" "$save_tgbdual"
	log_file_state "Client save (pre-RA)" "$save_new_path"
	log "###############################################################################"

	if [ -n "$cpuspeed" ]; then
		log "We set core CPU speed for Netplay: $cpuspeed"
		PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
		echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi

	cd /mnt/SDCARD/RetroArch
	log "Starting RetroArch loaded with $host_rom and $client_rom_clone"
	# notify_peer: signal host ready to client
	(sleep 2 && notify_peer "$client_ip" "host_ready" &) &
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
	# check_stop: client reported a setup/join issue
	check_stop "The client has had a problem joining the session."
	local counter=0

	build_infoPanel_and_log "Syncing" "Waiting for client to be ready for save sync"
	# notify_peer: signal ready to send save
	notify_peer "$client_ip" "ready_to_send"
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
	# check_stop: client reported a setup/join issue
	check_stop "The client has had a problem joining the session."
	build_infoPanel_and_log "Syncing" "Returning client save..."
	received_save="tmp/$(basename "${save_new_path/_client/}")"
	encoded_path=$(url_encode "${received_save}")
	log "Returning client save: $save_new_path to ftp://$client_ip/${encoded_path}_rcvd"
	curl --connect-timeout 20 -T "$save_new_path" "ftp://$client_ip/${encoded_path}_rcvd" # the first / after the IP must be not encoded

	curl_exit_status=$?

	if [ $curl_exit_status -eq 0 ]; then
		build_infoPanel_and_log "Syncing" "Client save returned!"
		log_file_state "Save returned to client (local copy)" "$save_new_path"
	else
		build_infoPanel_and_log "Syncing" "Failed to return the client save \n Progress has likely been lost \n Curl exit code: $curl_exit_status"
		log "curl exit status while returning client save: $curl_exit_status"
	fi
}

# Cleanup. If you don't call this you don't retransfer the saves - Users cannot under any circumstances miss this function.
cleanup() {
	# message: cleanup infoPanel text
	# args: hotspot_cleanup restore_ftp kill_infopanel disable_hotspot_flag
	# remove files
	netplay_cleanup \
		"Cleaning up after Pokemon session\n Do not power off!" \
		1 1 0 1 \
		"/tmp/host_ready" \
		"$save_new_path" \
		"$client_rom_clone" \
		"/tmp/ready_to_send" \
		"/tmp/ready_to_receive" \
		"/tmp/missing_save.srm" \
		"/tmp/stop_now" \
		"/tmp/dismiss_info_panel"
}

###########
#Utilities#
###########

# Rename the new save back to the original one ready to be re-transferred
remove_client_save_suffix() {
	if [ ! -z "$save_new_path" ]; then
		save_base_name=$(basename "$save_new_path" _client.srm)
		original_name="${save_base_name}.srm"
		original_path="/tmp/$original_name"
		mv "$save_new_path" "$original_path"
	fi
}

# confirm_join_panel: show host confirmation UI with host/client images
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

# unpack_rom: extract archive into its folder
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

#########
##Main.##
#########

lets_go() {
	# Allow user to abort via menu while setup runs
	pressMenu2Kill $(basename $0) &

	# Ensure WiFi is up (udhcpc, hard fail, keep wlan1)
	# check_wifi: use udhcpc, hard fail, keep wlan1
	check_wifi 1 1 0

	# Write cookie with host metadata
	create_cookie_info

	# Create hotspot for client
	. "$sysdir/script/network/hotspot_create.sh"

	# start_ftp: preflight check_stop for client join issues
	start_ftp "The client has had a problem joining the session."

	# Wait for client connection
	wait_for_client
	wait_for_client_network

	# Send cookie to client (downloaded as retroarch.cookie.client)
	sync_file "Cookie" "$COOKIE_FILE" 0 0 -f -m

	# Read client cookie for display and params
	client_read_cookie

	# Backup host save and fetch client save
	host_save_backup
	client_save_get
	client_save_rename

	# Sync client ROM and prepare local clone
	sync_file "Rom" "$client_rom" 1 "$romchecksum" -b -m
	client_rom_rename

	# Sync client image for confirmation display
	sync_file "Img" "$client_Img_path" 0 0 -o

	# Build display names for confirmation prompt
	host_game_name=$(format_game_name "$(basename "${host_rom%.*}")" "Host (me)")
	client_game_name=$(format_game_name "$(basename "${client_rom%.*}")" "Client" "\n ")

	# Confirm host start with host/client info
	confirm_join_panel "Host now?" "$host_game_name \n $client_game_name"

	# Stop menu watcher before launch
	pkill -9 pressMenu2Kill

	# Launch RetroArch host session
	start_retroarch

	# Restore saves and send back to client
	host_save_overwrite
	client_wait_for_save_return
	client_save_send

	# Cleanup and restore state
	cleanup
}

lets_go
