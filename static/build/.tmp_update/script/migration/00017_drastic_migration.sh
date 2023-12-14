#!/bin/sh

old_drastic_folder="/mnt/SDCARD/Emu/drastic"
backup_zip_file="/mnt/SDCARD/Emu/drastic.7z"

old_save_folder="/mnt/SDCARD/Emu/drastic/savestates"
new_save_folder="/mnt/SDCARD/Saves/CurrentProfile/states/Drastic"

drastic_package_location="/mnt/SDCARD/App/PackageManager/data/Emu/Nintendo - DS (Drastic)"
game_list_cache="/mnt/SDCARD/Roms/NDS/NDS_cache6.db"

if [ -d "$old_drastic_folder" ]; then
    echo "Migrating Drastic ..." >> /tmp/.update_msg
  
    mkdir -p "$new_save_folder"
    cp "$old_save_folder"/*.* "$new_save_folder"/

    7z a -r -mx0 "$backup_zip_file" "$old_drastic_folder"
    rm -rf "$old_drastic_folder"
    
    cp -r "$drastic_package_location"/* "/mnt/SDCARD/"
    rm "$game_list_cache"
fi