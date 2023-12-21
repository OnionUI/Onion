#!/bin/sh

json_file="/mnt/SDCARD/system.json"
currentThemefile="/mnt/SDCARD/Saves/CurrentProfile/theme/currentTheme"
defautTheme="/mnt/SDCARD/Themes/Silky by DiMo/"

progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
cd $progdir

# Save current time
./saveTime.sh
sync

# Save current Favourites + RecentList lists
cp /mnt/SDCARD/Roms/*.json /mnt/SDCARD/Saves/CurrentProfile/lists
rm /mnt/SDCARD/Roms/*.json

# Save current theme
jq -r .theme "$json_file" > "$currentThemefile"

if [ -d /mnt/SDCARD/Saves/GuestProfile ] ; then
	# The main profile is the current one
	cp ./data/configON.json ./config.json
	mv /mnt/SDCARD/Saves/CurrentProfile /mnt/SDCARD/Saves/MainProfile
	mv /mnt/SDCARD/Saves/GuestProfile /mnt/SDCARD/Saves/CurrentProfile

else
	# The guest profile is the current one
	cp ./data/configOFF.json ./config.json
	mv /mnt/SDCARD/Saves/CurrentProfile /mnt/SDCARD/Saves/GuestProfile
	mv /mnt/SDCARD/Saves/MainProfile /mnt/SDCARD/Saves/CurrentProfile
fi

# Favourites + RecentList restoration
cp /mnt/SDCARD/Saves/CurrentProfile/lists/*.json /mnt/SDCARD/Roms

if [ ! -e "$currentThemefile" ]; then
	# Default theme file
    echo "$themeString" > "$currentThemefile"
fi

# Theme restoration
if [ -e "$currentThemefile" ]; then
	jq --arg theme "$(cat "$currentThemefile")" '.theme = $theme' "$json_file" > temp.json
	mv temp.json "$json_file"
fi
themeSwitcher --reapply_icons

# Load current time
./loadTime.sh
