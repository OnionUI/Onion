# GLO Pokemon trade
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
save_dir="/mnt/SDCARD/Saves/CurrentProfile/saves/TGB Dual/"
tgb_dual_opts="/mnt/SDCARD/Saves/CurrentProfile/config/TGB Dual/TGB Dual.opt"
tgb_dual_opts_bk="/mnt/SDCARD/Saves/CurrentProfile/config/TGB Dual/TGB Dual.opt.bak"
LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

logfile=pokemon_link
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

rm /tmp/stop_now

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
		
		if is_running wpa_supplicant && ifconfig wlan0 > /dev/null 2>&1; then
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

# Pull the cookie local info that the GLO script has generated to build the host side rom path
get_cookie_info() {
    check_stop
    COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"
    MAX_FILE_SIZE_BYTES=26214400

    if [ -f "$COOKIE_FILE" ]; then
        host_core=$(grep '\[core\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs)
        host_rom=$(grep '\[rom\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs) 
        
        if [ -s "$host_core" ]; then
            core_size=$(stat -c%s "$host_core")
            if [ "$core_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
                echo "[coresize]: $core_size" >> "$COOKIE_FILE"
				log "Writing core size"
            else
                echo "[corechksum]: $(cksum "$host_core" | cut -f 1 -d ' ')" >> "$COOKIE_FILE"
				log "Writing core checksum"
            fi
        fi

        if [ -s "$host_rom" ]; then
            rom_size=$(stat -c%s "$host_rom")
            if [ "$rom_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
                echo "[romsize]: $rom_size" >> "$COOKIE_FILE"
				log "Writing rom size"
            else
                echo "[romchksum]: $(cksum "$host_rom" | cut -f 1 -d ' ')" >> "$COOKIE_FILE"
				log "Writing rom checksum"
            fi
        fi
    else
        log "No cookie found!"
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

        if [ $counter -ge 20 ]; then
            log "No client has connected after 20 seconds."
            build_infoPanel_and_log "Hotspot error" "No client has connected after 20 seconds. Exiting..."
            cleanup
        fi
    done
    
    sleep 1
    log "$client_ip has joined the hotspot"    
}

# Backup the save we're going to use before we do anythign else
backup_save() {
    check_stop
    save_file_filename_full=$(basename "$host_rom")
    save_file_filename="${save_file_filename_full%.*}"
    save_file_matched=$(find "$save_dir" -name "$save_file_filename.srm" -not -name "*.rtc" -not -path "*/.netplay/*")
    log "Backing up save file to: $(basename "${save_file_matched}")_bkup"
    sleep 1
    cp "$save_file_matched" "${save_file_matched}_bkup"
    sync
}

# The client will send us a save file, we'll pull the name from this, find it on the host and call duplicate_rename_rom - send to tmp 
wait_for_save_file() {
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
rename_client_save() {
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

# Download the cookie from the client so we know which second game to preload as GB2
download_cookie() {
    check_stop
	build_infoPanel_and_log "Downloading cookie"  "Retrieving info from client..." 
    log "Trying to pull the clients game info"
    local output_path="/mnt/SDCARD/RetroArch/retroarch.cookie.client"
    curl -o "$output_path" ftp://$client_ip/mnt/SDCARD/RetroArch/retroarch.cookie

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

# Read the cookie and store the paths and checksums into a var - we only need the rom name the client wants us to load as the second rom
read_cookie() {
    check_stop
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
    log "Cookie file read, we need to use: $client_rom"
    
    	#url encode or curl complains
	core_url=$(echo "$core" | sed 's/ /%20/g')
}

# Duplicate the rom to spoof the save loaded in on the host
handle_roms() {
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
    ping -c 5 192.168.100.100 > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        notify_peer "host_ready"
    else
        build_infoPanel_and_log "Error" "No connectivity to 192.168.100.100, \n is the client still connected?"
        notify_stop
    fi
}

# Set the screen and audio settings in TGB dual as these aren't default in Onion
change_tgb_dual_opt() {
    check_stop
    current_value=$(grep 'tgbdual_single_screen_mp' "$tgb_dual_opts" | cut -d '"' -f 2)
    current_audio_value=$(grep 'tgbdual_audio_output' "$tgb_dual_opts" | cut -d '"' -f 2)

    if [ "$1" = "replace" ]; then
        cp "$tgb_dual_opts" "$tgb_dual_opts_bk"
        log "The current TGB Opt value is: $current_value"
        log "Replacing TGB Opt value with 'player 1 only' and audio output to 'Game Boy #1'..."
        
        if [ -z "$current_value" ]; then
            log "The key 'tgbdual_single_screen_mp' was not found in the file, adding"
            echo -e "\ntgbdual_single_screen_mp = \"player 1 only\"" >> "$tgb_dual_opts"
        else
            sed -i 's|tgbdual_single_screen_mp = "'"$current_value"'"|tgbdual_single_screen_mp = "player 1 only"|' "$tgb_dual_opts"
        fi

        if [ -z "$current_audio_value" ]; then
            log "The key 'tgbdual_audio_output' was not found in the file, adding"
            echo 'tgbdual_audio_output = "Game Boy #1"' >> "$tgb_dual_opts"
        else
           sed -i 's|tgbdual_audio_output = "'"$current_audio_value"'"|tgbdual_audio_output = "Game Boy #1"|' "$tgb_dual_opts"
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
    log "Starting RetroArch loaded with $host_rom and $client_rom_clone"
	cd /mnt/SDCARD/RetroArch
    HOME=/mnt/SDCARD/RetroArch ./retroarch -H -v -L .retroarch/cores/tgbdual_libretro.so --subsystem "gb_link_2p" "$host_rom" "$client_rom_clone"
}

# Go into a waiting state for the client to be ready to accept the save
wait_for_save_return() {
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

        if [ $counter -ge 20 ]; then
            build_infoPanel_and_log "Error" "The client didn't ready up, cannot continue..."
            sleep 1
            cleanup
            break
        fi
    done
}


# Push the clients save file back
return_client_save() {
    check_stop
    build_infoPanel_and_log "Syncing" "Returning client save..."
    encoded_path=$(url_encode "${save_new_path/_client/}")
    log "Returning client save: $save_new_path to ftp://$client_ip/${encoded_path}_rcvd"
    curl --connect-timeout 20 -T "$save_new_path" "ftp://$client_ip/${encoded_path}_rcvd"

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

	for proc in hostapd dnsmasq tcpsvd; do
        if is_running $proc; then
            killall -9 $proc
        fi
    done
		
	ifconfig wlan1 down
	
	ifconfig wlan0 up
	
	restore_ftp
    $sysdir/bin/hostapd_cli all_sta flush
    
    # Remove some files we prepared and received
    rm "/tmp/host_ready"
    rm "$save_new_path"
    rm "$client_rom_clone"
    rm "/tmp/ready_to_send"
    rm "/tmp/ready_to_receive"
    rm "$tgb_dual_opts.bak"
    rm "$save_dir/MISSING.srm"
    rm "/tmp/MISSING.srm"
    rm "/tmp/stop_now"
    disable_flag hotspotState
	
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
            build_infoPanel_and_log "Message from client" "The client has had a problem joining the session."
            sleep 2
            cleanup
    fi
}

notify_peer() {
    local notify_file="/tmp/$1"
    touch "$notify_file"
    sync
    curl -T "$notify_file" "ftp://$client_ip/$notify_file" > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        log "Successfully transferred $notify_file to ftp://$client_ip/$notify_file"
    else
        log "Failed to transfer $notify_file to ftp://$client_ip/$notify_file"
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
    game_name="Host: \n$(basename "${host_rom%.*}")"
    game_name_client="\n Client: \n$(basename "${client_rom_clone%.*}")"
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
    curl -o "$file_path" "ftp://$client_ip/$file_url"

    if [ $? -eq 0 ]; then
        log "$file_type download completed"
    else
        log "$file_type download failed"
    fi
}

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
			build_infoPanel_and_log "Rom Missing" "The Rom doesn't exist on the client \n Cannot continue."
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

			else
				build_infoPanel_and_log "$file_type synced!" "$file_type checksums match, no sync required"
			fi
		else
			build_infoPanel_and_log "Syncing" "$file_type doesn't exist locally; syncing with host."
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
	fi
}

unpack_rom() {
  file="$1"
  folder=$(dirname "$file")
  extension="${file##*.}"

  if [ -f "$file" ]; then
    7z x "$file" -o"$folder" > /dev/null 2>&1
  else
    log "File '$file' not found - cannot continue"
  fi
}
    
url_encode() {
    echo "$1" | sed 's/ /%20/g'
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

udhcpc_control() {
	if pgrep udhcpc > /dev/null; then
		killall -9 udhcpc
	fi
	sleep 1
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script > /dev/null 2>&1 &	
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
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
    check_wifi
    start_hotspot
    start_ftp
    get_cookie_info
    wait_for_client
    backup_save
    wait_for_save_file
    rename_client_save
    download_cookie
    read_cookie
    sync_file Rom "$client_rom" "$romcheck" "$rom_url"
    handle_roms
    ready_up
    stripped_game_name
    confirm_join_panel "Join now?" "$game_name \n $game_name_client"
    change_tgb_dual_opt replace
    pkill -9 pressMenu2Kill
    start_retroarch
    change_tgb_dual_opt restore
    wait_for_save_return
    return_client_save
    cleanup
}

lets_go
