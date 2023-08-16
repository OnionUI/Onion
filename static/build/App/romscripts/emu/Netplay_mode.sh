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
		netplaycore=$(grep "^${romdirname} " "$sysdir/script/netplay/netplay_cores.cfg" | awk '{print $2}')
		if [ -n "$netplaycore" ]; then
			if [ "$netplaycore" = "none" ]; then
				DynamicLabel="No Netplay for $emulabel"
			else
				DynamicLabel="Netplay (core supported: ${netplaycore%_libretro.so})"
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
"./script/netplay/netplay_server.sh" "$1" "$2"
