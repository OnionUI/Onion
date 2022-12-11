#!/bin/sh
prompt_title="$1"
home_dir="$2"
file_extensions="$3"
current_dir=`cd -- "$home_dir" >/dev/null 2>&1; pwd -P`
selected_file=""

exit=0

while [ $exit -eq 0 ]; do

	dir_short=`basename "$current_dir"`

	file_list=`ls -a "$current_dir" | awk '!/^ / && !/^-/ && !/==/ && !/^\./'`
	file_list_filtered=`echo "$file_list" | awk -v d="$current_dir/" -v ext="$file_extensions" -F. '
	BEGIN {
		split(ext,tmp," "); for (i in tmp) ext_a[tmp[i]]
	}
	{
		if(system("[ -d \"" d $0 "\" ]") == 0)
			print($0 "/");
		else if (ext == "" || tolower($NF) in ext_a) {
			if (system("[ -f \"" d ".state_seen/" $0 "._seen\" ]") == 0)
				print("[x] " $0);
			else
				print("[ ] " $0);
		}
	}'`

	if [ "$current_dir" != "/mnt/SDCARD" ]; then
		file_list_filtered="../
$file_list_filtered"
	fi

	selected_file=`echo "$file_list_filtered" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "$dir_short - $prompt_title" -b "Press A to open, START to exit."`

	if [ -d "$current_dir/$selected_file" ]; then
		current_dir=`cd -- "$current_dir/$selected_file" >/dev/null 2>&1; pwd -P`
		continue
	fi

	selected_file=`echo "$selected_file" | cut -c 5-`

	if [ -f "$current_dir/$selected_file" ]; then
		mkdir -p "$current_dir/.state_seen"
		touch "$current_dir/.state_seen/$selected_file._seen"
		echo "$current_dir/$selected_file" > "$home_dir/.selected.pls"
		exit=1
	fi

done
