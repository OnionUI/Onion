#!/bin/sh
# OTA updates for Onion.

# Repository name :
GITHUB_REPOSITORY=OnionUI/Onion

# channel : stable or beta
channel=beta

GetVersion () { echo $@ | tr -d [:alpha:] | awk -F. '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }'; }

# Available space in MB
available_space=$(df -m /dev/mmcblk0p1 | awk 'NR==2{print $4}')

# Check available space
if [ "$available_space" -lt "1000" ]; then
	echo "Available space is insufficient on SD card"
	read -n 1 -s -r -p "Press any key to continue"
	exit 1
fi

# Enable wifi if necessary
IP=$(ip route get 1 | awk '{print $NF;exit}')
if [ "$IP" = "" ]; then
	echo "Wifi disabled, Trying to enable it..."
	insmod /mnt/SDCARD/8188fu.ko
	ifconfig lo up
	/customer/app/axp_test wifion
	sleep 2
	ifconfig wlan0 up
	wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	udhcpc -i wlan0 -s /etc/init.d/udhcpc.script
	sleep 3
fi

# Github source api url
if [ "$channel" = "beta" ]; then
	Release_assets_info=$(/mnt/SDCARD/.tmp_update/bin/curl -k -s https://api.github.com/repos/OnionUI/Onion/releases | /mnt/SDCARD/.tmp_update/bin/jq -r 'map(select(.prerelease)) | first')
else
	Release_assets_info=$(/mnt/SDCARD/.tmp_update/bin/curl -k -s https://api.github.com/repos/$GITHUB_REPOSITORY/releases/latest)
fi

echo checking internet connection...
if ping -4 -c 1 github.com &> /dev/null
then
    echo "Internet check successful"
else
    echo "Error : Offline. Check your wifi connection."
	read -n 1 -s -r -p "Press any key to continue"
	exit 2
fi


Release_url=$(echo $Release_assets_info | /mnt/SDCARD/.tmp_update/bin/jq '.assets[].browser_download_url' | tr -d '"')
Release_Version=$(echo $Release_assets_info | /mnt/SDCARD/.tmp_update/bin/jq '.name'| tr -d "'\"")
Release_size=$(echo $Release_assets_info | /mnt/SDCARD/.tmp_update/bin/jq -r '.assets[]? | select(.name | contains("Onion-v")) | .size')
Release_info=$(echo $Release_assets_info | /mnt/SDCARD/.tmp_update/bin/jq '.body')

Current_Version=$(/mnt/SDCARD/.tmp_update/bin/installUI --version)


#debug tests
#echo All: $Release_assets_info 
#echo Info : $Release_info
#GetVersion "$Release_Version"
#GetVersion "$Current_Version"
#Current_Version=1.1.0

echo -ne "\\n\\n======== Current Version ========= \\n Version : $Current_Version \\n==================================\\n"

echo -ne "\\n\\n======== Online Version  ========= \\n Version : $Release_Version  (Channel : $channel)\\n Size : $Release_size \\n URL : $Release_url \\n==================================\\n\\n\\n"



if [ $(GetVersion $Current_Version) -ge $(GetVersion $Release_Version) ]; then
    echo "Version is up to date"
	read -n 1 -s -r -p "Press any key to continue"
	exit 3
fi

read -n 1 -s -r -p "Press any key to continue"

Mychoice=$( echo -e "No\nYes" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "Download $Release_Version ($((($Release_size/1024)/1024))MB) ?" -b "Press A to validate your choice.")
clear
 if [ "$Mychoice" = "Yes" ]; then
    mkdir -p /mnt/SDCARD/.tmp_update/download/
    echo -ne "\\n\\n== Downloading Onion $Release_Version ($channel channel) ==\\n" 
    /mnt/SDCARD/.tmp_update/bin/wget --no-check-certificate $Release_url -O "/mnt/SDCARD/.tmp_update/download/$Release_Version.zip"
    echo -ne "\\n\\n=================== Download done =================== \\n" 
    
 else
    echo -e "Exiting.\n"
	read -n 1 -s -r -p "Press any key to continue"
    exit 4
 fi


Downloaded_size=$(stat -c %s "/mnt/SDCARD/.tmp_update/download/$Release_Version.zip")
if [ "$Downloaded_size" -eq "$Release_size" ] ; then
	echo "File size OK! ($Downloaded_size)"
	sleep 3
else 
	echo -ne "\\n\\nError : Wrong download size :\\n \"$Downloaded_size\" instead of \"$Release_size\"\\n"
	read -n 1 -s -r -p "Press any key to continue"
	exit 5
fi


Mychoice=$( echo -e "No\nYes" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "Apply update $Release_Version ?" -b "Press A to validate your choice.")
clear
 if [ "$Mychoice" = "Yes" ]; then
	echo "Applying update"
	unzip -o "/mnt/SDCARD/.tmp_update/download/$Release_Version.zip" -d "/mnt/SDCARD"

	if [ $? -eq 0 ]; then
		echo "Decompression successful."
		sleep 3
		echo -ne "\\n\\nUpdate $Release_Version applied.\\nRebooting to run installation !\\n"
		read -n 1 -s -r -p "Press any key to continue"
		sleep 1
		reboot
	else
		echo -ne "\\n\\nError : Something wrong happens during decompression.\nTry to run OTA update again or make manual update.\n Exiting."
		read -n 1 -s -r -p "Press any key to continue"
		exit 6
	fi
 else
	  echo -e "\\nYou have selected to not apply the update, see you next time !\\n Exiting.\\n"
	  read -n 1 -s -r -p "Press any key to continue"
	  exit 7
 fi

