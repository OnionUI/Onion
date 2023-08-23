# GLO CLIENT
# Script to:
# Enable Wifi
# Pull the cookie file from the host
# Pull the core from the host based on what the cookie file gives us
# Start retroarch with the above passed, and -C provided to connect.

# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export WPACLI=/customer/app/wpa_cli
export hostip="192.168.100.100" # This should be the default unless the user has changed it..

logfile=easy_netplay
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
	if ifconfig wlan0 &> /dev/null; then
        build_infoPanel_and_log "WIFI" "Wifi up"
	else
		log "Wi-Fi disabled, trying to enable before connecting.."
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
			sleep 1
			cleanup
		fi

		sleep 2
	fi
}

# Create a new network id, set it up and enable it, start udhcpc against it.
connect_to_host() {
	build_infoPanel_and_log "Connecting..." "Trying to join the hotspot..."

	new_id=$($WPACLI -i wlan0 add_network)
	if [ -z "$new_id" ]; then
		build_infoPanel_and_log "Failed" "Failed to create network\n unable to continue."
		cleanup
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
		build_infoPanel_and_log "Failed" "Failed to configure the network\n unable to continue."
		sleep 1
		cleanup
	fi

	udhcpc_control

	sleep 0.5
}

# We'd better wait for an ip address to be assigned before going any further.
wait_for_ip() {
    ip addr flush dev wlan0
    IP=""
    build_infoPanel_and_log "Connecting..." "Waiting for an IP..."
    local counter=0

    while [ -z "$IP" ]; do
        IP=$(ip route get 1 2> /dev/null | awk '{print $NF;exit}')
        sleep 1
        counter=$((counter + 1))

        if [ $counter -eq 10 ]; then
            build_infoPanel_and_log "Fallback" "Using static IP..."
            killall -9 udhcpc 
            ip addr flush dev wlan0
            RAND_IP=$((101 + RANDOM % 150))
            ip addr add 192.168.100.$RAND_IP/24 dev wlan0
            ip route add default via 192.168.100.100
        elif [ $counter -ge 20 ]; then
            build_infoPanel_and_log "Failed to connect!" "Could not get an IP in 20 seconds.\n Try again"
            sleep 1
            cleanup
        fi
    done
}

wait_for_connectivity() {
    build_infoPanel_and_log "Connecting..."  "Trying to reach $hostip..."
    counter=0

    while ! ping -c 1 -W 1 $hostip > /dev/null 2>&1; do
        sleep 0.5
        counter=$((counter+1))

        if [ $counter -ge 40 ]; then
            build_infoPanel_and_log "Failed to connect!"  "Could not reach $IP in 20 seconds."
            notify_stop
        fi
    done

    build_infoPanel_and_log "Joined hotspot!"  "Successfully reached the Hotspot! \n IP: $hostip" 
    sleep 1
}

# Download the cookie from the host, check whether it downloaded and make sure it still exists on the client before we move on
download_cookie() {
	build_infoPanel_and_log "Downloading cookie" "Retrieving info from host..."
	local output_path="/mnt/SDCARD/RetroArch/retroarch.cookie.client"
	curl -o "$output_path" ftp://$hostip/mnt/SDCARD/RetroArch/retroarch.cookie

	if [ $? -ne 0 ]; then
		build_infoPanel_and_log "Failed" "Can't download the cookie, can't continue"
		sleep 1
		cleanup
	fi

	if [ ! -f $output_path ]; then
		build_infoPanel_and_log "No cookie found" "Cookie has been eaten, can't continue"
		sleep 1
		cleanup
	fi

	build_infoPanel_and_log "Success!" "Got the cookie"
}

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
			"[cpuspeed]: "*)
				cpuspeed="${line##"[cpuspeed]: "}"
				;;
		esac
		log "$core $rom $coresize $corechksum $romsize $romchksum"
	done < "/mnt/SDCARD/RetroArch/retroarch.cookie.client"

	#url encode or curl complains
	export core_url=$(url_encode "$core")
	export rom_url=$(url_encode "$rom")

	log "Cookie file read"
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
                sleep 2
                cleanup
			fi
		else
			if [ "$file_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				build_infoPanel_and_log "Syncing" "$file_type doesn't exist locallyand too big to be syncing with host."
                sleep 2
                cleanup
			else
				build_infoPanel_and_log "Syncing" "$file_type doesn't exist locally; syncing with host."
				do_sync_file "$file_type" "$file_path" "$file_url"

				if [ ! -e "$file_path" ]; then
					build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
					sleep 2
					cleanup
				else
					# Refreshing roms list
					romdirname=$(echo "$rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
					rm "${rom%/*}/${romdirname}_cache6.db"
					build_infoPanel_and_log "Syncing" "$file_type synced."
				fi
			fi
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
                sleep 2
				cleanup
			else
				build_infoPanel_and_log "Syncing" "$file_type synced."
			fi
		fi
	fi
}


# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
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
	
	# We set core CPU speed for Netplay
	if [ -n "$cpuspeed" ]; then
		PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
		echo -n $cpuspeed > "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
	
	cd /mnt/SDCARD/RetroArch
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -C $hostip -L "$core" "$rom"
	
	# We restore previous core CPU speed
	if [ -n "$PreviousCPUspeed" ]; then
		echo -n $PreviousCPUspeed > "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
}

###########
#Utilities#
###########

# URL encode helper
url_encode(){
  encoded_str=`echo "$*" | awk '
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
' `
echo "$encoded_str"
}

wifi_disabled() {
    [ $(/customer/app/jsonval wifi) -eq 0 ]
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
	game_name=$(awk -F'/' '/\[rom\]:/ {print $NF}' /mnt/SDCARD/RetroArch/retroarch.cookie.client | sed 's/\(.*\)\..*/\1/')
}

# If we're currently connected to wifi, make a backup of the wpa_supplicant.conf to restore later
save_wifi_state() {
	IFACE=wlan0
	cp /appconfigs/wpa_supplicant.conf /tmp/wpa_supplicant.conf_bk
	old_ipv4=$(ip -4 addr show $IFACE | grep -o 'inet [^ ]*' | cut -d ' ' -f 2)
	ip addr del $old_ipv4/$old_mask dev $IFACE
}

# try and reset the IP and supp conf
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
	fi
	log "Old DHCP proc killed."
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

cleanup() {
    build_infoPanel_and_log "Cleanup" "Cleaning up after netplay session..."

    pkill -9 pressMenu2Kill
    
    if is_running infoPanel; then
        killall -9 infoPanel
    fi
        
    sync
    
    restore_wifi_state

    log "Cleanup done"
    exit
}


#########
##Main.##
#########

lets_go() {
	pressMenu2Kill $(basename $0) &
	check_wifi
	connect_to_host
	wait_for_ip
    wait_for_connectivity
	download_cookie
	read_cookie
	sync_file Rom "$rom" "$romcheck" "$rom_url"
	sync_file Core "$core" "$corecheck" "$core_url"
	stripped_game_name
	confirm_join_panel "Join now?" "$game_name"
	pkill -9 pressMenu2Kill
	start_retroarch
	cleanup
}

lets_go
