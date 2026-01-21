# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
. /mnt/SDCARD/.tmp_update/script/netplay/easy-netplay_env.sh

logfile=pokemon_link

# Source scripts
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running, restore_ftp, udhcpc_control, url_encode, strip_game_name, format_game_name, check_wifi, start_ftp
. $sysdir/script/netplay/easy-netplay_common.sh
# easy-netplay_signalling.sh: wait_for_client, ready_up, notify_peer, check_stop, notify_stop
. $sysdir/script/netplay/easy-netplay_signalling.sh

# Runtime vars
rm /tmp/stop_now
host_rom="$1"
romdirname=$(echo "$host_rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
host_rom_filename=$(basename "$host_rom")
host_rom_filename_NoExt="${host_rom_filename%.*}"

netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/gpsp_libretro.so"
export CurDate=$(date +%Y%m%d_%H%M%S)
log "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Easy Netplay Pokemon Host GBA -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\ndate : $CurDate"
##########
##Setup.##
##########

# start_retroarch: launch RetroArch in host mode with the local ROM
start_retroarch() {

	log "\n############################ RETROARCH DEBUGGING ############################"
	log "host_rom: $host_rom"
	log "netplaycore: $netplaycore"
	log "cpuspeed: $cpuspeed"
	log "###############################################################################"

	if [ -n "$cpuspeed" ]; then
		log "We set core CPU speed for Netplay: $cpuspeed"
		PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
		echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi

	cd /mnt/SDCARD/RetroArch
	log "Starting RetroArch loaded with $host_rom"
	# notify_peer: signal host ready to client
	(sleep 2 && notify_peer "$client_ip" "host_ready" &) &
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -H -v -L .retroarch/cores/gpsp_libretro.so "$host_rom"

	if [ -n "$PreviousCPUspeed" ]; then
		log "We restore previous core CPU speed: $PreviousCPUspeed"
		echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
}

# cleanup: restore network/ftp and clean temp files
cleanup() {
	# Rename savestate_auto_load so savestate doesn't overwrite next loadsave
	mv -f "/mnt/SDCARD/Saves/CurrentProfile/states/gpSP/$host_rom_filename_NoExt.state.auto" "/mnt/SDCARD/Saves/CurrentProfile/states/gpSP/$host_rom_filename_NoExt.state.auto_$CurDate"

	# message: cleanup infoPanel text
	# args: hotspot_cleanup restore_ftp kill_infopanel disable_hotspot_flag
	# remove files
	netplay_cleanup \
		"Cleaning up after Pokemon session\n Do not power off!" \
		1 1 0 1 \
		"/tmp/host_ready" \
		"/tmp/stop_now" \
		"/tmp/dismiss_info_panel"
}

# confirm_join_panel: show host confirmation UI with local ROM image
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

#########
##Main.##
#########

# lets_go: main flow for hosting GBA session
lets_go() {
	# Allow user to abort via menu while setup runs
	pressMenu2Kill $(basename $0) &

	# Ensure ROM path is provided
	if [ -z "$host_rom" ]; then
		build_infoPanel_and_log "Error" "No ROM path provided."
		exit 1
	fi

	# Create hotspot for client
	. "$sysdir/script/network/hotspot_create.sh"

	# Start FTP with preflight stop check
	start_ftp "The client has had a problem joining the session."

	# Wait for client connection
	wait_for_client

	# Build display names for confirmation prompt
	host_game_name=$(format_game_name "$(basename "${host_rom%.*}")" "Host (me)")

	# Confirm host start with local ROM display
	confirm_join_panel "Host now?" "$host_game_name"

	# Stop menu watcher before launch
	pkill -9 pressMenu2Kill

	# Launch RetroArch host session
	start_retroarch

	# Cleanup and restore state
	cleanup
}

lets_go
