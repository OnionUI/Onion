# GLO HOST
# Script to:
# 	Start hotspot,
# 	Create a cookie file containing details for the client,
# 	Start FTP to be able to host this file,
# 	Start RA as a netplay host with -H, the core path and the rom path.
# Used within GLO as an addon script.

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

logfile=easy_netplay

# Source scripts
. $sysdir/script/log.sh
# easy-netplay_common.sh: build_infoPanel_and_log, checksize_func, checksum_func, enable_flag, disable_flag, flag_enabled, is_running, restore_ftp, udhcpc_control, url_encode, check_wifi, start_ftp
. $sysdir/script/netplay/easy-netplay_common.sh

program=$(basename "$0" .sh)

##########
##Setup.##
##########

# Find the recommended core for the current system.
Get_NetplayCore() {

	platform=$(echo "$cookie_rom_path" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
	netplaycore_info=$(grep "^${platform};" "$sysdir/config/netplay_cores.conf")
	if [ -n "$netplaycore_info" ]; then

# Runtime vars
		netplaycore=$(echo "$netplaycore_info" | cut -d ';' -f 2)
		netplaycore="/mnt/SDCARD/RetroArch/.retroarch/cores/$netplaycore"
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

# Create a cookie with all the required info for the client. (client will use this cookie)
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

    # Cleanup $cookie_rom_path if it contains "/mnt/SDCARD/Emu/XX(ANYTHINGHERE)/launch.sh:"
    cookie_rom_path=$(echo "$cookie_rom_path" | sed 's|/mnt/SDCARD/Emu/.*/launch.sh:||')

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

	. "$sysdir/script/network/hotspot_cleanup.sh"

	restore_ftp

	# Remove some files we prepared and received
	log "Removing stale files"
	rm "/mnt/SDCARD/RetroArch/retroarch.cookie"
	rm "/tmp/dismiss_info_panel"
	sync
	log "Cleanup done"
	exit

}

#########
##Main.##
#########

lets_go() {
	# Allow user to abort via menu while setup runs
	pressMenu2Kill $(basename $0) &

	# Create hotspot for client
	. "$sysdir/script/network/hotspot_create.sh"

	# start_ftp: start FTP without preflight
	start_ftp

	# Determine netplay core based on ROM/core config
	Get_NetplayCore

	# Write cookie with core/rom metadata
	create_cookie_info

	# Stop menu watcher before launch
	pkill -9 pressMenu2Kill

	# Launch RetroArch host session
	start_retroarch

	# Cleanup and restore state
	cleanup
}

lets_go
