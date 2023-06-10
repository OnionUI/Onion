# GLO HOST
# Script to:
# 	Start hotspot, 
# 	Create a cookie file containing details for the client, 
# 	Start FTP to be able to host this file, 
# 	Start RA as a netplay host with -H, the core path and the rom path.
# 	Leave WPS commented out until it's further tested, it works but leads to an unstable connection. Possible issue with the new hostapd binary.
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..

check_wifi(){
if ifconfig wlan0 &>/dev/null; then
	sleep 1
	log "GLO::Retro_Quick_Host: Wi-Fi is up already"
	infoPanel --title "WIFI" --message "Wifi up" --auto
	break
else
	log "GLO::Retro_Quick_Host: Wi-Fi disabled, trying to enable before connecting.."
	infoPanel --title "WIFI" --message "Wifi starting..." --auto
	/customer/app/axp_test wifion
	sleep 2
	ifconfig wlan0 up
	udhcpc_control
	$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	sleep 2
	
	if is_running wpasupplicant && is_running udhcpc && ifconfig wlan0; then
		log "GLO::Retro_Quick_Host: WiFi started"
		infoPanel --title "WIFI" --message "Wifi started" --auto
	fi
fi
}


# We'll need hotspot to host the local connection
start_hotspot() { 
	if is_running hostapd; then
		killall -9 hostapd
	fi
	
	if is_running dnsmasq; then
		killall -9 dnsmasq
	fi
	
	ifconfig wlan1 up 
	ifconfig wlan0 down # Put wlan0 down to suspend the current wifi connections
		
	hotspot0addr=$(grep -E '^dhcp-range=' "$sysdir/config/dnsmasq.conf" | cut -d',' -f1 | cut -d'=' -f2) 
	hotspot0addr=$(echo $hotspot0addr | awk -F'.' -v OFS='.' '{$NF-=1; print}') 
	gateway0addr=$(grep -E '^dhcp-option=3' $sysdir/config/dnsmasq.conf | awk -F, '{print $2}')
	subnetmask=$(grep -E '^dhcp-range=.*,[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+,' "$sysdir/config/dnsmasq.conf" | cut -d',' -f3) 
	
	ifconfig wlan1 $hotspot0addr netmask $subnetmask 
	
	infoPanel --title "Hotspot" --message "Starting hotspot..." --auto
	
	$sysdir/bin/hostapd -P /var/run/hostapd.pid -B -i wlan1 $sysdir/config/hostapd.conf &
	$sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root & 
	ip route add default via $gateway0addr
	
	if is_running hostapd; then
		log "GLO::Retro_Quick_Host: Started with IP of: $hotspot0addr, subnet of: $subnetmask"
	else
		log "GLO::Retro_Quick_Host: Failed to start, please try turning off/on. If this doesn't resolve the issue reboot your device."
		infoPanel --title "Hotspot" --message "Failed to start hotspot, exiting.." --auto
		sleep 2
		exit
	fi
	
	if is_running wpa_supplicant; then # kill the sup, this means wifi will come back up after quitting RA.
		killall -9 wpa_supplicant
	fi
	
	if is_running udhcpc; then 
		killall -9 udhcpc
	fi
}

# We'll need FTP to host the cookie to the client - use the built in FTP, it allows us to curl (errors on bftpd re: path)
start_ftp(){
if is_running bftpd; then
	log "GLO::Retro_Quick_Host: FTP already running, killing to rebind"
	killall -9 bftpd
	killall -9 tcpsvd
	tcpsvd -E 0.0.0.0 21 ftpd -w / &
else
	tcpsvd -E 0.0.0.0 21 ftpd -w / &
	log "GLO::Retro_Quick_Host: Starting FTP server"
fi
}

# We'll need WPS to make a hands off connection (even if the user has changed the hotspot password from the default)
# start_wps(){
	# touch /tmp/hostapd.psk
	# sync
	# sleep 1
	# $sysdir/bin/hostapd_cli wps_pbc
	# log "GLO::Retro_Quick_Host: Starting WPS host"
# }

# Pull the cookie info that the GLO script has generated
get_cookie_info() {
    COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"

    if [ -f "$COOKIE_FILE" ]; then
        host_core=$(grep '\[core\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs) && export host_core
        host_rom=$(grep '\[rom\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs) && export host_rom
	else
		log "GLO::Retro_Quick_Host: No cookie found!"
    fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch(){
	infoPanel --title "RetroArch" --message "Starting RetroArch..." --auto
	killall -9 infoPanel
	cd /mnt/SDCARD/RetroArch
	HOME=/mnt/SDCARD/RetroArch ./retroarch -H -v -L "$host_core" "$host_rom"
}

cleanup(){
	if is_running hostapd; then
		killall -9 hostapd
	fi

	if is_running dnsmasq; then
		killall -9 dnsmasq
	fi
	
	if is_running tcpsvd; then
		killall -9 tcpsvd
	fi
	
	if is_running retroarch; then
		killall -9 retroarch
	fi
	
	if is_running infoPanel; then
		killall -9 infoPanel
	fi
	
	ifconfig wlan1 down
	ifconfig wlan0 up
	
	log "GLO::Retro_Quick_Host: Cleanup done"
	
}

###########
#Utilities#
###########

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

log() {
    echo "$(date)" $* >> $sysdir/logs/ra_quick_host.log
}

#########
##Main.##
#########

lets_go(){
check_wifi
start_hotspot
start_ftp
get_cookie_info
start_retroarch
cleanup
}

lets_go