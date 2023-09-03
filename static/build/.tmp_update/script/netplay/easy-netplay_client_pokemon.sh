# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
save_dir="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/"
tgb_dual_opts="/mnt/SDCARD/Saves/CurrentProfile/config/TGB Dual/TGB Dual.opt"
tgb_dual_opts_bk="/mnt/SDCARD/Saves/CurrentProfile/config/TGB Dual/TGB Dual.opt.bak"
LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
WPACLI=/customer/app/wpa_cli
hostip="192.168.100.100" # This should be the default unless the user has changed it..
rm /tmp/stop_now
local_rom="$cookie_rom_path"


## Source global utils
logfile=pokemon_link
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

##########
##Setup.##
##########

# Read the cookie and store the paths and checksums into a var.
read_cookie() {
    sync
    while IFS= read -r line; do
        case $line in
        "[core]: "*)
            core="${line##"[core]: "}"
            ;;
        "[rom]: "*)
            rom="${line##"[rom]: "}"
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

    romdirname=$(echo "$rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
    romName=$(basename "$rom")
    romNameNoExtension=${romName%.*}
    Img_path="/mnt/SDCARD/Roms/$romdirname/Imgs/$romNameNoExtension.png"
    log "Cookie file read"
}

# Push our saves over to the host - The save will be found based on the rom we've started GLO on and it will look in the $save_dir path for it (see line13) - Backs up first
send_saves() {
    check_stop
    missing=""
    build_infoPanel_and_log "Syncing saves" "Syncing our save files with the host"

    save_file_filename_full=$(basename "$local_rom")
    save_file_filename="${save_file_filename_full%.*}"
    save_file_matched=$(find "$save_dir" -name "$save_file_filename.srm" -not -name "*.rtc" -not -path "*/.netplay/*")

    if [ ! -f "$save_file_matched" ]; then
        if [ -f "/mnt/SDCARD/Saves/CurrentProfile/saves/Gambatte/$save_file_filename.srm" ]; then
            cp "/mnt/SDCARD/Saves/CurrentProfile/saves/Gambatte/$save_file_filename.srm"  "$save_dir$save_file_filename.srm"
            save_file_matched=$(find "$save_dir" -name "$save_file_filename.srm" -not -name "*.rtc" -not -path "*/.netplay/*")
        else
            touch "$save_dir/MISSING.srm"
            save_file_matched="$save_dir/MISSING.srm"
            missing=1
        fi
    fi

    save_file_stripped="${save_file_matched##*/}"
    encoded_save_file=$(url_encode "$save_file_stripped")

    curl -T "$save_file_matched" "ftp://$hostip/tmp/$encoded_save_file"

    if [ "$missing" = "1" ]; then
        build_infoPanel_and_log "Save not found" "You don't have a save for this game, \n or we failed to find it. Cannot continue \n Notified host."
        notify_stop
    fi
}

# Backup local save file
backup_save() {
    check_stop
    log "Backing up save file to: ${save_file_matched}_$(date +%Y%m%d_%H%M%S)"
    cp "$save_file_matched" "${save_file_matched}_$(date +%Y%m%d_%H%M%S)"
    sync
}

# Wait for the host to tell us it's ready, this happens just before it starts its RA session and we look in /tmp for a file indicator (file removed in host script cleanup)
wait_for_host() {

    local counter=0

    build_infoPanel_and_log "Ready" "Waiting for host to ready up"
    while true; do
        sync
        check_stop
        for file in /tmp/host_ready; do
            if [ -f "$file" ]; then
                build_infoPanel_and_log "Message from host" "Setup complete"
                break 2
            fi
        done

        sleep 1
        counter=$((counter + 1))

        if [ $counter -ge 25 ]; then
            build_infoPanel_and_log "Error" "The host didn't ready up, cannot continue..."
            notify_stop
        fi
    done
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {
    build_infoPanel_and_log "Starting RA" "Starting RetroArch"

    echo "*****************************************"
    echo "romfullpath: $rom"
    echo "platform: ${platform}"
    echo "netplaycore: $netplaycore"
    echo "core: $core"
    echo "core_config_folder: $core_config_folder"
    echo "cpuspeed: $cpuspeed"
    echo "hostip: $hostip"
    echo "*****************************************"

    if [ -n "$cpuspeed" ]; then
        log "We set core CPU speed for Netplay: $cpuspeed"
        PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
        echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    fi

    cd /mnt/SDCARD/RetroArch
    log "Starting RetroArch loaded with $rom and $local_rom"
    HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -C $hostip -L "$core" --subsystem "gb_link_2p" "$rom" "$local_rom"

    if [ -n "$PreviousCPUspeed" ]; then
        log "We restore previous core CPU speed: $PreviousCPUspeed"
        echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    else
        rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
    fi
}

# Go into a waiting state for the host to return the save
wait_for_save_return() {
    build_infoPanel_and_log "Syncing" "Waiting for host to be ready for save sync"
    notify_peer "ready_to_receive"

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

    log "cp -f \"${save_file_matched}_rcvd\" \"$save_file_matched\""
    cp -f "${save_file_matched}_rcvd" "$save_file_matched"
    cp_exit_status=$?

    if [ $cp_exit_status -eq 0 ]; then
        build_infoPanel_and_log "Syncing save" "Save merged successfully"
        sleep 1
    else
        build_infoPanel_and_log "Syncing save" "Failed to merge save \n Error code: $cp_exit_status"
        sleep 1
    fi
}

# If you don't call this you don't retransfer the saves - Users cannot under any circumstances miss this function.
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
    rm "/tmp/ready_to_send"
    rm "/tmp/ready_to_receive"
    rm "${save_file_matched}_rcvd"
    rm "$tgb_dual_opts.bak"
    rm "$save_dir/MISSING.srm"
    rm "/tmp/MISSING.srm"
    rm "/tmp/stop_now"
    rm "/tmp/wpa_supplicant.conf_bk"
    # rm "/mnt/SDCARD/RetroArch/retroarch.cookie.client"
    # rm "/mnt/SDCARD/RetroArch/retroarch.cookie"

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
    curl -T "$notify_file" "ftp://$hostip/$notify_file" >/dev/null 2>&1

    if [ $? -eq 0 ]; then
        log "Successfully transferred $notify_file to ftp://$hostip/$notify_file"
    else
        log "Failed to transfer $notify_file to ftp://$hostip/$notify_file"
    fi
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
    file_url="ftp://${hostip}/$(url_encode "${file_path#*/}")"

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
            [ $file_mandatory -eq 1 ] && cleanup

        fi
        if [ "$same_chksum" -ne 1 ]; then
            msg="$msg\nFiles doesn't have the same checksum."
            sync_success=0
            [ $file_mandatory -eq 1 ] && cleanup

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
        [ $file_mandatory -eq 1 ] && cleanup

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
            if [ $file_mandatory -eq 1 ]; then
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

# Create a cookie with all the required info for the client. (client will use this cookie)
create_cookie_info() {
    echo -e "\n\n\n\n\n\n\n"
    echo "we create =================================o=o=o=o=o=o==o"
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
        log "Writing rom size"
        rom_size=$(stat -c%s "$cookie_rom_path")
        if [ "$rom_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
            echo "[romchksum]: 0" >>"$COOKIE_FILE"
        else
            echo "[romchksum]: $(xcrc "$cookie_rom_path")" >>"$COOKIE_FILE"
        fi
    fi

    if [ -s "$cpuspeed" ]; then
        echo "[cpuspeed]: $cpuspeed" >>"$COOKIE_FILE"
        log "Writing cpuspeed: $cpuspeed"
    fi

}

# This will restore the users original ftp state
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

build_infoPanel_and_log() {
    local title="$1"
    local message="$2"

    log "Info Panel: \n\tStage: $title\n\tMessage: $message"

    infoPanel --title "$title" --message "$message" --persistent &
    touch /tmp/dismiss_info_panel
    sync
    sleep 0.3
}

confirm_join_panel() {
    local title="$1"
    local message="$2"

	if [ -e "$Img_path" ]; then   # remote rom image
		pngScale "$Img_path" "/tmp/CurrentNetplay.png" 100 100
        sync
		imgpop 3 2 "/tmp/CurrentNetplay.png" 530 300 >/dev/null 2>&1 &
	fi

	if [ -e "/mnt/SDCARD/Roms/$romdirname/Imgs/$save_file_filename.png" ]; then  # local rom image
		pngScale "/mnt/SDCARD/Roms/$romdirname/Imgs/$save_file_filename.png" "/tmp/CurrentNetplay2.png" 100 100
        sync
		imgpop 2 1 "/tmp/CurrentNetplay2.png" 10 10 >/dev/null 2>&1 &
	fi

    # TO DO : allow to confirm only once the host has started
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
    game_name="Host: \n$(basename "${rom%.*}")"
    game_name="$(echo "$game_name" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"

    local_rom_trimmed="$(echo "$save_file_filename" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
    game_name_client="\n Client (me): \n$local_rom_trimmed"
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

is_running() {
    process_name="$1"
    pgrep "$process_name" >/dev/null
}

#########
##Main.##
#########

lets_go() {
    pressMenu2Kill $(basename $0) &
    create_cookie_info
    . "$sysdir/script/network/hotspot_join.sh"
    start_ftp
    sync_file "Cookie" "/mnt/SDCARD/RetroArch/retroarch.cookie" 0 0 -f -m
    read_cookie
    send_saves
    backup_save
    # sync_file Rom "$local_rom" # Doesn't sync anything, just uses the sync function to confirm it exists locally
    sync_file "Core" "$core" 1 "$corechecksum" -b -m
    sync_file "Rom" "$rom" 1 "$romchecksum" -b -m
    sync_file "Img" "$Img_path" 0 0 -o
    stripped_game_name
    wait_for_host
    confirm_join_panel "Join now?" "Start the game on the host first! \n $game_name \n $game_name_client"
    pkill -9 pressMenu2Kill
    start_retroarch
    wait_for_save_return
    # TO DO : export save to Gambatte (with question)
    cleanup
}

lets_go
