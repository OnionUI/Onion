#!/bin/sh

# This script allows moving roms to subfolders within a system
# Can help managing roms e.g. 
# Requires creating those folders first manually e.g. Favoruites, RPGs, Fighting, Relaxing etc
# From GLO, Select `Move To ...`
# This will show a list of all folders within including the top folder in that system
# Choosing any of those folders will move the rom there.
# Requires a refresh ofcourse to see the changes

scriptlabel="Move to ..."
echo $0 $*

# turns lines into quoted strings
list_args() {
    printf "\"%s\" " $(echo "$1" | sed "s/'/\'/g" )
}

# sysdir=/mnt/SDCARD/.tmp_update

# get evaluated path of rom e.g. turns path/to/../file to path/file
rom_file_path=$(realpath "$1")
# get path up to this systems main folder from rom path e.g. /mnt/SDCARD/Roms/GBA/Action/combat.zip to /mnt/SDCARD/Roms/GBA/
game_system_path=$(echo $rom_file_path | cut -d'/' -f-5)
# get all directories in this system path
folders=$(ls -d $game_system_path/*/ | cut -f6 -d'/')
# add reference to top dir too. will appear as a '.' in the list
folders=$(echo -e ".\n$folders")
folders_count=$(echo "$folders" | wc -l)

# show list of folders 
runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t \"Move rom to folder\" $(list_args "$folders")"

eval $runcmd
retcode=$?

if [ $retcode -lt 0 ] || [ $retcode -ge $folders_count ]; then
    exit 1
fi

# get folder name to move to using returned index
target_folder=$(echo $folders | awk -v N=$((retcode + 1)) '{print $N}')

mv "$rom_file_path" "$game_system_path/$target_folder"
