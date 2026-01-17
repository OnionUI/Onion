# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
hostip="192.168.100.100" # This should be the default unless the user has changed it..
rm /tmp/stop_now
client_rom="$1"
romdirname=$(echo "$client_rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
client_rom_filename=$(basename "$client_rom")
client_rom_filename_NoExt="${client_rom_filename%.*}"
netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/gpsp_libretro.so"

logfile=pokemon_link
# Source scripts
. $sysdir/script/log.sh
. $sysdir/script/netplay/netplay_common.sh

log "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Easy Netplay Pokemon Client GBA -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

##########
##Setup.##
##########

# Wait for the host to tell us it's ready, this happens just before it starts its RA session and we look in /tmp for a file indicator (file removed in host script cleanup)

# Start retroarch with -C in client mode if everything's gone to plan
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

    wait_for_host # we wait a second flag triggered from the host once RA is running
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

cleanup() {
    build_infoPanel_and_log "Cleanup" "Cleaning up after Pokemon session\n Do not power off!"

    pkill -9 pressMenu2Kill

    if is_running infoPanel; then
        killall -9 infoPanel
    fi

    # restore_wifi_state
    . "$sysdir/script/network/hotspot_cleanup.sh"
    restore_ftp

    # Remove some files we prepared and received
    log "Removing stale files"
    rm "/tmp/host_ready"
    rm "/tmp/stop_now"
    rm "/tmp/wpa_supplicant.conf_bk"
    rm "/tmp/dismiss_info_panel"
    sync

    log "Cleanup done"

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
        build_infoPanel_and_log "Message from client" "The host has had a problem setting up the session"
        sleep 2
        cleanup
    fi
}

# Notify other MMP
notify_peer() {
    local notify_file="/tmp/$1"
    touch "$notify_file"
    sync
    curl -T "$notify_file" "ftp://${hostip}/${notify_file}" >/dev/null 2>&1 # the first / after the IP must be not encoded

    if [ $? -eq 0 ]; then
        log "Successfully transferred $notify_file to ftp://${hostip}/${notify_file}"
    else
        log "Failed to transfer $notify_file to ftp://${hostip}/${notify_file}"
    fi
}

# We'll need FTP to transfer files
start_ftp() {
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

# This will restore the users original ftp state


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
	sleep 0.3
	sync
}

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

    # TO DO : allow to confirm only once the host has started
    infoPanel -t "$title" -m "$message"
    retcode=$?

    if [ $retcode -ne 0 ]; then
        build_infoPanel_and_log "Cancelled" "User cancelled, exiting."
        cleanup
        exit 1
    fi
}

stripped_game_names() {
    client_rom_trimmed="$(echo "$client_rom_filename_NoExt" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
    game_name_client="Client (me): \n$client_rom_trimmed"
}


#########
##Main.##
#########

lets_go() {
    pressMenu2Kill $(basename $0) &
    if [ -z "$client_rom" ]; then
        build_infoPanel_and_log "Error" "No ROM path provided."
        exit 1
    fi
    . "$sysdir/script/network/hotspot_join.sh"
    start_ftp
    stripped_game_names
    wait_for_host
    confirm_join_panel "Join now?" "Start the game on the host first! \n $game_name_client"
    pkill -9 pressMenu2Kill
    start_retroarch
    cleanup
}

lets_go
