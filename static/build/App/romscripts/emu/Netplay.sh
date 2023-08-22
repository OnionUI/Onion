#!/bin/sh
# echo $0 $*

scriptlabel="DynamicLabel"
sysdir=/mnt/SDCARD/.tmp_update
require_networking=1

# DynamicLabel management:
if [ "$3" = "DynamicLabel" ]; then
	emulabel="$4"
	retroarch_core="$5"
	romdirname="$6"
	romext="$7"
	if [ -z "$romdirname" ]  || [ "$romext" == "miyoocmd" ]; then
		DynamicLabel = "none"
	else
		netplaycore_info=$(grep "^${romdirname};" "$sysdir/script/netplay/netplay_cores.cfg")
		if [ -n "$netplaycore_info" ]; then
			netplaycore=$(echo "$netplaycore_info" | cut -d ';' -f 2)
			if [ "$netplaycore" = "none" ]; then
				DynamicLabel="No Netplay for $emulabel"
			else
				netplaycore_without_suffix=$(echo "$netplaycore" | awk -F "_libretro.so" '{print $1}')
				DynamicLabel="Netplay (core supported: ${netplaycore_without_suffix})"				
			fi

		else
			DynamicLabel="Netplay (core not verified: ${retroarch_core%_libretro})"
		fi
	fi

	echo -n "$DynamicLabel" > /tmp/DynamicLabel.tmp
    exit
fi


# Netplay mode main script:
cd $sysdir

LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "Netplay" "Host" "Join"
retcode=$?
if [ $retcode -eq 0 ] ; then
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "Netplay type" "Standard Netplay (Use current Wifi)" "Easy Netplay (play anywhere, local only)"
	retcode=$?
	if [ $retcode -eq 0 ] ; then
		"./script/netplay/standard_netplay.sh" "$1" "$2" "host"
	else
		cd $sysdir
		/bin/sh "$sysdir/script/easynetplay/netplay_server.sh"
	fi

else
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "Netplay type" "Standard Netplay (Use current Wifi)" "Easy Netplay (play anywhere, local only)"
	retcode=$?
	if [ $retcode -eq 0 ] ; then
		"./script/netplay/standard_netplay.sh" "$1" "$2" "join"
	else
		cd $sysdir
		/bin/sh "$sysdir/script/easynetplay/netplay_client.sh"
	fi
	
fi
