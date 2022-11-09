#!/bin/sh

ScummvmCfgPath=/mnt/SDCARD/BIOS/scummvm.ini
i=0


echo "Running scummVM Scan ============================================================="
/mnt/SDCARD/.tmp_update/bin/infoPanel -t "ScummVM Script" -m "Please wait..." &

cp $ScummvmCfgPath ./standalone/.config/scummvm/scummvm.ini
export HOME=./standalone
./standalone/scummvm  -p "/mnt/SDCARD/Roms/SCUMMVM/Games" --add --recursive
cp ./standalone/.config/scummvm/scummvm.ini $ScummvmCfgPath


# removing all the old shortcuts
rm /mnt/SDCARD/Roms/SCUMMVM/Shortcuts/*.target
# ...and create the default shortcut to run import again
touch "/mnt/SDCARD/Roms/SCUMMVM/Shortcuts/◦ Import games in ScummVM.target"


# here we get all the targets names
cat $ScummvmCfgPath | sed -n 's/^[ \t]*\[\(.*\)\].*/\1/p' | (while read target ;

do

	# We skip the first Scummvm section which is not a game
	if [ "$target" = "scummvm" ]; then continue; fi

	# get the full name of the game (we also remove special characters)  :
	FullName=`cat $ScummvmCfgPath | sed -n "/^[ \t]*\["$target"]/,/\[/s/^[ \t]*description[ \t]*=[ \t]*//p"  | sed -e 's/: / - /g' | tr -cd "A-Z a-z0-9()._'-"`

	# get the current path of the game :
	Path=`cat $ScummvmCfgPath | sed -n "/^[ \t]*\["$target"]/,/\[/s/^[ \t]*path[ \t]*=[ \t]*//p" `


	echo ---- 
	echo full name : $FullName
	echo target : ${target}
	echo path : $Path
	echo creating file "/mnt/SDCARD/Roms/SCUMMVM/Shortcuts/$FullName.target" with value ${target}
	echo ---- 
	
	echo ${target} > "/mnt/SDCARD/Roms/SCUMMVM/Shortcuts/$FullName.target"
	
	let i++;

done


sleep 1
killall infoPanel


if [ "$i" -eq 0 ] 
then
	/mnt/SDCARD/.tmp_update/bin/infoPanel -t "ScummVM Script" -m "Import done.\n\nNo games detected."
else
	/mnt/SDCARD/.tmp_update/bin/infoPanel -t "ScummVM Script" -m "Done.\n\n$i game(s) detected."
fi

	sed -i "/\"pageend\":/s/:.*,/:   6,/" "/tmp/state.json"   # Little trick which allows to displays all the new items in the game list of MainUI
	rm "/mnt/SDCARD/Roms/SCUMMVM/Shortcuts/Shortcuts_cache2.db"


)


