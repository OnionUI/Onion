sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo

#### allow to run this script outside GLO : ####

[ -z "$logfile" ] && logfile=hotspot_create

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



# We'll need wifi up for this. Lets try and start it..

check_wifi() {
	ifconfig wlan1 down
	if ifconfig wlan0 &>/dev/null; then
		$display_func "WIFI" "Wifi up"
	else
		$display_func "WIFI" "Wifi disabled, starting..."

		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		sleep 1
		$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf

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

# We'll need hotspot to host the local connection
start_hotspot() {
	if pgrep "hostapd" >/dev/null; then
		killall -9 hostapd
	fi

	if pgrep "dnsmasq" >/dev/null; then
		killall -9 dnsmasq
	fi

	ifconfig wlan1 up
	ifconfig wlan0 down # Put wlan0 down to suspend the current wifi connections

	hotspot0addr=$(grep -E '^dhcp-range=' "$sysdir/config/dnsmasq.conf" | cut -d',' -f1 | cut -d'=' -f2)
	hotspot0addr=$(echo $hotspot0addr | awk -F'.' -v OFS='.' '{$NF-=1; print}')
	gateway0addr=$(grep -E '^dhcp-option=3' $sysdir/config/dnsmasq.conf | awk -F, '{print $2}')
	subnetmask=$(grep -E '^dhcp-range=.*,[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+,' "$sysdir/config/dnsmasq.conf" | cut -d',' -f3)

	ifconfig wlan1 $hotspot0addr netmask $subnetmask

	$display_func "Hotspot" "Starting hotspot..."

	$sysdir/bin/hostapd -P /var/run/hostapd.pid -B -i wlan1 $sysdir/config/hostapd.conf &
	$sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root &
	ip route add default via $gateway0addr

	if pgrep "hostapd" >/dev/null; then
		$log_func "Started with IP of: $hotspot0addr, subnet of: $subnetmask"
	else
		$display_func "Hotspot" "Failed to start hotspot, exiting.."
		sleep 2
		$cleanup_func
	fi

	if pgrep "wpa_supplicant" >/dev/null; then # kill the sup, this means wifi will come back up after quitting RA.
		killall -9 wpa_supplicant
	fi

	if pgrep "udhcpc" >/dev/null; then
		killall -9 udhcpc
	fi
}

check_wifi
start_hotspot
