sysdir=/mnt/SDCARD/.tmp_update
export WPACLI=/customer/app/wpa_cli

#### allow to run this script outside GLO : ####

[ -z "$old_ipv4" ] && old_ipv4=$(cat /tmp/old_ipv4.txt)

rm -f /tmp/old_ipv4.txt

if type log >/dev/null 2>&1; then
	log_func="log"
else
	log_func="echo -e"
fi
################################################

# try and reset the IP and supp conf
Stop_hotspot_Client() {
	# mv /tmp/wpa_supplicant.conf_bk /appconfigs/wpa_supplicant.conf
	ip link set wlan1 down
	ip addr flush dev wlan1
	pkill -9 -f wpa_supplicant
	/mnt/SDCARD/miyoo/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	# sync

	ip_output=$(ip link set wlan0 down 2>&1)
	if [ $? -ne 0 ]; then
		$log_func "Failed to bring down the interface."
		$log_func "Output from 'ip link set down' command: $ip_output"
	fi

	ip -4 addr show wlan0 | awk '/inet/ {print $2}' | while IFS= read -r line; do
		ip addr del "$line" dev wlan0
	done

	if [ -z "$old_ipv4" ]; then
		$log_func "Old IP address not found."

	else
		$log_func "Old IP address found : $old_ipv4"
		ip_output=$(ip addr add $old_ipv4 dev wlan0 2>&1)
		if [ $? -ne 0 ]; then
			$log_func "Failed to assign the old IP address."
			$log_func "Output from 'ip addr add' command: $ip_output"
		fi
	fi

	ip_output=$(ip link set wlan0 up 2>&1)
	if [ $? -ne 0 ]; then
		$log_func "Failed to bring up the interface."
		$log_func "Output from 'ip link set up' command: $ip_output"
	fi
	udhcpc_control
}

Stop_hotspot_Server() {
	$sysdir/bin/hostapd_cli all_sta flush
	killall -9 hostapd
	killall -9 dnsmasq
	killall -9 tcpsvd
	ip link set wlan1 down
	ip addr flush dev wlan1
	ifconfig wlan0 up
	if ! pgrep "wpa_supplicant" >/dev/null; then
		/mnt/SDCARD/miyoo/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	fi
	udhcpc_control
	$WPACLI reconfigure
}

udhcpc_control() {
	if pgrep udhcpc >/dev/null; then
		killall -9 udhcpc
	fi
	$log_func "Old DHCP proc killed."
	sleep 0.5
	$log_func "Trying to start DHCP"
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
	if pgrep "udhcpc" >/dev/null; then
		$log_func "DHCP started"
	else
		$display_func "DHCP" "Unable to start DHCP client\n unable to continue."
	fi
}

if pgrep "hostapd" >/dev/null; then
	$log_func "Running cleanup for Hotspot server."
	Stop_hotspot_Server
else
	$log_func "Running cleanup for Hotspot client."
	Stop_hotspot_Client
fi
