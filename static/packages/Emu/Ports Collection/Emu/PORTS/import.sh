#!/bin/sh
# Ports import script
scandir=/mnt/SDCARD/Emu/PORTS/../../Roms/PORTS/Shortcuts
count_found=0
count_notfound=0

infoPanel -t "Ports Import" -m "Scanning..." --persistent &

rm $scandir/../import.log 2> /dev/null

printf " STATUS   %-50s  %-40s  %-20s  %s\n\n" \
		"PORT NAME" \
		"CATEGORY" \
		"CHECKED FILE" \
		"GAMES DIRECTORY" \
		>> $scandir/../import.log

OIFS="$IFS"
IFS=$'\n'

for file_path in $(find "$scandir" -type f  \( -name "*.port" -o -name "*.notfound" \))
do
	ext=`echo "$(basename "$file_path")" | awk -F. '{print tolower($NF)}'`

	# Remove old .port file if equivalent .notfound exists (allows for updates)
	if [ "$ext" == "port" ] && [ -f "${file_path%.*}.notfound" ]; then
		rm "$file_path" 2> /dev/null
		continue
	fi
	if [ "$ext" == "notfound" ] && [ -f "${file_path%.*}.port" ]; then
		rm "${file_path%.*}.port" 2> /dev/null
	fi

	echo "======================================================================================"

# ==================================== GAME PATH ====================================

	# classic case : the "GameDir" has been specified.
	GameDir=$(grep "GameDir=" "$file_path" | cut -d "=" -f2 | grep -o '".*"' | tr -d '"')
	# If the "GameDir" has not been specified we take the RomDir name instead
	if [ -z "$GameDir" ]; then
		GameDir=$(grep "RomDir=" "$file_path" | cut -d "=" -f2  | grep -o '".*"' | tr -d '"')
	fi
	# If the "RomDir" has not been specified, it's probably a retroarch core without rom so we take the name of the core 
	if [ -z "$GameDir" ]; then
		GameDir="/mnt/SDCARD/RetroArch/.retroarch/cores"
	else
		GameDir="/mnt/SDCARD/Roms/PORTS/Games/${GameDir}"
	fi

# ================================= GAME FILE NAME =================================

	# classic case : the "GameDataFile" has been specified.
	GameDataFile=$(grep "GameDataFile=" "$file_path" | cut -d "=" -f2  | grep -o '".*"' | tr -d '"')
	# If the "GameDataFile" has not been specified we take the game executable name instead
	if [ -z "$GameDataFile" ]; then
		GameDataFile=$(grep "GameExecutable=" "$file_path" | cut -d "=" -f2  | grep -o '".*"' | tr -d '"')
	fi
	# If the "GameExecutable" has not been specified we take the rom name instead
	if [ -z "$GameDataFile" ]; then
		GameDataFile=$(grep "RomFile=" "$file_path" | cut -d "=" -f2  | grep -o '".*"' | tr -d '"')
	fi
	# If the "RomFile" has not been specified, it's probably a retroarch core withtout rom so we take the name of the core 
	if [ -z "$GameDataFile" ]; then
		GameDataFile=$(grep "Core=" "$file_path" | cut -d "=" -f2  | grep -o '".*"' | tr -d '"')
		GameDataFile=${GameDataFile}_libretro.so 
	fi
# ==================================== RESULT ====================================
	
	echo "Current file ---- : $file_path"
	echo "GameDir --------- : $GameDir"
	echo "GameDataFile ---- : $GameDataFile"
  
	  
# =============================== GAME PRESENCE CHECK ===============================
	
	find "${GameDir}" -maxdepth 2 -type f -iname "${GameDataFile}" | grep .
	not_found=$?

	if [ $not_found -gt 0 ]; then
		echo "Presence -------- :  NOT FOUND !"
		if [ "$ext" != "notfound" ]; then
			# rename .port -> .notfound
			mv "$file_path" "${file_path%.*}.notfound"
		fi
		let count_notfound++
	else
		echo "Presence -------- :  OK"
		if [ "$ext" != "port" ]; then
			# rename .notfound -> .port
			mv "$file_path" "${file_path%.*}.port"
		fi
		let count_found++
	fi

	printf "[ %s ]  %-50s  %-40s  %-20s  %s\n" \
		"$([ $not_found -gt 0 ] && (echo "FAIL") || (echo " OK "))" \
		"$(basename "${file_path%.*}")" \
		"$(dirname "$file_path" | sed 's/^.*\/Roms\/PORTS\/Shortcuts\///g')" \
		"$GameDataFile" \
		"$(echo "$GameDir" | sed 's/^\/mnt\/SDCARD\/Roms\/PORTS\/Games\///g' | sed 's/\/mnt\/SDCARD\/RetroArch\/.retroarch\/cores//g')" \
		>> $scandir/../import.log
done
IFS="$OIFS"

touch /tmp/dismiss_info_panel
sync

ports_total=$((count_found + count_notfound))
infoPanel -t "Ports Import" -m "Found $count_found of $ports_total $([ $ports_total -eq 1 ] && (echo "port") || (echo "ports"))" --auto

/mnt/SDCARD/.tmp_update/script/reset_list.sh "$scandir"
