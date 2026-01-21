# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
. /mnt/SDCARD/.tmp_update/script/netplay/easy-netplay_env.sh

# Runtime vars
rm /tmp/stop_now
client_rom="$1"
romdirname=$(echo "$client_rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
client_rom_filename=$(basename "$client_rom")
client_rom_filename_NoExt="${client_rom_filename%.*}"
netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/gpsp_libretro.so"

logfile=pokemon_link

# Source scripts
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running, restore_ftp, udhcpc_control, url_encode, strip_game_name, format_game_name, check_wifi, start_ftp
. $sysdir/script/netplay/easy-netplay_common.sh
# easy-netplay_signalling.sh: wait_for_host, check_stop, notify_stop
. $sysdir/script/netplay/easy-netplay_signalling.sh

log "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Easy Netplay Pokemon Client GBA -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

##########
##Setup.##
##########

# start_retroarch: launch RetroArch in client mode with local ROM
start_retroarch() {
    build_infoPanel_and_log "Starting RA" "Starting RetroArch"

    log "\n############################ RETROARCH DEBUGGING ############################"
    log "client_rom (local): ${client_rom}"
    log "core: $netplaycore"
    log "core_config_folder: $core_config_folder"
    log "cpuspeed: $cpuspeed"
    log "hostip: $hostip"
    log "###############################################################################"

    if [ -n "$cpuspeed" ]; then
        log "We set core CPU speed for Netplay: $cpuspeed"
        PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
        echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    fi

    cd /mnt/SDCARD/RetroArch
    log "Starting RetroArch loaded with $client_rom"
    HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -C $hostip -L "$netplaycore" "$client_rom"

    if [ -n "$PreviousCPUspeed" ]; then
        log "We restore previous core CPU speed: $PreviousCPUspeed"
        echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    else
        rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    fi
}

# cleanup: restore network/ftp and clean temp files
cleanup() {
    log "Removing stale files"
    # message: cleanup infoPanel text
    # args: hotspot_cleanup restore_ftp kill_infopanel disable_hotspot_flag
    # remove files
    netplay_cleanup \
        "Cleaning up after Pokemon session\n Do not power off!" \
        1 1 1 0 \
        "/tmp/host_ready" \
        "/tmp/stop_now" \
        "/tmp/wpa_supplicant.conf_bk" \
        "/tmp/dismiss_info_panel"
}

# confirm_join_panel: show join confirmation UI with local ROM image
confirm_join_panel() {
    local title="$1"
    local message="$2"

    if [ "$title" = "Join now?" ]; then
        if [ -e "/mnt/SDCARD/Roms/$romdirname/Imgs/$client_rom_filename_NoExt.png" ]; then # local rom image
            pngScale "/mnt/SDCARD/Roms/$romdirname/Imgs/$client_rom_filename_NoExt.png" "/tmp/CurrentNetplay2.png" 100 100
            sync
            imgpop 2 1 "/tmp/CurrentNetplay2.png" 10 10 >/dev/null 2>&1 &
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

# lets_go: main flow for joining GBA session
lets_go() {
    # Allow user to abort via menu while setup runs
    pressMenu2Kill $(basename $0) &

    # Ensure ROM path is provided
    if [ -z "$client_rom" ]; then
        build_infoPanel_and_log "Error" "No ROM path provided."
        exit 1
    fi

	# Join host hotspot
	. "$sysdir/script/network/hotspot_join.sh"
	build_infoPanel_and_log "Connected" "Client IP: ${IP:-unknown}\nHost IP: $hostip"

	# Start FTP for lightweight signaling
	start_ftp

	# Build display names for confirmation prompt
	game_name_client=$(format_game_name "$client_rom_filename_NoExt" "Client (me)")

    # Wait for host ready signal
    wait_for_host

    # Confirm join with local ROM display
    confirm_join_panel "Join now?" "$game_name_client"

    # Stop menu watcher before launch
    pkill -9 pressMenu2Kill

    # Launch RetroArch client session
    start_retroarch

    # Cleanup and restore state
    cleanup
}

lets_go
