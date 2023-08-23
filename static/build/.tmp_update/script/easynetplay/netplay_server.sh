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

logfile=easy_netplay
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

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
		log "Started with IP of: $hotspot0addr, subnet of: $subnetmask"
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


# Find the recommended core for the current system.
Get_NetplayCore() {

	platform=$(echo "$cookie_rom_path" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
	netplaycore_info=$(grep "^${platform};" "$sysdir/script/netplay/netplay_cores.cfg")
	if [ -n "$netplaycore_info" ]; then
		netplaycore=$(echo "$netplaycore_info" | cut -d ';' -f 2)
		netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/$netplaycore"
		core_config_folder=$(echo "$netplaycore_info" | cut -d ';' -f 3)
		cpuspeed=$(echo "$netplaycore_info" | cut -d ';' -f 4)

		if [ -n "$netplaycore" ]; then
			if [ "$netplaycore" = "none" ]; then
				build_infoPanel_and_log "Netplay impossible" "$platform not compatible with Netplay"
				sleep 3
				return 1
			fi
		else
			netplaycore="$cookie_core_path"
		fi
	fi
	return 0


}


# Create a cookie with all the required info for the client. (client will use this cookie)
create_cookie_info() {
	COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"
	MAX_FILE_SIZE_BYTES=26214400

		echo "[core]: $netplaycore" > "$COOKIE_FILE"
		echo "[rom]: $cookie_rom_path" >> "$COOKIE_FILE"


		if [ -s "$netplaycore" ]; then
			core_size=$(stat -c%s "$netplaycore")
			if [ "$core_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				echo "[coresize]: $core_size" >> "$COOKIE_FILE"
				log "Writing core size"
			else
				echo "[corechksum]: $(cksum "$netplaycore" | cut -f 1 -d ' ')" >> "$COOKIE_FILE"
				log "Writing core checksum"
			fi
		fi

		if [ -s "$cookie_rom_path" ]; then
			rom_size=$(stat -c%s "$cookie_rom_path")
			if [ "$rom_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
				echo "[romsize]: $rom_size" >> "$COOKIE_FILE"
				log "Writing rom size"
			else
				echo "[romchksum]: $(cksum "$cookie_rom_path" | cut -f 1 -d ' ')" >> "$COOKIE_FILE"
				log "Writing rom checksum"
			fi
		fi

		if [ -s "$cpuspeed" ]; then
				echo "[cpuspeed]: $cpuspeed" >> "$COOKIE_FILE"
				log "Writing cpuspeed: $cpuspeed"
		fi

}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {
	log "RetroArch" "Starting RetroArch..."
	echo "*****************************************"
	echo "romfullpath: $cookie_rom_path"
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
	# sleep 5
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -H -L "$netplaycore" "$cookie_rom_path"
	# We restore previous core CPU speed
	if [ -n "$PreviousCPUspeed" ]; then
		echo -n $PreviousCPUspeed > "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
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

    # Remove some files we prepared and received
	log "Removing stale files"
	rm "/mnt/SDCARD/RetroArch/retroarch.cookie"
	
	log "Cleanup done"
	exit

}

###########
#Utilities#
###########

build_infoPanel_and_log() {
	local title="$1"
	local message="$2"

	log "Info Panel: \n\tStage: $title\n\tMessage: $message"
	
	infoPanel --title "$title" --message "$message" --persistent &
	touch /tmp/dismiss_info_panel
	sync
	sleep 0.3
}

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
	Get_NetplayCore
	create_cookie_info
	pkill -9 pressMenu2Kill
	start_retroarch
	cleanup
}

lets_go
