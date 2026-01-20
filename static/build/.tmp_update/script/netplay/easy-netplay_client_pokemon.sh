# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
. /mnt/SDCARD/.tmp_update/script/netplay/easy-netplay_env.sh
WPACLI=/customer/app/wpa_cli

# Runtime vars
rm /tmp/stop_now
client_rom="$cookie_rom_path"
client_rom_filename=$(basename "$client_rom")
client_rom_filename_NoExt="${client_rom_filename%.*}"

SaveFromGambatte=0

logfile=pokemon_link

# Source scripts
. $sysdir/script/log.sh
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running,
# restore_ftp, udhcpc_control, url_encode, strip_game_name, read_cookie, check_wifi, start_ftp, sync_file
. $sysdir/script/netplay/easy-netplay_common.sh
# easy-netplay_signalling.sh: check_stop, notify_peer, notify_stop, wait_for_host
. $sysdir/script/netplay/easy-netplay_signalling.sh

program=$(basename "$0" .sh)

export CurDate=$(date +%Y%m%d_%H%M%S)
log "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* Easy Netplay Pokemon Client -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\ndate : $CurDate"

##########
##Setup.##
##########

# Push our save over to the host - The save will be found based on the rom we've started GLO on and it will look in the $save_dir path for it - Backs up first
backup_and_send_save() {
    # check_stop: host reported a setup issue
    check_stop "The host has had a problem setting up the session"
    missing=""
    build_infoPanel_and_log "Syncing saves" "Syncing our save files with the host"

    mkdir -p "/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual"
    save_gambatte="/mnt/SDCARD/Saves/CurrentProfile/saves/Gambatte/$client_rom_filename_NoExt.srm"
    save_tgbdual="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/$client_rom_filename_NoExt.srm"

    if [ -f "$save_gambatte" ]; then
        SaveFromGambatte=1
        log "Gambatte save file detected."
        log "Backing up Gambatte save file to: $client_rom_filename_NoExt.srm_$Curdate"
        cp -f "$save_gambatte" "${save_gambatte}_$CurDate"
        if [ -f "$save_tgbdual" ]; then
            confirm_join_panel "Continue ?" "There is a local save for\n$client_rom_filename_NoExt\nfor TGB Dual and for Gambatte.\n Gambatte save will be used by default."
            log "Existing TGB Dual save file detected. Continue with Gambatte save."
            # cp -f "$save_tgbdual" "${save_tgbdual}_$CurDate"
        fi
        # copy save from Gambatte to TGB Dual
        cp -f "$save_gambatte" "$save_tgbdual"
        save_file_matched="$save_gambatte"
    elif [ -f "$save_tgbdual" ]; then
        log "No Gambatte save file detected, using TGB Dual save file instead."
        log "Backing up current TGB Dual save file to: $client_rom_filename_NoExt.srm_$Curdate"
        cp -f "$save_tgbdual" "${save_tgbdual}_$CurDate"
        save_file_matched="$save_tgbdual"
    else
        touch "/tmp/MISSING.srm"
        save_file_matched="/tmp/MISSING.srm"
    fi

    save_file_stripped="${save_file_matched##*/}"
    encoded_save_file=$(url_encode "$save_file_stripped")
    log "encoded_save_file: $encoded_save_file"

    curl -T "$save_file_matched" "ftp://$hostip/tmp/$encoded_save_file"

    if [ "$missing" = "1" ]; then
        build_infoPanel_and_log "Save not found" "You don't have a save for this game, \n or we failed to find it. Cannot continue \n Notified host."
        notify_stop
    fi

}

# Start retroarch with -C in client mode if everything's gone to plan
start_retroarch() {
    build_infoPanel_and_log "Starting RA" "Starting RetroArch"

    log "\n############################ RETROARCH DEBUGGING ############################"
    log "host_rom: $rom"
    log "client_rom_clone (here): ${client_rom}"
    log "core: $core"
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
    log "Starting RetroArch loaded with $rom and $client_rom"
    HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -C $hostip -L "$core" --subsystem "gb_link_2p" "$rom" "$client_rom"

    if [ -n "$PreviousCPUspeed" ]; then
        log "We restore previous core CPU speed: $PreviousCPUspeed"
        echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    else
        rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    fi
}

# Go into a waiting state for the host to return the save (If you don't call this you don't retransfer the saves - Users cannot under any circumstances miss this function)
wait_for_save_return() {
    build_infoPanel_and_log "Syncing" "Waiting for host to be ready for save sync"
    # notify_peer: signal ready to receive save
    notify_peer "$hostip" "ready_to_receive"

    sync

    while true; do
        for file in /tmp/ready_to_send; do
            if [ -f "$file" ]; then
                build_infoPanel_and_log "Message from Host" "Host is ready to send save"
                log "Received notification \n Host is ready to send the save"
                break 2
            fi
        done

        sleep 1
        counter=$((counter + 1))

        if [ $counter -ge 30 ]; then
            build_infoPanel_and_log "Error" "The Host didn't ready up, cannot continue..."
            log "We ran out of time waiting for the host to ready up, possibly due to host->client connecitivity"
            sleep 1
            cleanup
        fi
    done

    sleep 3
    sync

    received_save="/tmp/$(basename "$save_file_matched")_rcvd"

    log "cp -f \"$received_save\" \"$save_file_matched\""
    cp -f "${received_save}" "$save_file_matched"
    cp_exit_status=$?

    if [ $cp_exit_status -eq 0 ]; then
        if [ $SaveFromGambatte -eq 1 ]; then
            mv -f "/mnt/SDCARD/Saves/CurrentProfile/states/Gambatte/$client_rom_filename_NoExt.state.auto" "/mnt/SDCARD/Saves/CurrentProfile/states/Gambatte/$client_rom_filename_NoExt.state.auto_$CurDate"
        else
            mv -f "/mnt/SDCARD/Saves/CurrentProfile/states/TGB Dual/$client_rom_filename_NoExt.state.auto" "/mnt/SDCARD/Saves/CurrentProfile/states/TGB Dual/$client_rom_filename_NoExt.state.auto_$CurDate"
        fi
        build_infoPanel_and_log "Syncing save" "Save merged successfully"
        sleep 1
    else
        build_infoPanel_and_log "Syncing save" "Failed to merge save \n Error code: $cp_exit_status"
        sleep 1
    fi
}

# cleanup: restore wifi/ftp and remove session temp files
cleanup() {
    log "Removing stale files"
    # message: cleanup infoPanel text
    # args: hotspot_cleanup restore_ftp kill_infopanel disable_hotspot_flag
    # remove files
    netplay_cleanup \
        "Cleaning up after Pokemon session\n Do not power off!" \
        1 1 1 0 \
        "/tmp/host_ready" \
        "/tmp/ready_to_send" \
        "/tmp/ready_to_receive" \
        "${save_file_matched}_rcvd" \
        "/tmp/MISSING.srm" \
        "/tmp/stop_now" \
        "/tmp/wpa_supplicant.conf_bk" \
        "/mnt/SDCARD/RetroArch/retroarch.cookie.client" \
        "/mnt/SDCARD/RetroArch/retroarch.cookie" \
        "/tmp/dismiss_info_panel"
}

###########
#Utilities#
###########

# confirm_join_panel: show join confirmation UI with host/client images
confirm_join_panel() {
    local title="$1"
    local message="$2"

    if [ "$title" = "Join now?" ]; then
        if [ -e "$Img_path" ]; then # remote rom image
            pngScale "$Img_path" "/tmp/CurrentNetplay.png" 100 100
            sync
            imgpop 3 2 "/tmp/CurrentNetplay.png" 530 300 >/dev/null 2>&1 &
        fi

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

# stripped_game_names: format host/client display names
stripped_game_names() {
    host_name_trimmed="$(strip_game_name "$(basename "${rom%.*}")")"
    game_name="Host: \n$host_name_trimmed"

    client_rom_trimmed="$(strip_game_name "$client_rom_filename_NoExt")"
    game_name_client="\n Client (me): \n$client_rom_trimmed"
}

#########
##Main.##
#########

lets_go() {
    # Allow user to abort via menu while setup runs
    pressMenu2Kill $(basename $0) &

    # Write cookie for host (core/rom metadata)
    create_cookie_info

    # Join host hotspot
    . "$sysdir/script/network/hotspot_join.sh"

	# start_ftp: start FTP without preflight
	start_ftp

    # Send cookie to host
    sync_file "Cookie" "/mnt/SDCARD/RetroArch/retroarch.cookie" 0 0 -f -m

    # Read host cookie and parse paths/checksums (verbose logging)
    read_cookie 1

    # Send local save to host
    backup_and_send_save

    # Sync required core, rom, and image
    sync_file "Core" "$core" 1 "$corechecksum" -b -m
    sync_file "Rom" "$rom" 1 "$romchecksum" -b -m
    sync_file "Img" "$Img_path" 0 0 -o

    # Build display names for confirmation prompt
    stripped_game_names

    # Wait for host ready signal
    wait_for_host

    # Confirm join with host/client info
    confirm_join_panel "Join now?" "$game_name \n $game_name_client"

    # Stop menu watcher before launch
    pkill -9 pressMenu2Kill

    # Launch RetroArch client session
    start_retroarch

    # Wait for save return from host
    wait_for_save_return

    # Cleanup and restore state
    cleanup
}

lets_go
