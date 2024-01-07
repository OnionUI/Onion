#!/bin/sh

migrate_drastic() {

    if [ -z "$1" ]; then
        echo "Error: Empty argument provided." >&2
        exit 1
    fi
    local old_drastic_folder="/mnt/SDCARD/Emu/$1"
    local backup_zip_file="/mnt/SDCARD/Emu/$1_old.7z"
    local old_save_folder="/mnt/SDCARD/Emu/$1/backup"
    local old_savestate_folder="/mnt/SDCARD/Emu/$1/savestates"
    local new_save_folder="/mnt/SDCARD/Saves/CurrentProfile/states/Drastic"
    local drastic_package_location="/mnt/SDCARD/App/PackageManager/data/Emu/Nintendo - DS (Drastic)"
    local game_list_cache="/mnt/SDCARD/Roms/NDS/NDS_cache6.db"

    if [ -d "$old_drastic_folder" ]; then
        echo "Migrating Drastic from $1 folder..." >>/tmp/.update_msg

        mkdir -p "$new_save_folder"
        cp "$old_save_folder"/*.* "$new_save_folder"/
        cp "$old_savestate_folder"/*.* "$new_save_folder"/

        7z a -r -mx0 "$backup_zip_file" "$old_drastic_folder"/
        rm -rf "$old_drastic_folder"/

        cp -r "$drastic_package_location"/* "/mnt/SDCARD/"
        rm "$game_list_cache"
    fi
}

# Migrate Drastic from Emu/NDS
migrate_drastic "NDS"

# Migrate Drastic from Emu/drastic
migrate_drastic "drastic"
