#!/bin/sh
# OTA updates for Onion.
cmd=$1
sysdir=/mnt/SDCARD/.tmp_update

# Colors
RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
NC='\033[0m' # No Color

# Repository name :
GITHUB_REPOSITORY=OnionUI/Onion

# channel : stable or beta
channel=$(cat "$sysdir/config/ota_channel" 2> /dev/null)
if [ "$channel" == "" ]; then
	channel="stable"
fi

main() {
	if [ "$cmd" == "check" ]; then
		IP=$(ip route get 1 | awk '{print $NF;exit}')
		if [ "$IP" != "" ]; then
			get_release_info
			if [ $? -eq 0 ]; then
				touch "$sysdir/.updateAvailable"
				exit 0
			fi
		fi
		exit 1
	fi

	rm $sysdir/cmd_to_run.sh 2> /dev/null

	check_available_space
	enable_wifi
	check_connection
	run_bootstrap

	sleep 2
	channel_choice

	get_release_info
	if [ $? -eq 1 ]; then
		echo -ne "${YELLOW}"
		read -n 1 -s -r -p "Press A to exit"
		exit 3
	else
		touch "$sysdir/.updateAvailable"
	fi

	download_update
	apply_update
}

check_available_space() {
	# Available space in MB
	mount_point=$(mount | grep -m 1 '/mnt/SDCARD' | awk '{print $1}') # it could be /dev/mmcblk0p1 or /dev/mmcblk0
	available_space=$(df -m $mount_point | awk 'NR==2{print $4}')

	# Check available space
	if [ "$available_space" -lt "1000" ]; then
		echo -e "${RED}Available space is insufficient on SD card${NC}\n"
		echo -ne "${YELLOW}"
		read -n 1 -s -r -p "Press A to exit"
		exit 1
	fi
}

enable_wifi() {
	# Enable wifi if necessary
	IP=$(ip route get 1 | awk '{print $NF;exit}')
	if [ "$IP" = "" ]; then
		echo "Wifi is disabled - trying to enable it..."
		insmod /mnt/SDCARD/8188fu.ko
		ifconfig lo up
		/customer/app/axp_test wifion
		sleep 2
		ifconfig wlan0 up
		wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
		udhcpc -i wlan0 -s /etc/init.d/udhcpc.script
		sleep 3
		clear
	fi
}

check_connection() {
	echo -n "Checking internet connection... "
	if wget -q --spider https://github.com > /dev/null; then
		echo -e "${GREEN}OK${NC}"
	else
		echo -e "${RED}FAIL${NC}\nError: https://github.com not reachable. Check your wifi connection."
		echo -ne "${YELLOW}"
		read -n 1 -s -r -p "Press A to exit"
		exit 2
	fi
}

run_bootstrap() {
	curl -k -s https://raw.githubusercontent.com/OnionUI/Onion/main/static/build/.tmp_update/script/ota_bootstrap.sh | sh
}

channel_choice() {
	channel=$(echo -e "stable\nbeta" | $sysdir/script/shellect.sh -t "Select distribution channel:" -b "Press A to validate your choice.")
	clear
	echo "$channel" > "$sysdir/config/ota_channel"
}

get_release_info() {
	echo -n "Retrieving release information... "

	# Github source api url
	if [ "$channel" = "beta" ]; then
		Release_assets_info=$(curl -k -s https://api.github.com/repos/$GITHUB_REPOSITORY/releases/tags/latest)
	else
		Release_assets_info=$(curl -k -s https://api.github.com/repos/$GITHUB_REPOSITORY/releases/latest)
	fi

	if echo "$Release_assets_info" | grep -q '"message": "Not Found"'; then
		echo -e "${GREEN}DONE${NC}\n\n" \
			"No update available for $channel channel\n"
		return 1
	fi

	Release_asset=$(echo "$Release_assets_info" | jq '.assets[]? | select(.name | contains("Onion-v"))')

	Release_url=$(echo $Release_asset | jq '.browser_download_url' | tr -d '"')
	Release_FullVersion=$(echo $Release_asset | jq '.name' | tr -d "\"" | sed 's/^Onion-v//g' | sed 's/\.zip$//g')
	Release_Version=$(echo $Release_FullVersion | sed 's/-.*$//g')
	Release_size=$(echo $Release_asset | jq -r '.size')
	Release_size_MB=$(echo "$(($Release_size / 1024 / 1024))MB")
	Release_Date=$(echo $Release_asset | jq -r '.created_at')
	Release_info=$(echo $Release_assets_info | jq '.body')

	Current_FullVersion=$(installUI --version)
	Current_Version=$(echo $Current_FullVersion | sed 's/-.*$//g')

	echo -e "${GREEN}DONE${NC}"

	echo -ne "\n\n" \
		"${BLUE}======= Installed Version ========${NC}\n" \
		" Version: $Current_FullVersion \n" \
		"${BLUE}==================================${NC}\n"
	echo -ne "\n\n" \
		"${BLUE}======== Online Version  =========${NC}\n" \
		" Version: $Release_FullVersion \n" \
		" Channel: $channel \n" \
		" Size:    $Release_size_MB \n" \
		" Date:    $Release_Date \n" \
		" URL:     $Release_url \n" \
		"${BLUE}==================================${NC}\n\n\n"

	v1=$(get_version $Current_Version)
	v2=$(get_version $Release_Version)

	if [ $v1 -gt $v2 ] || ([ $v1 -eq $v2 ] && [ "$Current_FullVersion" = "$Release_FullVersion" ]); then
		echo -e "Version is up to date\n"
		return 1
	fi

	echo -e "${GREEN}Update available!${NC}\n"
	return 0
}

download_update() {
	echo -ne "${YELLOW}"
	read -n 1 -s -r -p "Press A to continue"
	echo -ne "${NC}"

	Mychoice=$(echo -e "No\nYes" | $sysdir/script/shellect.sh -t "Download $Release_Version ($Release_size_MB) ?" -b "Press A to validate your choice.")
	clear
	if [ "$Mychoice" = "Yes" ]; then

		echo -ne "\n${BLUE}================== CHECKDISK ==================${NC}\n"
		/mnt/SDCARD/.tmp_update/script/stop_audioserver.sh > nul 2> nul # we need a maximum of memory available to run fsck.fat
		/mnt/SDCARD/.tmp_update/bin/freemma > NUL
		echo -ne "\n" \
			"Please wait during FAT file system integrity check.\n" \
			"Issues should be fixed automatically.\n" \
			"The process can be long:\n" \
			"about 2 minutes for 128GB SD card\n\n\n"
		fsck.fat -a $mount_point

		mkdir -p $sysdir/download/
		echo -ne "\n\n" \
			"${BLUE}== Downloading Onion $Release_Version ($channel channel) ==${NC}\n"
		/mnt/SDCARD/.tmp_update/bin/freemma > NUL
		sync
		wget --no-check-certificate $Release_url -O "$sysdir/download/$Release_Version.zip"
		echo -ne "\n\n" \
			"${GREEN}================== Download done ==================${NC}\n"
		sync
		sleep 2
	else
		exit 4
	fi

	Downloaded_size=$(stat -c %s "$sysdir/download/$Release_Version.zip")
	if [ "$Downloaded_size" -eq "$Release_size" ]; then
		echo -e "${GREEN}File size OK!${NC} ($Downloaded_size)"
		sleep 3
	else
		echo -ne "\n\n" \
			"${RED}Error: Wrong download size${NC} ($Downloaded_size instead of $Release_size)\n"
		echo -ne "${YELLOW}"
		read -n 1 -s -r -p "Press A to exit"
		exit 5
	fi
}

apply_update() {
	Mychoice=$(echo -e "No\nYes" | $sysdir/script/shellect.sh -t "Apply update $Release_Version ?" -b "Press A to validate your choice.")
	clear
	if [ "$Mychoice" = "Yes" ]; then
		echo "Applying update... "

		umount /mnt/SDCARD/miyoo/app/MainUI 2> /dev/null
		/mnt/SDCARD/.tmp_update/bin/freemma > NUL

		# unzip -o "$sysdir/download/$Release_Version.zip" -d "/mnt/SDCARD"
		7z x -aoa -o"/mnt/SDCARD" "$sysdir/download/$Release_Version.zip"

		if [ $? -eq 0 ]; then
			echo -e "${GREEN}Decompression successful.${NC}"
			sync
			sleep 3
			echo -ne "\n\n" \
				"Update $Release_Version applied.\n" \
				"Rebooting to run installation...\n"
			echo -ne "${YELLOW}"
			read -n 1 -s -r -p "Press A to reboot"
			sleep 1
			reboot
		else
			echo -ne "\n\n" \
				"${RED}Error: Something wrong happened during decompression.${NC}\n" \
				"Try to run OTA update again or do a manual update.\n"
			echo -ne "${YELLOW}"
			read -n 1 -s -r -p "Press A to exit"
			exit 6
		fi
	else
		exit 7
	fi
}

get_version() { echo "$@" | tr -d [:alpha:] | awk -F'[.-]' '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }'; }

main
