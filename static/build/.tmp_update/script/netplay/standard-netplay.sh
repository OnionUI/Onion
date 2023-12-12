# GLO NETPLAY HOST
# Script to:
#   Find the recommended core for the current system
# 	Start RA as a netplay host with -H, the slected netplay core path, the rom path and the RA config override to have a stable Netplay.
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
romfullpath="$1"
romname=$(basename "$1")
echo "cookie_core_path $cookie_core_path"
CurrentSystem=$(echo "$1" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
NetplayAction="$3" # host or join

logfile=netplay
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..
check_wifi() {
	ifconfig wlan1 down
	if ifconfig wlan0 &>/dev/null; then
		log "Wifi up"
	else
		build_infoPanel_and_log "WIFI" "Wifi disabled, starting..."

		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		sleep 1
		$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf

		if is_running wpa_supplicant && ifconfig wlan0 >/dev/null 2>&1; then
			build_infoPanel_and_log "WIFI" "Wifi started."
		else
			build_infoPanel_and_log "WIFI" "Unable to start WiFi\n unable to continue."
			sleep 1
		fi

		sleep 2
	fi
}

# Find the recommended core for the current system.
Get_NetplayCore() {
	platform="$1"
	netplaycore_info=$(grep "^${platform};" "$sysdir/config/netplay_cores.conf")

	if [ -n "$netplaycore_info" ]; then
		netplaycore=$(echo "$netplaycore_info" | cut -d ';' -f 2)
		core_config_folder=$(echo "$netplaycore_info" | cut -d ';' -f 3)
		cpuspeed=$(echo "$netplaycore_info" | cut -d ';' -f 4)

	fi

	if [ -n "$netplaycore" ]; then
		if [ "$netplaycore" = "none" ]; then
			build_infoPanel_and_log "Netplay impossible" "$platform not compatible with Netplay"
			sleep 3
			return 1
		fi
	else
		netplaycore="$cookie_core_path"
	fi
	return 0

}

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
start_retroarch() {

	# We set core CPU speed for Netplay
	if [ -n "$cpuspeed" ]; then
		PreviousCPUspeed=$(cat "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt")
		echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi

	# Starting Retroarch
	cd /mnt/SDCARD/RetroArch

	if [ "$NetplayAction" = "host" ]; then
		HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/netplay_override.cfg -H -L "$netplaycore" "$romfullpath"
	else
		HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/netplay_override.cfg -L "$netplaycore" "$romfullpath"

		# cp "./.retroarch/retroarch.cfg" "./.retroarch/netplay_alternative.cfg"
		# $sysdir/script/patch_ra_cfg.sh  "./.retroarch/netplay_override.cfg" "./.retroarch/netplay_alternative.cfg"
		# HOME=/mnt/SDCARD/RetroArch ./retroarch --config=./.retroarch/netplay_alternative.cfg -L "$core" "$romfullpath"
	fi

	# We restore previous core CPU speed
	if [ -n "$PreviousCPUspeed" ]; then
		echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	else
		rm -f "/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi
}

Check_PlayerName() {

	config_file="/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
	netplay_nickname=$(grep "^netplay_nickname" "$config_file" | awk -F '"' '{print $2}')

	if [ "$netplay_nickname" = "--0 Miyoo Mini+ - Onion 0--" ]; then
		new_nickname="0nion"
		sed -i "s/^netplay_nickname = .*/netplay_nickname = \"$new_nickname\"/" "$config_file"
		log "The new netplay_nickname value is: \"$new_nickname\""
	else
		if [ "${netplay_nickname#0nion}" = "$netplay_nickname" ]; then
			new_nickname="0nion - $netplay_nickname"
			sed -i "s/^netplay_nickname = .*/netplay_nickname = \"$new_nickname\"/" "$config_file"
			log "netplay_nickname prefix added: \"$netplay_nickname\" is now \"$new_nickname\""
		else
			log "netplay_nickname prefix already OK: \"$netplay_nickname\""
		fi
	fi

}

###########
#Utilities#
###########

build_infoPanel_and_log() {
	local title="$1"
	local message="$2"

	log "Info Panel: \n\tStage: $title\n\tMessage: $message"
	if is_running infoPanel; then
		killall -9 infoPanel
	fi
	infoPanel --title "$title" --message "$message" --persistent &
	sync
	touch /tmp/dismiss_info_panel
	sync
	sleep 0.5
	sync
}

is_running() {
	process_name="$1"
	pgrep "$process_name" >/dev/null
}

#########
##Main.##
#########

lets_go() {
	check_wifi
	Get_NetplayCore "$CurrentSystem"
	if [ $? -eq 0 ]; then
		echo "*****************************************"
		echo "romfullpath: $romfullpath"
		echo "platform: ${platform}"
		echo "netplaycore: $netplaycore"
		echo "core_config_folder: $core_config_folder"
		echo "cpuspeed: $cpuspeed"
		echo "*****************************************"
	else
		exit
	fi
	Check_PlayerName
	start_retroarch
}

lets_go
