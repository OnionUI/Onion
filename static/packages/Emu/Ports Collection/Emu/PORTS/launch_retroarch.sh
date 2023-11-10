#!/bin/sh

echo -ne "\n\n"
echo --------------------------------------------------------------------
echo ":: RETROARCH PORT LAUNCH"
echo --------------------------------------------------------------------

GameName="$1"
Core="$2"
RomDir="/mnt/SDCARD/Roms/PORTS/Games/$3"
RomFile="$4"
Arguments="$5"
KillAudioserver="$6"
PerformanceMode="$7"


if [ -z "$RomFile" ]; then
	echo " No RomFile specified."
else
	find "$RomDir" -maxdepth 1 -type f -iname "$RomFile" | grep .
	if [ ! $? -eq 0 ]; then
		echo "$GameDataFile is missing in\nRoms/PORTS/Games/$2.">/tmp/MissingPortFile.tmp
		exit
	fi
	RomFullPath="$RomDir/$RomFile"
fi

echo GameName ---------- : $GameName
echo Core -------------- : $Core
echo RomFullPath ------- : $RomFullPath
echo Arguments --------- : $Arguments
echo KillAudioserver --- : $KillAudioserver
echo PerformanceMode --- : $PerformanceMode




echo --------------------------------------------------------------------
echo ":: APPLYING ADDITIONNAL CONFIGURATION"
echo --------------------------------------------------------------------

if [ "$KillAudioserver" = "1" ]; then . /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh; fi
if [ "$PerformanceMode" = "1" ]; then echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor; fi

romcfgpath="$(dirname "$RomFullPath")/.game_config/$(basename "$RomFullPath").name"
mkdir -p "$(dirname "$RomFullPath")/.game_config"
echo "$GameName" > "$romcfgpath"

echo --------------------------------------------------------------------
echo ":: RUNNING THE PORT"
echo --------------------------------------------------------------------

cd /mnt/SDCARD/RetroArch/


echo running "$GameName" ...


eval echo -ne "Command line : \\\nHOME=/mnt/SDCARD/RetroArch/ ./retroarch -v -L .retroarch/cores/${Core}_libretro.so \"$RomFullPath\" $Arguments \\\n\\\n\\\n"
eval  HOME=/mnt/SDCARD/RetroArch/ ./retroarch -v -L .retroarch/cores/${Core}_libretro.so \"$RomFullPath\" $Arguments


echo --------------------------------------------------------------------
echo ":: POST RUNNING TASKS"
echo --------------------------------------------------------------------

unset LD_PRELOAD

echo -ne "\n\n"
