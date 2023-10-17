#!/bin/sh

echo -ne "\n\n"
echo --------------------------------------------------------------------
echo ":: STANDALONE PORT LAUNCH"
echo --------------------------------------------------------------------

GameName="$1"
GameDir="/mnt/SDCARD/Roms/PORTS/Games/$2"
GameExecutable="$3"
Arguments="$4"
GameDataFile="$5"
KillAudioserver="$6"
PerformanceMode="$7"


if [ -z "$GameDataFile" ]; then
	echo " No GameDataFile specified, taking executable instead for presence checker."
	GameDataFile="$GameExecutable"
fi

echo GameName ---------- : $GameName
echo GameDir ----------- : $GameDir
echo GameExecutable ---- : $GameExecutable
echo Arguments --------- : $Arguments
echo GameDataFile ------ : $GameDataFile
echo KillAudioserver --- : $KillAudioserver
echo PerformanceMode --- : $PerformanceMode

echo --------------------------------------------------------------------

echo Command line ------ : find "$GameDir" -maxdepth 2 -type f -iname "$GameDataFile" \| grep .
find "$GameDir" -maxdepth 2 -type f -iname "$GameDataFile" | grep .
if [ ! $? -eq 0 ]; then
	echo "$GameDataFile is missing in Roms/PORTS/Games/$2."
	echo "$GameDataFile is missing in\nRoms/PORTS/Games/$2.">/tmp/MissingPortFile.tmp
	exit
fi


echo --------------------------------------------------------------------
echo ":: APPLYING ADDITIONNAL CONFIGURATION"
echo --------------------------------------------------------------------

if [ "$KillAudioserver" = "1" ]; then . /mnt/SDCARD/.tmp_update/script/stop_audioserver.sh; fi
if [ "$PerformanceMode" = "1" ]; then echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor; fi

ParasytePath="/mnt/SDCARD/.tmp_update/lib/parasyte"

if [ -d "$GameDir/lib" ]; then
	echo "exporting \"$GameDir/lib\" to LD_LIBRARY_PATH"
	export LD_LIBRARY_PATH="$GameDir/lib:$ParasytePath:$LD_LIBRARY_PATH:$GameDir/lib"
fi


if [ -d "$GameDir/libs" ]; then
	echo "exporting \"$GameDir/libs\" to LD_LIBRARY_PATH"
	export LD_LIBRARY_PATH="$GameDir/libs:$ParasytePath:$LD_LIBRARY_PATH"
fi


echo --------------------------------------------------------------------
echo ":: RUNNING THE PORT"
echo --------------------------------------------------------------------

cd "$GameDir"
HOME="$GameDir"

echo running "$GameName" ...


eval echo -ne "Command line : \\\n\"$GameDir/$GameExecutable\" $Arguments \\\n\\\n\\\n"
eval  \"$GameDir/$GameExecutable\" $Arguments


echo --------------------------------------------------------------------
echo ":: POST RUNNING TASKS"
echo --------------------------------------------------------------------

unset LD_PRELOAD

echo -ne "\n\n"
