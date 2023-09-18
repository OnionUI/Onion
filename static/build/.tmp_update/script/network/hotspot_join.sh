sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export WPACLI=/customer/app/wpa_cli
export hostip="192.168.100.100" # This should be the default unless the user has changed it..

#### allow to run this script outside GLO : ####

[ -z "$logfile" ] && logfile=hotspot_join

if type log >/dev/null 2>&1; then
	log_func="log"
else
	log_func="echo -e"
fi

if type build_infoPanel_and_log >/dev/null 2>&1; then
	display_func="build_infoPanel_and_log"
else
	display_func="echo -e"
fi

if type cleanup >/dev/null 2>&1; then
	cleanup_func="cleanup"
else
	cleanup_func="exit"
fi
################################################

check_wifi() {
	ifconfig wlan1 down
	$WPACLI save_config
	save_wifi_state
	sync
	if ifconfig wlan0 &>/dev/null; then
		$display_func "WIFI" "Wifi up"
	else
		$log_func "Wi-Fi disabled, trying to enable before connecting.."
		$display_func "WIFI" "Wifi disabled, starting..."

		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		sleep 1
		$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c $sysdir/config/hotspot_wpa_supplicant.conf

		if pgrep "wpa_supplicant" >/dev/null && ifconfig wlan0 >/dev/null 2>&1; then
			$display_func "WIFI" "Wifi started."
		else
			$display_func "WIFI" "Unable to start WiFi\n unable to continue."
			sleep 1
			$cleanup_func
		fi

		sleep 2
	fi
}

# Create a new network id, set it up and enable it, start udhcpc against it.
connect_to_host() {
	$display_func "Connecting..." "Trying to join the hotspot..."

	pkill -9 -f wpa_supplicant
	$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c $sysdir/config/hotspot_wpa_supplicant.conf

	#################### No more nand writing, avoid wifi problem on unexpected reboot ##########
	# new_id=$($WPACLI -i wlan0 add_network)
	# if [ -z "$new_id" ]; then
	# 	$display_func "Failed" "Failed to create network\n unable to continue."
	# 	$cleanup_func
	# fi

	# $log_func "Added new network with id $new_id"

	# net_setup=$(
	# 	$WPACLI -i wlan0 <<-EOF
	# 		set_network $new_id ssid "MiyooMini+APOnionOS"
	# 		set_network $new_id psk "onionos+"
	# 		disable_network all
	# 		select_network $new_id
	# 		enable_network $new_id
	# 		save_config
	# 		reconfigure
	# 		quit
	# 	EOF
	# )

	# if [ $? -ne 0 ]; then
	# 	$display_func "Failed" "Failed to configure the network\n unable to continue."
	# 	sleep 1
	# 	$cleanup_func
	# fi
	##########################################################################################
	udhcpc_control
	sleep 2
}

# We'd better wait for an ip address to be assigned before going any further.
wait_for_ip() {
	
	IP=""
	$display_func "Connecting..." "Waiting for an IP..."
	local counter=0
	ip addr flush dev wlan0

	while [ -z "$IP" ]; do
		IP=$(ip route get 1 2>/dev/null | awk '{print $NF;exit}')
		sleep 0.5
		counter=$((counter + 1))

		if [ $counter -eq 20 ]; then
			$display_func "Fallback" "Using static IP..."
			killall -9 udhcpc
			ip addr flush dev wlan0
			RAND_IP=$((101 + RANDOM % 150))
			ip addr add 192.168.100.$RAND_IP/24 dev wlan0
			ip route add default via 192.168.100.100
		elif [ $counter -ge 40 ]; then
			$display_func "Failed to connect!" "Could not get an IP in 20 seconds.\n Try again"
			sleep 1
			$cleanup_func
		fi
	done
}

wait_for_connectivity() {
	$display_func "Connecting..." "Trying to reach $hostip..."
	counter=0

	while ! ping -c 1 -W 1 $hostip >/dev/null 2>&1; do
		sleep 0.5
		counter=$((counter + 1))

		if [ $counter -ge 40 ]; then
			$display_func "Failed to connect!" "Could not reach $IP in 20 seconds."
			sleep 2
			$cleanup_func
		fi
	done

	$display_func "Joined hotspot!" "Successfully reached the Hotspot! \n IP: $hostip"
	sleep 0.5
}

# If we're currently connected to wifi, make a backup of the wpa_supplicant.conf to restore later
save_wifi_state() {
	# cp /appconfigs/wpa_supplicant.conf /tmp/wpa_supplicant.conf_bk
	old_ipv4=$(ip -4 addr show wlan0 | grep -o 'inet [^ ]*' | cut -d ' ' -f 2)
	ip addr del $old_ipv4/$old_mask dev wlan0
	echo "$old_ipv4" >/tmp/old_ipv4.txt
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

check_wifi
connect_to_host
wait_for_ip
wait_for_connectivity
