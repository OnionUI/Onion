#!/bin/sh

sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
WPA="wpa_cli -i wlan0"

logfile=wifi
. $sysdir/script/log.sh

wifi_enabled() {
	if ifconfig wlan0 &>/dev/null; then
		echo 1
	else
		echo 0
	fi
}

enable_wifi() {
	/customer/app/axp_test wifion
	sleep 2
	ifconfig wlan0 up
	sleep 1
	$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
}

disable_wifi() {
	pkill -9 wpa_supplicant
	pkill -9 udhcpc
	/customer/app/axp_test wifioff
}

is_known() {
	list_known_wifi | grep -e $'\t'"$1"$'\t' | cut -f1
}

connect_wifi() {
	id=$(is_known "$1")
	if [ -z "$id" ]; then
		log "Adding new Wi-Fi network: $1"
		network_id=$($WPA add_network | tail -n 1)
		pkill -9 udhcpc
		$WPA set_network "$network_id" ssid "\"$1\""
		if [ -n "$2" ]; then
			$WPA set_network "$network_id" psk "\"$2\""
		else
			$WPA set_network "$network_id" key_mgmt NONE
		fi
		$WPA enable_network "$network_id"
		$WPA select_network "$network_id"
		$WPA reassociate
		$WPA save_config
	else
		log "Connecting to known Wi-Fi network: $1 (ID:$id)"
		pkill -9 udhcpc
		$WPA select_network "$id"
		$WPA reassociate
		$WPA save_config
	fi

	pkill -9 wpa_supplicant
	$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf >/dev/null 2>&1 &
	sleep 1
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
}

get_connected_wifi() {
	$WPA status | grep '^ssid=' | cut -d'=' -f2
}

list_known_wifi() {
	$WPA list_networks | tail -n+2
}

delete_known_wifi() {
	if [ -n "$1" ]; then

		$WPA remove_network "$1"
		$WPA save_config
	else
		echo "Usage: $0 delete <ID>"
		exit 1
	fi
}

delete_all_known_wifi() {
	$WPA remove_network all
	$WPA save_config
}

scan() {
	$WPA scan
	sleep 2
	$WPA scan_results
}

reset_wifi() {
	pkill -9 wpa_supplicant
	$WPA remove_network all >/dev/null 2>&1
	$WPA save_config >/dev/null 2>&1
	rm -f /appconfigs/wpa_supplicant.conf >/dev/null 2>&1
	cat <<-EOF >/appconfigs/wpa_supplicant.conf
		ctrl_interface=/var/run/wpa_supplicant
		update_config=1
	EOF
	$WPA reconfigure >/dev/null 2>&1
}

case "$1" in
"on")
	enable_wifi
	;;
"off")
	disable_wifi
	;;
"enabled")
	wifi_enabled
	;;
"known")
	is_known "$2"
	;;
"connect")
	connect_wifi "$2" "$3"
	;;
"connected")
	get_connected_wifi
	;;
"list")
	list_known_wifi
	;;
"delete")
	delete_known_wifi "$2"
	;;
"delete_all")
	delete_all_known_wifi
	;;
"scan")
	scan "$2"
	;;
"reset")
	reset_wifi
	;;
*)
	echo "Usage: $0 {on|off|enabled|known|connect|connected|list|delete|delete_all|scan}"
	echo ""
	echo "Options:"
	echo "  on          Enable Wi-Fi"
	echo "  off         Disable Wi-Fi"
	echo "  enabled     Check if Wi-Fi is enabled"
	echo "  known       Check if a network is known, returns it's ID"
	echo "              Usage: $0 known <SSID>"
	echo "  connect     Connect to a Wi-Fi network"
	echo "              Usage: $0 connect <SSID> [PSK]"
	echo "  connected   Get the name of the connected Wi-Fi network"
	echo "  list        List known Wi-Fi networks"
	echo "  delete      Delete a known Wi-Fi network by its ID"
	echo "              Usage: $0 delete <ID>"
	echo "  delete_all  Delete all known Wi-Fi networks"
	echo "  scan        Scan for available Wi-Fi networks"
	echo "  reset       Reset Wi-Fi settings"
	exit 1
	;;
esac

exit 0
