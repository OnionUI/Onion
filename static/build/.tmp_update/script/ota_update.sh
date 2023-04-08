#!/bin/sh
# OTA updates for Onion.
sysdir=/mnt/SDCARD/.tmp_update

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
	Release_assets_info=$(curl -k -s https://api.github.com/repos/OnionUI/Onion/releases | jq -r 'map(select(.prerelease)) | first')
else
	Release_assets_info=$(curl -k -s https://api.github.com/repos/$GITHUB_REPOSITORY/releases/latest)
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


Release_asset=`echo "$Release_assets_info" | jq '.assets[]? | select(.name | contains("Onion-v"))'`

Release_url=$(echo $Release_asset | jq '.browser_download_url' | tr -d '"')
Release_FullVersion=$(echo $Release_asset | jq '.name' | tr -d "\"" | sed 's/^Onion-v//g' | sed 's/\.zip$//g')
Release_Version=$(echo $Release_FullVersion | sed 's/-dev.*$//g')
Release_size=$(echo $Release_asset | jq -r '.size')
Release_info=$(echo $Release_assets_info | jq '.body')

Current_FullVersion=$(installUI --version)
Current_Version=$(echo $Current_FullVersion | sed 's/-dev.*$//g')

#debug tests
#echo All: $Release_assets_info 
#echo Info : $Release_info
#GetVersion "$Release_Version"
#GetVersion "$Current_Version"
#Current_Version=1.1.0

echo -ne "\\n\\n======== Current Version ========= \\n Version : $Current_Version \\n==================================\\n"

echo -ne "\\n\\n======== Online Version  ========= \\n Version : $Release_Version  (Channel : $channel)\\n Size : $Release_size \\n URL : $Release_url \\n==================================\\n\\n\\n"

v1=$(GetVersion $Current_Version)
v2=$(GetVersion $Release_Version)

if [ $v1 -gt $v2 ] || ( [ $v1 -eq $v2 ] && [ "$Current_FullVersion" == "$Release_FullVersion" ] ); then
    echo "Version is up to date"
	read -n 1 -s -r -p "Press any key to continue"
	exit 3
fi

read -n 1 -s -r -p "Press any key to continue"

Mychoice=$( echo -e "No\nYes" | $sysdir/script/shellect.sh -t "Download $Release_Version ($((($Release_size/1024)/1024))MB) ?" -b "Press A to validate your choice.")
clear
 if [ "$Mychoice" = "Yes" ]; then
    mkdir -p $sysdir/download/
    echo -ne "\\n\\n== Downloading Onion $Release_Version ($channel channel) ==\\n" 
    wget --no-check-certificate $Release_url -O "$sysdir/download/$Release_Version.zip"
    echo -ne "\\n\\n=================== Download done =================== \\n" 
    
 else
    echo -e "Exiting.\n"
	read -n 1 -s -r -p "Press any key to continue"
    exit 4
 fi


Downloaded_size=$(stat -c %s "$sysdir/download/$Release_Version.zip")
if [ "$Downloaded_size" -eq "$Release_size" ] ; then
	echo "File size OK! ($Downloaded_size)"
	sleep 3
else 
	echo -ne "\\n\\nError : Wrong download size :\\n \"$Downloaded_size\" instead of \"$Release_size\"\\n"
	read -n 1 -s -r -p "Press any key to continue"
	exit 5
fi


Mychoice=$( echo -e "No\nYes" | $sysdir/script/shellect.sh -t "Apply update $Release_Version ?" -b "Press A to validate your choice.")
clear
 if [ "$Mychoice" = "Yes" ]; then
	echo "Applying update"
	unzip -o "$sysdir/download/$Release_Version.zip" -d "/mnt/SDCARD"

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

