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

logfile=pokemon_link
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..

check_wifi() {
    ifconfig wlan1 down
    $WPACLI save_config
    save_wifi_state
    sync
	if ifconfig wlan0 &>/dev/null; then
		build_infoPanel_and_log "WIFI" "Wifi up"
	else
		build_infoPanel_and_log "WIFI" "Wifi disabled, starting..." 
		
		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		sleep 1
		$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
		
		if is_running wpa_supplicant && ifconfig wlan0 > /dev/null 2>&1; then
			build_infoPanel_and_log "WIFI" "Wifi started."
		else
			build_infoPanel_and_log "WIFI" "Unable to start WiFi\n unable to continue."
			notify_stop
		fi
		
		sleep 2 
	fi
}


# Create a new network id, set it up and enable it, start udhcpc against it.
connect_to_host() {
	build_infoPanel_and_log "Connecting..." "Trying to join the hotspot..."

	export new_id=$($WPACLI -i wlan0 add_network)
    
	if [ -z "$new_id" ]; then
		build_infoPanel_and_log "Failed"  "Failed to create network\n unable to continue." 
		return 1
	fi
	
	log "Added new network with id $new_id"

	net_setup=$(
		$WPACLI -i wlan0 <<- EOF
			set_network $new_id ssid "MiyooMini+APOnionOS"
			set_network $new_id psk "onionos+"
			disable_network all
			select_network $new_id
			enable_network $new_id
			save_config
			reconfigure
			quit
		EOF
	)
	
	if [ $? -ne 0 ]; then
		build_infoPanel_and_log "Failed"  "Failed to configure the network\n unable to continue." 
		notify_stop
	fi
    
    ip addr flush dev wlan0
    ip addr add 192.168.100.101/24 dev wlan0
    ip route add default via 192.168.100.100
    
    killall -9 udhcpc
    sleep 1
	
	log "Added new network and connected"
}


# We'd better wait for an ip address to be assigned before going any further.
wait_for_connectivity() {
    build_infoPanel_and_log "Connecting..."  "Trying to reach $hostip..."
    counter=0

    while ! ping -c 2 -W 2 $hostip > /dev/null 2>&1; do
        counter=$((counter+1))

        if [ $counter -ge 20 ]; then
            build_infoPanel_and_log "Failed to connect!"  "Could not reach $hostip"
            cleanup
        fi
    done

    build_infoPanel_and_log "Joined hotspot!"  "Successfully reached the Hotspot! \n IP: $hostip" 
    sleep 1
}


# Download the cookie from the host, check whether it downloaded and make sure it still exists on the client before we move on
download_cookie() {
    check_stop
    local output_path="/mnt/SDCARD/RetroArch/retroarch.cookie.client"
    curl -o "$output_path" ftp://$hostip/mnt/SDCARD/RetroArch/retroarch.cookie

    if [ $? -ne 0 ]; then
		build_infoPanel_and_log "Failed"  "Can't download the cookie, can't continue" 
		notify_stop
    fi

    if [ ! -f $output_path ]; then
		build_infoPanel_and_log "No cookie found"  "Cookie has been eaten, can't continue" 
		notify_stop
    fi
	
	build_infoPanel_and_log "Success!"  "Got the cookie" 
}

# Read the cookie and store the paths and checksums into a var.
read_cookie() {
    check_stop
	while IFS= read -r line; do
		case $line in
			"[core]: "*)
				core="${line##"[core]: "}"
				;;
			"[rom]: "*)
				rom="${line##"[rom]: "}"
				;;
			"[coresize]: "*)
				corecheck="${line##"[coresize]: "}"
				;;
			"[corechksum]: "*)
				corecheck="${line##"[corechksum]: "}"
				;;
			"[romsize]: "*)
				romcheck="${line##"[romsize]: "}"
				;;
			"[romchksum]: "*)
				romcheck="${line##"[romchksum]: "}"
				;;
		esac
	done <"/mnt/SDCARD/RetroArch/retroarch.cookie.client"
    log "$core $rom"
	
	#url encode or curl complains
	export core_url=$(echo "$core" | sed 's/ /%20/g')

	log "Cookie file read"
}

# Pull the cookie local info that the GLO script has generated to build the host side rom path
get_cookie_info() {
    check_stop
    COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"

    if [ -f "$COOKIE_FILE" ]; then
        local_rom=$(grep '\[rom\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs) 
        local_rom_ext="${local_rom##*.}"
    else
        log "No cookie found!"
    fi
    
    # Check the rom actually exists before we go any further
    sync
    if [ ! -f "$local_rom" ]; then
        build_infoPanel_and_log "No rom file found" "Youre trying to start a session for a game \n that doesn't exist! How? \n Can't continue.." 
        notify_stop
    fi
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
        touch "$save_dir/MISSING.srm"
        save_file_matched="$save_dir/MISSING.srm"
        missing=1
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
    log "Backing up save file to: ${save_file_matched}_bkup"
    cp "$save_file_matched" "${save_file_matched}_bkup"
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


# Set the screen and audio settings in TGB dual as these aren't default in Onion
change_tgb_dual_opt() {
    current_value=$(grep 'tgbdual_single_screen_mp' "$tgb_dual_opts" | cut -d '"' -f 2)
    current_audio_value=$(grep 'tgbdual_audio_output' "$tgb_dual_opts" | cut -d '"' -f 2)

    if [ "$1" = "replace" ]; then
        cp "$tgb_dual_opts" "$tgb_dual_opts_bk"
        log "The current TGB Opt value is: $current_value"
        log "Replacing TGB Opt value with 'player 2 only' and audio output to 'Game Boy #2'..."
        
        if [ -z "$current_value" ]; then
            log "The key 'tgbdual_single_screen_mp' was not found in the file, adding"
            echo -e "\ntgbdual_single_screen_mp = \"player 2 only\"" >> "$tgb_dual_opts"
        else
            sed -i 's|tgbdual_single_screen_mp = "'"$current_value"'"|tgbdual_single_screen_mp = "player 2 only"|' "$tgb_dual_opts"
        fi

        if [ -z "$current_audio_value" ]; then
            log "The key 'tgbdual_audio_output' was not found in the file, adding"
            echo -e 'tgbdual_audio_output = "Game Boy #2"' >> "$tgb_dual_opts"
        else
           sed -i 's|tgbdual_audio_output = "'"$current_audio_value"'"|tgbdual_audio_output = "Game Boy #2"|' "$tgb_dual_opts"
        fi 
    elif [ "$1" = "restore" ]; then
        log "Restoring TGB opt original values..."
        mv "$tgb_dual_opts_bk" "$tgb_dual_opts"
    else
        log "Invalid argument for TGB Opt. Please use 'replace' or 'restore'."
    fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {
    check_stop
	build_infoPanel_and_log "RetroArch" "Starting RetroArch..."
    log "Starting RetroArch loaded with $rom and $local_rom"
	cd /mnt/SDCARD/RetroArch
    HOME=/mnt/SDCARD/RetroArch ./retroarch -C $hostip -v -L .retroarch/cores/tgbdual_libretro.so --subsystem "gb_link_2p" "$rom" "$local_rom"
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

        if [ $counter -ge 20 ]; then
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
    
    udhcpc_control
    sleep 1
    
    restore_wifi_state	
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
		
	log "Cleanup done"
	exit
}

###########
#Utilities#
###########

# Use the safe word
notify_stop(){
    notify_peer "stop_now"
    sleep 2 
    cleanup
}

# Check stop, if the client tells us to stop we will.
check_stop(){
    sync
    if [ -e "/tmp/stop_now" ]; then
            build_infoPanel_and_log "Message from client" "The host has had a problem setting up the session"
            sleep 2
            cleanup
    fi
}

# URL encode helper
url_encode() {
    echo "$1" | sed 's/ /%20/g'
}

# Notify other MMP
notify_peer() {
    local notify_file="/tmp/$1"
    touch "$notify_file"
    sync
    curl -T "$notify_file" "ftp://$hostip/$notify_file" > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        log "Successfully transferred $notify_file to ftp://$hostip/$notify_file"
    else
        log "Failed to transfer $notify_file to ftp://$hostip/$notify_file"
    fi
}

# Function to sync files
sync_file() {
	file_type="$1"
	file_path="$2"
	file_checksum="$3"
	file_url="$4"
	MAX_FILE_SIZE_BYTES=26214400

	if [ -z "$file_path" ]; then
        build_infoPanel_and_log "Something went wrong" "We didn't receive a file path for the rom \n Cannot continue."
        sleep 2
        cleanup
	fi

	if [ "$file_type" == "Rom" ]; then
		if [ -e "$file_path" ]; then
			log "$file_path exists."

			local file_size=$(stat -c%s "$file_path")
			local file_chksum_actual

			if [ "$file_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				file_chksum_actual=$file_size
			else
				file_chksum_actual=$(cksum "$file_path" | awk '{ print $1 }')
			fi

			if [ "$file_checksum" -ne "$file_chksum_actual" ]; then
				build_infoPanel_and_log "Checksum Mismatch" "The Rom exists but the checksum doesn't match \n Cannot continue."
                notify_peer "stop_now"
                sleep 2
                cleanup
			else
				build_infoPanel_and_log "Rom Check Complete!" "Rom exists and checksums match!"
			fi
		else
			build_infoPanel_and_log "Rom Missing" "A rom requested doesn't exist on the local client \n Cannot continue."
            notify_peer "stop_now"
            sleep 2
            cleanup
		fi
	else
		if [ -e "$file_path" ]; then
			log "$file_path exists."

			local file_size=$(stat -c%s "$file_path")
			local file_chksum_actual

			if [ "$file_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				file_chksum_actual=$file_size
			else
				file_chksum_actual=$(cksum "$file_path" | awk '{ print $1 }')
			fi

			if [ "$file_checksum" -ne "$file_chksum_actual" ]; then
				build_infoPanel_and_log "Syncing" "$file_type checksums don't match, syncing"
				sleep 0.5
				do_sync_file "$file_type" "$file_path" "$file_url"

				if [ ! -e "$file_path" ]; then
					build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
                    notify_peer "stop_now"
                    sleep 2
					cleanup
				else
					build_infoPanel_and_log "Syncing" "$file_type synced."
				fi
			fi
		else
			build_infoPanel_and_log "Syncing" "$file_type doesn't exist locally; syncing with host."
			do_sync_file "$file_type" "$file_path" "$file_url"

			if [ ! -e "$file_path" ]; then
				build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
                notify_peer "stop_now"
                sleep 2
				cleanup
			fi
		fi
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
restore_ftp(){
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
	sleep 0.5
}

confirm_join_panel() {
    local title="$1"
    local message="$2"
    
    sync
            
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
    game_name_client="\n Client: \n$(basename "${local_rom%.*}")"
}

# If we're currently connected to wifi, save the network ID so we can reconnect after we're done with retroarch - save the IP address and subnet so we can restore these.
save_wifi_state() {
    IFACE=wlan0
    cp /appconfigs/wpa_supplicant.conf /tmp/wpa_supplicant.conf_bk
	old_ipv4=$(ip -4 addr show $IFACE | grep -o 'inet [^ ]*' | cut -d ' ' -f 2)
	ip addr del $old_ipv4/$old_mask dev $IFACE
}

restore_wifi_state() {
    cp /tmp/wpa_supplicant.conf_bk /appconfigs/wpa_supplicant.conf
    sync
	if [ -z "$old_ipv4" ]; then
		log "Old IP address not found."
	fi

	ip_output=$(ip link set wlan0 down 2>&1)
	if [ $? -ne 0 ]; then
		log "Failed to bring down the interface."
		log "Output from 'ip link set down' command: $ip_output"
	fi

	ip -4 addr show wlan0 | awk '/inet/ {print $2}' | while IFS= read -r line; do
		ip addr del "$line" dev wlan0
	done

	ip_output=$(ip addr add $old_ipv4 dev wlan0 2>&1)
	if [ $? -ne 0 ]; then
		log "Failed to assign the old IP address."
		log "Output from 'ip addr add' command: $ip_output"
	fi

	ip_output=$(ip link set wlan0 up 2>&1)
	if [ $? -ne 0 ]; then
		log "Failed to bring up the interface."
		log "Output from 'ip link set up' command: $ip_output"
	fi
    
    $WPACLI reconfigure
}

do_sync_file() {
    file_type="$1"
    file_path="$2"
    file_url="$3"

    dir_path=$(dirname "$file_path")

    if [ ! -d "$dir_path" ]; then
        mkdir -p "$dir_path"
    fi

    if [ -e "$file_path" ]; then
        mv "$file_path" "${file_path}_old"
        log "Existing $file_type file moved to ${file_path}_old"
    fi

    log "Starting to download $file_type from $file_url"
    curl -o "$file_path" "ftp://$hostip/$file_url" > /dev/null 2>&1

    if [ $? -eq 0 ]; then
        log "$file_type download completed"
    else
        log "$file_type download failed"
    fi
}

udhcpc_control() {
	if pgrep udhcpc > /dev/null; then
		killall -9 udhcpc
        log "Old DHCP proc killed."
	fi
	sleep 1
	log "Trying to start DHCP"
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script > /dev/null 2>&1 &
	if is_running udhcpc; then
		log "DHCP started"
	else
		build_infoPanel_and_log "DHCP" "Unable to start DHCP client\n unable to continue."
	fi
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}


#########
##Main.##
#########

lets_go() {
    pressMenu2Kill $(basename $0) &
    check_wifi
    connect_to_host
    wait_for_connectivity
    start_ftp
    get_cookie_info
    download_cookie
    read_cookie
    send_saves
    backup_save
    sync_file Rom "$local_rom" # Doesn't sync anything, just uses the sync function to confirm it exists locally
    sync_file Rom "$rom" "$romcheck" "$rom_url"
    sync_file Core "$core" "$corecheck" "$core_url"
    stripped_game_name
    wait_for_host
    confirm_join_panel "Join now?" "Start the game on the host first! \n $game_name \n $game_name_client"
    change_tgb_dual_opt replace
    pkill -9 pressMenu2Kill
    start_retroarch
    change_tgb_dual_opt restore
    wait_for_save_return
    cleanup
}


lets_go
