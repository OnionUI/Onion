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
LOGGING=$([ -f $sysdir/config/.logging ] && echo 1 || echo 0)
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..

check_wifi() {
	ifconfig wlan1 down
	if ifconfig wlan0 &> /dev/null; then
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
			sleep 1
			cleanup
		fi

		sleep 2
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

	build_infoPanel_and_log "Hotspot" "Starting hotspot..."

	$sysdir/bin/hostapd -P /var/run/hostapd.pid -B -i wlan1 $sysdir/config/hostapd.conf &
	$sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root &
	ip route add default via $gateway0addr

	if is_running hostapd; then
		log "GLO::Easy_Netplay: Started with IP of: $hotspot0addr, subnet of: $subnetmask"
	else
		build_infoPanel_and_log "Hotspot" "Failed to start hotspot, exiting.."
		sleep 2
		cleanup
	fi

	if is_running wpa_supplicant; then # kill the sup, this means wifi will come back up after quitting RA.
		killall -9 wpa_supplicant
	fi

	if is_running udhcpc; then
		killall -9 udhcpc
	fi
}

# We'll need FTP to host the cookie to the client - use the built in FTP, it allows us to curl (errors on bftpd re: path)
start_ftp() {
	if is_running bftpd; then
		log "GLO::Easy_Netplay: FTP already running, killing to rebind"
		bftpd_p=$(ps | grep bftpd | grep -v grep | awk '{for(i=4;i<=NF;++i) printf $i" "}')
		killall -9 bftpd
		killall -9 tcpsvd
		tcpsvd -E 0.0.0.0 21 ftpd -w / &
	else
		tcpsvd -E 0.0.0.0 21 ftpd -w / &
		log "GLO::Easy_Netplay: Starting FTP server"
	fi
}

# Pull the cookie info that the GLO script has generated (and use it to write the cksum to the cookie so the client has it)
get_cookie_info() {
	COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"
	MAX_FILE_SIZE_BYTES=26214400

	if [ -f "$COOKIE_FILE" ]; then
		host_core=$(grep '\[core\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs) && export host_core
		host_rom=$(grep '\[rom\]' "$COOKIE_FILE" | cut -d ':' -f 2 | xargs) && export host_rom

		if [ -s "$host_core" ]; then
			core_size=$(stat -c%s "$host_core")
			if [ "$core_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				echo "[coresize]: $core_size" >> "$COOKIE_FILE"
				log "GLO::Easy_Netplay: Writing core size"
			else
				echo "[corechksum]: $(cksum "$host_core" | cut -f 1 -d ' ')" >> "$COOKIE_FILE"
				log "GLO::Easy_Netplay: Writing core checksum"
			fi
		fi

		if [ -s "$host_rom" ]; then
			rom_size=$(stat -c%s "$host_rom")
			if [ "$rom_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				echo "[romsize]: $rom_size" >> "$COOKIE_FILE"
				log "GLO::Easy_Netplay: Writing rom size"
			else
				echo "[romchksum]: $(cksum "$host_rom" | cut -f 1 -d ' ')" >> "$COOKIE_FILE"
				log "GLO::Easy_Netplay: Writing rom checksum"
			fi
		fi
	else
		log "GLO::Easy_Netplay: No cookie found!"
	fi
}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {
	build_infoPanel_and_log "RetroArch" "Starting RetroArch..."
	cd /mnt/SDCARD/RetroArch
	HOME=/mnt/SDCARD/RetroArch ./retroarch -H -v -L "$host_core" "$host_rom"
}

cleanup() {
	build_infoPanel_and_log "Cleanup" "Cleaning up after netplay session..."

	pkill -9 pressMenu2Kill

	if is_running hostapd; then
		killall -9 hostapd
	fi

	if is_running dnsmasq; then
		killall -9 dnsmasq
	fi

	if is_running tcpsvd; then
		killall -9 tcpsvd
	fi

	ifconfig wlan1 down

	ifconfig wlan0 up

	restore_ftp

	log "GLO::Easy_Netplay: Cleanup done"
	exit

}

###########
#Utilities#
###########

build_infoPanel_and_log() {
	local title="$1"
	local message="$2"

	if [ $LOGGING -eq 1 ]; then
		echo "$(date) GLO::Easy_Netplay: Stage: $title Message: $message" >> $sysdir/logs/easy_netplay.log
	fi
	
	infoPanel --title "$title" --message "$message" --persistent &
	touch /tmp/dismiss_info_panel
	sync
	sleep 0.5
}

log() {
	if [ $LOGGING -eq 1 ]; then
		echo "$(date)" $* >> $sysdir/logs/easy_netplay.log
	fi
}

restore_ftp() {
	log "GLO::Easy_Netplay: Restoring original FTP server"
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

#########
##Main.##
#########

lets_go() {
	pressMenu2Kill $(basename $0) &
	check_wifi
	start_hotspot
	start_ftp
	get_cookie_info
	pkill -9 pressMenu2Kill
	start_retroarch
	cleanup
}

lets_go
