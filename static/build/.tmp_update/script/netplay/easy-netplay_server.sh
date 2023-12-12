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
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

##########
##Setup.##
##########

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
	netplaycore_info=$(grep "^${platform};" "$sysdir/config/netplay_cores.conf")
	if [ -n "$netplaycore_info" ]; then
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
create_cookie_info() {
	COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"
	MAX_FILE_SIZE_BYTES=26214400

	echo "[core]: $netplaycore" >"$COOKIE_FILE"
	echo "[rom]: $cookie_rom_path" >>"$COOKIE_FILE"

	if [ -s "$netplaycore" ]; then
		log "Writing core size"
		core_size=$(stat -c%s "$netplaycore")
		if [ "$core_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
			echo "[corechksum]: 0" >>"$COOKIE_FILE"
		else
			echo "[corechksum]: $(xcrc "$netplaycore")" >>"$COOKIE_FILE"
		fi
	fi

	if [ -s "$cookie_rom_path" ]; then
		log "Writing rom size"
		rom_size=$(stat -c%s "$cookie_rom_path")
		if [ "$rom_size" -gt "$MAX_FILE_SIZE_BYTES" ]; then
			echo "[romchksum]: 0" >>"$COOKIE_FILE"
		else
			echo "[romchksum]: $(xcrc "$cookie_rom_path")" >>"$COOKIE_FILE"
		fi
	fi

	if [ -s "$cpuspeed" ]; then
		echo "[cpuspeed]: $cpuspeed" >>"$COOKIE_FILE"
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
		echo -n $cpuspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
	fi

	cd /mnt/SDCARD/RetroArch
	# sleep 5
	HOME=/mnt/SDCARD/RetroArch ./retroarch --appendconfig=./.retroarch/easynetplay_override.cfg -H -L "$netplaycore" "$cookie_rom_path"
	# We restore previous core CPU speed
	if [ -n "$PreviousCPUspeed" ]; then
		echo -n $PreviousCPUspeed >"/mnt/SDCARD/Saves/CurrentProfile/config/${core_config_folder}/cpuclock.txt"
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
	sleep 0.3
	sync
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
	if pgrep udhcpc >/dev/null; then
		killall -9 udhcpc
	fi
	sleep 1
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
}

is_running() {
	process_name="$1"
	pgrep "$process_name" >/dev/null
}

#########
##Main.##
#########

lets_go() {
	pressMenu2Kill $(basename $0) &
	. "$sysdir/script/network/hotspot_create.sh"
	start_ftp
	Get_NetplayCore
	create_cookie_info
	pkill -9 pressMenu2Kill
	start_retroarch
	cleanup
}

lets_go
