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
	infoPanel --title "WIFI" --message "Wifi up" --auto
	save_wifi_state
else
	log "GLO::Retro_Quick_Host: Wi-Fi disabled, trying to enable before connecting.."
	
	infoPanel --title "WIFI" --message "Wifi disabled, starting..." --auto
	
	/customer/app/axp_test wifion
	sleep 2
	ifconfig wlan0 up
	sleep 1
	$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	
	if is_running wpa_supplicant && ifconfig wlan0 > /dev/null 2>&1; then
		log "GLO::Retro_Quick_Host: WiFi started"
		infoPanel --title "WIFI" --message "Wifi started." --auto
	fi
	
	sleep 2 
fi
}


# Create a new network id, set it up and enable it, start udhcpc against it.
connect_to_host() {
	infoPanel --title "Connecting..." --message "Trying to join the hotspot..." --auto

	export new_id=$($WPACLI -i wlan0 add_network)
	log "GLO::Retro_Quick_Host: Added new network with id $new_id"

	$WPACLI -i wlan0 <<-EOF
	set_network $new_id ssid "MiyooMini+APOnionOS"
	set_network $new_id psk "onionos+"
	disable_network all
	select_network $new_id
	enable_network $new_id
	save_config
	reconfigure
	quit
	EOF
	
	udhcpc_control

	log "GLO::Retro_Quick_Host: Added new network and connected"
	log "###############################################################"
	cat /appconfigs/wpa_supplicant.conf >> $sysdir/logs/ra_quick_host.log
	log "###############################################################"
	sleep 0.5
}


# We'd better wait for an ip address to be assigned before going any further.
wait_for_ip() {
    local IP=$(ip route get 1 2>/dev/null | awk '{print $NF;exit}')
	infoPanel --title "Connecting..." --message "Waiting for an IP..." --auto

    while [ -z "$IP" ]; do
        IP=$(ip route get 1 2>/dev/null | awk '{print $NF;exit}')
        sleep 1
    done
	
	infoPanel --title "Joined hotspot!" --message "IP: $IP" --auto
    log "GLO::Retro_Quick_Host: IP address assigned: $IP"
	sleep 1
}

# Download the cookie from the host, check whether it downloaded and make sure it still exists on the client before we move on
download_cookie() {
	infoPanel --title "Downloading cookie" --message "Retrieving info from host..." --auto
    local output_path="/mnt/SDCARD/RetroArch/retroarch.cookie.client"
    curl -o "$output_path" ftp://$hostip/mnt/SDCARD/RetroArch/retroarch.cookie

    if [ $? -ne 0 ]; then
        log "GLO::Retro_Quick_Host: Failed to download cookie file."
		infoPanel --title "Failed" --message "Can't download the cookie, can't continue" --auto
		sleep 1
        exit
    fi

    if [ ! -f $output_path ]; then
        log "GLO::Retro_Quick_Host: We didn't get a cookie"
		infoPanel --title "No cookie found" --message "Cookie has been eaten, can't continue" --auto
		sleep 1
        exit
    fi
	
	infoPanel --title "Success!" --message "Got the cookie" --auto
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
			"[corechksum]: "*)
				corechksum="${line##"[corechksum]: "}"
				export corechksum
				;;
			"[romchksum]: "*)
				romchksum="${line##"[romchksum]: "}"
				export romchksum
				;;
		esac
	done <"/mnt/SDCARD/RetroArch/retroarch.cookie.client"
	
	#url encode or curl complains
	export rom_url=$(echo "$rom" | sed 's/ /%20/g')
	export core_url=$(echo "$core" | sed 's/ /%20/g')

	log "GLO::Retro_Quick_Host: Cookie file read"
}

# See if we need to download the rom - check if we have the file first. If we do check; if the checksum matches, if it doesn't; rename the existing file and sync it from the host..
sync_rom() {
    if [ -z "$rom" ]; then
        return
		log "GLO::Retro_Quick_Host: Potentially starting a contentless core."
    fi

    if [ -e "$rom" ]; then
        log "GLO::Retro_Quick_Host: $rom exists."
        local rom_chksum_actual=$(cksum "$rom" | awk '{ print $1 }')
        if [ "$romchksum" -ne "$rom_chksum_actual" ]; then
            log "GLO::Retro_Quick_Host: Checksum doesn't match for $rom. Renaming the existing file and syncing rom again."
			infoPanel --title "Rom Syncing" --message "Rom checksums don't match, syncing" --auto
			sleep 0.5
			do_sync_rom
        else
            log "GLO::Retro_Quick_Host: $rom exists and the checksum matches."
            infoPanel --title "Rom Synced!" --message "Rom checksums match, no sync required" --auto
        fi
    else
		infoPanel --title "Syncing" --message "Rom doesn't exist locally; syncing with host." --auto
		log "GLO::Retro_Quick_Host: $rom doesn't exist. Syncing."
		do_sync_rom
    fi
}

# See if we need to download the core - check if we have the file first. If we do; check if the checksum matches, if it doesn't; rename the existing file and sync it from the host.
sync_core() {
    if [ -e "$core" ]; then
        log "GLO::Retro_Quick_Host: $core exists."
        local core_chksum_actual=$(cksum "$core" | awk '{ print $1 }')
        if [ "$corechksum" -ne "$core_chksum_actual" ]; then
            log "GLO::Retro_Quick_Host: Checksum doesn't match for $core. Renaming the existing file and Syncing again."
			infoPanel --title "Core Syncing" --message "Core checksums don't match, syncing" --auto
			sleep 0.5
            do_sync_core
        else
            log "GLO::Retro_Quick_Host: $core exists and the checksum matches."
			infoPanel --title "Core Synced!" --message "Core checksums match, no sync required" --auto
        fi
    else
		infoPanel --title "Syncing" --message "Core doesn't exist locally; syncing with host." --auto
		log "GLO::Retro_Quick_Host: $core doesn't exist. Syncing."
		do_sync_core
    fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch(){
	infoPanel --title "Starting RA" --message "Starting RetroArch" --auto
	cd /mnt/SDCARD/RetroArch
	HOME=/mnt/SDCARD/RetroArch ./retroarch -C $hostip -v -L "$core" "$rom"
}

###########
#Utilities#
###########

# If we're currently connected to wifi, save the network ID so we can reconnect after we're done with retroarch - save the IP address and subnet so we can restore these.
save_wifi_state() {
    export IFACE=wlan0
    export old_id=$(wpa_cli -i $IFACE list_networks | awk '/CURRENT/ {print $1}')
	export old_ipv4=$(ip -4 addr show $IFACE | grep -o 'inet [^ ]*' | cut -d ' ' -f 2)
}

do_sync_rom() {
	mv "$rom" "${rom}_old"
	curl -o "$rom" "ftp://$hostip/$rom_url" >> $sysdir/logs/ra_quick_host.log
	infoPanel --title "Syncing" --message "Rom synced" --auto
}

do_sync_core() {
	mv "$core" "${core}_old"
	curl -o "$core" "ftp://$hostip/$core_url"
	infoPanel --title "Syncing" --message "Core synced." --auto
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
		log "GLO::Retro_Quick_Host: DHCP failed to start"
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
	
	$WPACLI -i $IFACE <<-EOF # remove our hotspot network and connect back to original id
	remove_network $new_id
	select_network $old_id
	enable_network $old_id
	save_config
	quit
	EOF
	
	ip addr add $old_ipv4 dev wlan0
	ip link set dev wlan0 up
	
	sleep 1
		
	log "GLO::Retro_Quick_Host: Cleanup done"
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
	sync_rom
	sync_core
	start_retroarch
	cleanup
	
}

lets_go