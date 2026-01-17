# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

logfile=pokemon_link
# Source scripts
. $sysdir/script/log.sh
. $sysdir/script/netplay/netplay_common.sh

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

# We'll need FTP for lightweight signaling - use the built in FTP, it allows us to curl (errors on bftpd re: path)
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

# Wait for a hit on the sta list for someone joining the hotspot

# Tell the client we're ready to accept connections

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
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
	(sleep 2 && notify_peer "host_ready" &) &
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -H -v -L .retroarch/cores/gpsp_libretro.so "$host_rom"

	if [ -n "$PreviousCPUspeed" ]; then
		log "We restore previous core CPU speed: $PreviousCPUspeed"
		echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
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
	rm "/tmp/stop_now"
	disable_flag hotspotState
	rm "/tmp/dismiss_info_panel"
	sync
	log "Cleanup done"
	
	#Rename savestate_auto_load so savestate doesn't overwrite next loadsave
	mv -f "/mnt/SDCARD/Saves/CurrentProfile/states/gpSP/$host_rom_filename_NoExt.state.auto" "/mnt/SDCARD/Saves/CurrentProfile/states/gpSP/$host_rom_filename_NoExt.state.auto_$CurDate"
	
	exit
}

###########
#Utilities#
###########

# Use the safe word

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

stripped_game_names() {
	host_game_name="$(basename "${host_rom%.*}")"
	host_game_name="$(echo "$host_game_name" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
	host_game_name="Host (me): \n$host_game_name"
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






#########
##Main.##
#########

lets_go() {
	pressMenu2Kill $(basename $0) &
	if [ -z "$host_rom" ]; then
		build_infoPanel_and_log "Error" "No ROM path provided."
		exit 1
	fi
	. "$sysdir/script/network/hotspot_create.sh"
	start_ftp
	wait_for_client
	ready_up
	stripped_game_names
	confirm_join_panel "Host now?" "$host_game_name"
	pkill -9 pressMenu2Kill
	start_retroarch
	cleanup
}

lets_go
