#!/bin/sh
home_dir="$1"
current_dir="$home_dir"
selected_file=""

exit=0

while [ $exit -eq 0 ]; do

	file_list=`ls -a "$current_dir" | awk '!/^ / && !/^-/ && !/==/ && (/^..$/ || !/^\./)'`
	selected_file=`echo "$file_list" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "File Selector" -b "Press A to play, START to exit."`

	if [ -d "$current_dir/$selected_file" ]; then
		current_dir="$current_dir/$selected_file"
		continue
	fi

	if [ -f "$current_dir/$selected_file" ]; then
		echo "$current_dir/$selected_file" > "$home_dir/.selected.pls"
		exit=1
	fi

done
