#!/bin/sh

echo -ne "\n\n"
echo --------------------------------------------------------------------
echo ":: PYTHON PORT LAUNCH"
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

echo Command line ------ : find "$GameDir" -maxdepth 1 -type f -iname "$GameDataFile" \| grep .
find "$GameDir" -maxdepth 1 -type f -iname "$GameDataFile" | grep .
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
export PYTHONPATH=$ParasytePath/python2.7:$ParasytePath/python2.7/site-packages:$ParasytePath/python2.7/lib-dynload
export PYTHONHOME=$ParasytePath/python2.7:$ParasytePath/python2.7/site-packages:$ParasytePath/python2.7/lib-dynload
export LD_LIBRARY_PATH=$ParasytePath:$ParasytePath/python2.7/:$ParasytePath/python2.7/lib-dynload:$LD_LIBRARY_PATH


echo --------------------------------------------------------------------
echo ":: RUNNING THE PORT"
echo --------------------------------------------------------------------

cd "$GameDir"
HOME="$GameDir"

echo running "$GameName" ...

eval echo -ne "Command line : \\\n\"$ParasytePath/python2\" \"$GameExecutable\" $Arguments \\\n\\\n\\\n"
eval /mnt/SDCARD/.tmp_update/bin/parasyte/python2 \"$GameExecutable\" $Arguments


echo --------------------------------------------------------------------
echo ":: POST RUNNING TASKS"
echo --------------------------------------------------------------------

unset LD_PRELOAD

echo -ne "\n\n" 
