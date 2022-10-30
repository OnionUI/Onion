#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

# migration information gpSP to mGBA

if ! [ -f "/mnt/SDCARD/Saves/CurrentProfile/saves/mGBA/.gpspImportDone" ]; then 
	mkdir /mnt/SDCARD/Saves/CurrentProfile/saves/mGBA
	
	ls /mnt/SDCARD/Saves/CurrentProfile/saves/gpSP/*.sav
	retVal=$?

	if [ $retVal -eq 0 ]; then
		/mnt/SDCARD/.tmp_update/bin/prompt -r -t "New default GBA core!"  -m \
		"- mGBA offers improved game compatibility!\n\
		- gpSP moved to expert section.                                    \n\
		Refer to the Onion Wiki.\n\
		-\nDo you want to import saves from gpSP ?" \
		"Yes" \
		"No"

		
		retcode=$?

		if [ $retcode -eq 0 ]; then
			cp /mnt/SDCARD/Saves/CurrentProfile/saves/gpSP/*.sav /mnt/SDCARD/Saves/CurrentProfile/saves/mGBA
			for file in /mnt/SDCARD/Saves/CurrentProfile/saves/mGBA/*.sav; do
				mv -n  -- "$file" "${file%.sav}.srm"
			done
			rm /mnt/SDCARD/Saves/CurrentProfile/saves/mGBA/*.sav

		fi
	else
		/mnt/SDCARD/.tmp_update/bin/infoPanel -t "New default GBA core!"  -m "- mGBA offers improved game compatibility!\n\
		- gpSP moved to expert section.                                 \n\
		Refer to the Onion Wiki."

	fi

touch "/mnt/SDCARD/Saves/CurrentProfile/saves/mGBA/.gpspImportDone"
fi


# Timer initialisation
cd /mnt/SDCARD/App/PlayActivity
./playActivity "init"

cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/mgba_libretro.so "$1"

# Timer registration
cd /mnt/SDCARD/App/PlayActivity
./playActivity "$1"