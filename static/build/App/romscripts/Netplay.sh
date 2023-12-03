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
	if [ -z "$romdirname" ] || [ "$romext" == "miyoocmd" ]; then
		DynamicLabel="none"
	else
		netplaycore_info=$(grep "^${romdirname};" "$sysdir/config/netplay_cores.conf")
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

echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

cores_configurator() {

	if [ "$romdirname" == "GB" ] || [ "$romdirname" == "GBC" ]; then
		tgb_dual_opts="/mnt/SDCARD/Saves/CurrentProfile/config/TGB Dual/TGB Dual.opt"
		tgb_dual_opts_tmp="/tmp/TGB Dual.patch"
		echo -e "tgbdual_single_screen_mp = \"player ${PlayerNum} only\"" > "$tgb_dual_opts_tmp"
		echo -e "tgbdual_audio_output = \"Game Boy #${PlayerNum}\"" >> "$tgb_dual_opts_tmp"
		$sysdir/script/patch_ra_cfg.sh "$tgb_dual_opts_tmp" "$tgb_dual_opts"
		rm "$tgb_dual_opts_tmp"
	fi

	if [ "$romdirname" == "MD" ]; then
		pico_opts_tmp="/tmp/MD.patch"
		echo -e "picodrive_input1 = \"6 button pad\"" > "$pico_opts_tmp"
		echo -e "picodrive_input2 = \"6 button pad\"" >> "$pico_opts_tmp"
		$sysdir/script/patch_ra_cfg.sh "$pico_opts_tmp" "/mnt/SDCARD/Saves/CurrentProfile/config/PicoDrive/PicoDrive.opt"
		rm "$pico_opts_tmp"
	fi
}

romdirname=$(echo "$1" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
if [ "$romdirname" == "GB" ] || [ "$romdirname" == "GBC" ]; then
	EasyNetplayPokemon="Easy Netplay - Pokemon Trade/Battle"
fi
# Netplay mode main script:
cd $sysdir
echo "###################################################################################################################"
echo "#################################### Netplay.sh script start.######################################################"
echo "###################################################################################################################"
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "Netplay" \
	"Host a session..." \
	"Join a session..."

retcode=$?
if [ $retcode -eq 0 ]; then

	PlayerNum=1
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "HOST - Netplay type" \
		"Standard Netplay (Use current Wifi)" \
		"Easy Netplay (play anywhere, local only)" \
		"$EasyNetplayPokemon"

	retcode=$?

	[ "$retcode" -ne 255 ] && cores_configurator

	if [ $retcode -eq 0 ]; then
		/bin/sh "$sysdir/script/netplay/standard-netplay.sh" "$1" "$2" "host"
	elif [ $retcode -eq 1 ]; then
		/bin/sh "$sysdir/script/netplay/easy-netplay_server.sh"
	elif [ $retcode -eq 2 ]; then
		/bin/sh "$sysdir/script/netplay/easy-netplay_server_pokemon.sh"
	elif [ $retcode -eq 255 ]; then
		exit
	fi
elif [ $retcode -eq 1 ]; then
	PlayerNum=2
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t "JOIN - Netplay type" \
		"Standard Netplay (Use current Wifi)" \
		"Easy Netplay (play anywhere, local only)" \
		"$EasyNetplayPokemon"

	retcode=$?
	[ "$retcode" -ne 255 ] && cores_configurator
	if [ $retcode -eq 0 ]; then
		/bin/sh "$sysdir/script/netplay/standard-netplay.sh" "$1" "$2" "join"
	elif [ $retcode -eq 1 ]; then
		/bin/sh "$sysdir/script/netplay/easy-netplay_client.sh"
	elif [ $retcode -eq 2 ]; then
		/bin/sh "$sysdir/script/netplay/easy-netplay_client_pokemon.sh"
	elif [ $retcode -eq 255 ]; then
		exit
	fi

elif [ $retcode -eq 255 ]; then
	exit
fi

if [ "$romdirname" == "GB" ] || [ "$romdirname" == "GBC" ]; then
	echo -e "tgbdual_single_screen_mp = \"player 1 only\"" > "$tgb_dual_opts_tmp"
	echo -e "tgbdual_audio_output = \"Game Boy #1\"" >> "$tgb_dual_opts_tmp"
	$sysdir/script/patch_ra_cfg.sh "$tgb_dual_opts_tmp" "$tgb_dual_opts"
	rm "$tgb_dual_opts_tmp"
fi

echo "###################################################################################################################"
echo "############################################# Netplay.sh script end. ##############################################"
echo "###################################################################################################################"

# restore wifi in case of sudden quit :
if [ -f "/tmp/old_ipv4.txt" ]; then
	"$sysdir/script/network/hotspot_cleanup.sh"
fi
