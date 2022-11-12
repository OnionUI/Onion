#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
romdir=/mnt/SDCARD/Roms/SCUMMVM
scandir=/mnt/SDCARD/Roms/SCUMMVM/Shortcuts
scummdir=/mnt/SDCARD/BIOS/scummvm

ScummvmCfgPath=/mnt/SDCARD/BIOS/scummvm.ini
i=0


echo "Running scummVM Scan ============================================================="
$sysdir/bin/infoPanel -t "ScummVM import" -m "Please wait..." --persistent &

cp $ScummvmCfgPath $scummdir/standalone/.config/scummvm/scummvm.ini
export HOME=$scummdir/standalone
$scummdir/standalone/scummvm  -p "$romdir" --add --recursive
cp $scummdir/standalone/.config/scummvm/scummvm.ini $ScummvmCfgPath


# removing all the old shortcuts
rm $scandir/*.target
# ...and create the default shortcut to run import again
touch "$scandir/◦ Import games in ScummVM.target"


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
		
		let i++;
	done

	sleep 1
	touch /tmp/dismiss_info_panel

	if [ "$i" -eq 0 ] 
	then
		$sysdir/bin/infoPanel -t "ScummVM Script" -m "Import done.\n\nNo games detected." --auto
	else
		$sysdir/bin/infoPanel -t "ScummVM Script" -m "Done.\n\n$i game(s) detected." --auto
	fi

	sed -i "/\"pageend\":/s/:.*,/:   6,/" "/tmp/state.json"   # Little trick which allows to displays all the new items in the game list of MainUI
	rm "$scandir/Shortcuts_cache2.db"
)
