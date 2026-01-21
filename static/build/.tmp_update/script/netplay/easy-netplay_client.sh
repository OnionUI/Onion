# GLO CLIENT
# Script to:
# Enable Wifi
# Pull the cookie file from the host
# Pull the core from the host based on what the cookie file gives us
# Start retroarch with the above passed, and -C provided to connect.

# Used within GLO as an addon script.

# Env setup
# easy-netplay_env.sh: sysdir, miyoodir, LD_LIBRARY_PATH, hostip, peer_ip
. /mnt/SDCARD/.tmp_update/script/netplay/easy-netplay_env.sh

logfile=easy_netplay

# Source scripts
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running, restore_ftp,
# udhcpc_control, url_encode, strip_game_name, format_game_name, wifi_disabled, read_cookie, check_wifi, start_ftp, sync_file
. $sysdir/script/netplay/easy-netplay_common.sh

program=$(basename "$0" .sh)

##########
##Setup.##
##########

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

# confirm_join_panel: show join confirmation UI with ROM image
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

# cleanup: restore wifi/ftp and remove session temp files
cleanup() {
	# message: cleanup infoPanel text
	# args: hotspot_cleanup restore_ftp kill_infopanel disable_hotspot_flag
	# remove files
	netplay_cleanup \
		"Cleaning up after netplay session..." \
		1 0 1 0 \
		"/tmp/dismiss_info_panel" \
		"$COOKIE_CLIENT_PATH"
}

#########
##Main.##
#########

lets_go() {
	# Allow user to abort via menu while setup runs
	pressMenu2Kill $(basename $0) &

	# Join host hotspot
	. "$sysdir/script/network/hotspot_join.sh"
	build_infoPanel_and_log "Connected" "Client IP: ${IP:-unknown}\nHost IP: $hostip"

	# Fetch cookie from host
	sync_file "Cookie" "$COOKIE_FILE" 0 0 -f -m

	# Read host cookie and parse paths/checksums
	read_cookie

	# Sync required core, rom, and image
	sync_file "Core" "$core" 1 "$corechecksum" -b -m
	sync_file "Rom" "$rom" 1 "$romchecksum" -b -m
	sync_file "Img" "$Img_path" 0 0 -o

	# Build display name for confirmation prompt
	game_name=$(format_game_name "$(basename "${rom%.*}")")

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
