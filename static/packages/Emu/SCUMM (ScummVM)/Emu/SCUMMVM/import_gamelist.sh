#!/bin/sh
romdir=/mnt/SDCARD/Roms/SCUMMVM
scandir=/mnt/SDCARD/Roms/SCUMMVM/Shortcuts

ScummvmCfgPath=/mnt/SDCARD/BIOS/scummvm.ini
count=0


echo "Running scummVM Scan ============================================================="
infoPanel -t "ScummVM import" -m "Please wait..." --persistent &

scummvm -c "$ScummvmCfgPath" -p "$romdir" --add --recursive


# removing all the old shortcuts
rm $scandir/*.target
# ...and create the default shortcut to run import again
touch "$scandir/~Import games.target"


# here we get all the targets names
cat $ScummvmCfgPath | sed -n 's/^[ \t]*\[\(.*\)\].*/\1/p' | (
	while read target ; do
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
		echo creating file "$scandir/$FullName.target" with value ${target}
		echo ---- 
		
		echo ${target} > "$scandir/$FullName.target"
		
		let count++;
	done

	sleep 1
	touch /tmp/dismiss_info_panel

	if [ "$count" -eq 0 ] 
	then
		infoPanel -t "ScummVM Script" -m "Import done.\n\nNo games detected." --auto
	else
		infoPanel -t "ScummVM Script" -m "Done.\n\n$count game(s) detected." --auto
	fi

	sed -i "/\"pageend\":/s/:.*,/:   6,/" "/tmp/state.json"   # Little trick which allows to displays all the new items in the game list of MainUI
	rm "$scandir/Shortcuts_cache2.db"
)
