# GLO CLIENT
# Script to:
	# Enable Wifi
	# Pull the cookie file from the host
	# Pull the core and the rom from the host based on what the cookie file gives us
	# Start retroarch with the above passed, and -C provided to connect.

# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export WPACLI=/customer/app/wpa_cli
export hostip="192.168.100.100" # This should be the default unless the user has changed it..

##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..

check_wifi(){
ifconfig wlan1 down
	if ifconfig wlan0 &>/dev/null; then
		log "GLO::Retro_Quick_Host: Wi-Fi is up already"
		build_infoPanel "WIFI" "Wifi up"
		save_wifi_state
	else
		log "GLO::Retro_Quick_Host: Wi-Fi disabled, trying to enable before connecting.."
		build_infoPanel "WIFI" "Wifi disabled, starting..." 
		
		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		sleep 1
		$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
		
		if is_running wpa_supplicant && ifconfig wlan0 > /dev/null 2>&1; then
			log "GLO::Retro_Quick_Host: WiFi started"
			build_infoPanel "WIFI" "Wifi started."
		else
			log "GLO::Retro_Quick_Host: WiFi started"
			build_infoPanel "WIFI" "Unable to start WiFi\n unable to continue."
			sleep 1
			exit
		fi
		
		sleep 2 
	fi
}


# Create a new network id, set it up and enable it, start udhcpc against it.
connect_to_host() {
	build_infoPanel "Connecting..." "Trying to join the hotspot..."

	export new_id=$($WPACLI -i wlan0 add_network)
	if [ -z "$new_id" ]; then
		build_infoPanel "Failed"  "Failed to create network\n unable to continue." 
		return 1
	fi
	
	log "GLO::Retro_Quick_Host: Added new network with id $new_id"

	net_setup=$($WPACLI -i wlan0 <<-EOF
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
		build_infoPanel "Failed"  "Failed to configure the network\n unable to continue." 
		sleep 1
		exit
	fi
	

	udhcpc_control
	
	log "GLO::Retro_Quick_Host: Added new network and connected"
	log "#############################Supplicant dump##################################"
	cat /appconfigs/wpa_supplicant.conf >> $sysdir/logs/ra_quick_host.log
	log "#############################Supplicant dump##################################"
	sleep 0.5
}


# We'd better wait for an ip address to be assigned before going any further.
wait_for_ip() {
    local IP=$(ip route get 1 2>/dev/null | awk '{print $NF;exit}')
    build_infoPanel "Connecting..."  "Waiting for an IP..."
    local counter=0

    while [ -z "$IP" ]; do
        IP=$(ip route get 1 2>/dev/null | awk '{print $NF;exit}')
        sleep 1
        counter=$((counter+1))

        if [ $counter -ge 20 ]; then
            build_infoPanel "Failed to connect!"  "Could not get an IP in 20 seconds."
            log "GLO::Retro_Quick_Host: Failed to get an IP address within 20 seconds."
            return 1
        fi
    done

    build_infoPanel "Joined hotspot!"  "IP: $IP" 
    log "GLO::Retro_Quick_Host: IP address assigned: $IP"
    sleep 1
}

# Download the cookie from the host, check whether it downloaded and make sure it still exists on the client before we move on
download_cookie() {
	build_infoPanel "Downloading cookie"  "Retrieving info from host..." 
    local output_path="/mnt/SDCARD/RetroArch/retroarch.cookie.client"
    curl -o "$output_path" ftp://$hostip/mnt/SDCARD/RetroArch/retroarch.cookie

    if [ $? -ne 0 ]; then
        log "GLO::Retro_Quick_Host: Failed to download cookie file."
		build_infoPanel "Failed"  "Can't download the cookie, can't continue" 
		sleep 1
        exit
    fi

    if [ ! -f $output_path ]; then
        log "GLO::Retro_Quick_Host: We didn't get a cookie"
		build_infoPanel "No cookie found"  "Cookie has been eaten, can't continue" 
		sleep 1
        exit
    fi
	
	build_infoPanel "Success!"  "Got the cookie" 
    log "GLO::Retro_Quick_Host: Cookie file downloaded successfully."
}

# Read the cookie and store the paths and checksums into a var.
read_cookie() {
	while IFS= read -r line; do
		case $line in
			"[core]: "*)
				core="${line##"[core]: "}"
				export core
				;;
			"[rom]: "*)
				rom="${line##"[rom]: "}"
				export rom
				;;
		esac
	done <"/mnt/SDCARD/RetroArch/retroarch.cookie.client"

    export romchksum=$(cksum "$rom" | cut -f 1 -d ' ')
    export corechksum=$(cksum "$core" | cut -f 1 -d ' ')
	
	#url encode or curl complains
	export rom_url=$(echo "$rom" | sed 's/ /%20/g')
	export core_url=$(echo "$core" | sed 's/ /%20/g')

	log "GLO::Retro_Quick_Host: Cookie file read"
}

sync_file() {
    file_type="$1"
    file_path="$2"
    file_checksum="$3"
    file_url="$4"

    if [ -z "$file_path" ]; then
        log "GLO::Retro_Quick_Host: Potentially starting a contentless $file_type."
        return
    fi

    if [ -e "$file_path" ]; then
        log "GLO::Retro_Quick_Host: $file_path exists."
        local file_chksum_actual=$(cksum "$file_path" | awk '{ print $1 }')
        if [ "$file_checksum" -ne "$file_chksum_actual" ]; then
            log "GLO::Retro_Quick_Host: Checksum doesn't match for $file_path. Renaming the existing file and syncing $file_type again."
            build_infoPanel "Syncing"  "$file_type checksums don't match, syncing" 
            sleep 0.5
            do_sync_file "$file_type" "$file_path" "$file_url"
            
            if [ ! -e "$file_path" ]; then
                build_infoPanel "Sync Failed"  "Failed to download the $file_type file." 
                exit 1
			else
				build_infoPanel "Syncing"  "$file_type synced." 
            fi

        else
            log "GLO::Retro_Quick_Host: $file_path exists and the checksum matches."
            build_infoPanel "$file_type synced!"  "$file_type checksums match, no sync required" 
        fi
    else
        build_infoPanel "Syncing"  "$file_type doesn't exist locally; syncing with host." 
        log "GLO::Retro_Quick_Host: $file_path doesn't exist. Syncing."
        do_sync_file "$file_type" "$file_path" "$file_url"
        
        if [ ! -e "$file_path" ]; then
            build_infoPanel "Sync Failed"  "Failed to download the $file_type file." 
            exit 1
		else
			build_infoPanel "Syncing"  "$file_type synced." 
		fi

    fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch(){
	build_infoPanel "Starting RA"  "Starting RetroArch" 
	cd /mnt/SDCARD/RetroArch
	HOME=/mnt/SDCARD/RetroArch ./retroarch -C $hostip -v -L "$core" "$rom"
}

###########
#Utilities#
###########

build_infoPanel() {
    local title="$1"
    local message="$2"
    
    infoPanel --title "$title" --message "$message" --auto
}

# If we're currently connected to wifi, save the network ID so we can reconnect after we're done with retroarch - save the IP address and subnet so we can restore these.
save_wifi_state() {
    export IFACE=wlan0
    export old_id=$(wpa_cli -i $IFACE list_networks | awk '/CURRENT/ {print $1}')
	export old_ipv4=$(ip -4 addr show $IFACE | grep -o 'inet [^ ]*' | cut -d ' ' -f 2)
	ip addr del $old_ip/$old_mask dev $IFACE
}

restore_wifi_state() {
    if [ -z "$old_ipv4" ]; then
        log "GLO::Retro_Quick_Host: Old IP address not found."
    fi
   
    ip_output=$(ip link set wlan0 down 2>&1)
    if [ $? -ne 0 ]; then
        log "GLO::Retro_Quick_Host: Failed to bring down the interface."
        log "GLO::Retro_Quick_Host: Output from 'ip link set down' command: $ip_output"
    fi

	ip -4 addr show wlan0 | awk '/inet/ {print $2}' | while IFS= read -r line
	do
		ip addr del "$line" dev wlan0
	done
	
    ip_output=$(ip addr add $old_ipv4 dev wlan0 2>&1)
    if [ $? -ne 0 ]; then
        log "GLO::Retro_Quick_Host: Failed to assign the old IP address."
        log "GLO::Retro_Quick_Host: Output from 'ip addr add' command: $ip_output"
    fi
    
    ip_output=$(ip link set wlan0 up 2>&1)
    if [ $? -ne 0 ]; then
        log "GLO::Retro_Quick_Host: Failed to bring up the interface."
        log "GLO::Retro_Quick_Host: Output from 'ip link set up' command: $ip_output"
    fi
}

do_sync_file() {
    file_type=$1
    file_path=$2
    file_url=$3

    mv "$file_path" "${file_path}_old"
    curl -o "$file_path" "ftp://$hostip/$file_url"
}

udhcpc_control() {
	if pgrep udhcpc > /dev/null; then
		killall -9 udhcpc
	fi
	log "GLO::Retro_Quick_Host: Old DHCP proc killed."
	sleep 1
	log "GLO::Retro_Quick_Host: Trying to start DHCP"
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script > /dev/null 2>&1 &
	if is_running udhcpc; then
		log "GLO::Retro_Quick_Host: DHCP started"
	else
		build_infoPanel "DHCP" "Unable to start DHCP client\n unable to continue."
	fi
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

log() {
    echo "$(date)" $* >> $sysdir/logs/ra_quick_host.log
}

cleanup(){
	if is_running retroarch; then
		killall -9 retroarch
	fi
	
	if is_running infoPanel; then
		killall -9 infoPanel
	fi
	
	net_setup=$($WPACLI -i $IFACE <<-EOF
	remove_network $new_id
	select_network $old_id
	enable_network $old_id
	save_config
	quit
	EOF
	)
	
	if [ $? -ne 0 ]; then
		log "GLO::Retro_Quick_Host: Failed to configure the network"
	fi
	
	sleep 1
	
	restore_wifi_state
		
	log "GLO::Retro_Quick_Host: Cleanup done"
	return 0
}

#########
##Main.##
#########

lets_go(){

	check_wifi
	connect_to_host
	wait_for_ip
	download_cookie
	read_cookie
	sync_file Rom "$rom" "$romchksum" "$rom_url"
	sync_file Core "$core" "$corechksum" "$core_url"
	start_retroarch
	cleanup
	
}

lets_go