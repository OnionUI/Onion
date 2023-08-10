#!/bin/sh

sysdir=/mnt/SDCARD/.tmp_update
PATH="$sysdir/bin:$PATH"
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/mnt/SDCARD/.tmp_update/lib:$sysdir/lib/parasyte"

config_file="/mnt/SDCARD/.tmp_update/config/scraper.json"

# Read the content of the file into a variable
config_json=$(cat "$config_file")

# Check if the content is empty or invalid JSON
if [ -z "$config_json" ] || ! echo "$config_json" | jq . >/dev/null 2>&1; then
    # If JSON is empty or invalid, use default values
    config_json='{
      "screenscraper_username": "",
      "screenscraper_password": "",
      "Retroarch_enabled": "true",
      "Screenscraper_enabled": "true",
      "Launchbox_enabled": "true",
      "ScrapeInBackground": "false",
      "RetroarchMediaType": "Named_Boxarts",
      "ScreenscraperMediaType": "box-2D",
      "LaunchboxMediaType": "Box - Front",
      "LaunchboxRegion": "United States",
      "ScreenscraperRegion": "us"
    }'
fi

# Update missing tags
updated_config_json=$(echo "$config_json" | jq 'if .screenscraper_username == null then .screenscraper_username = "" else . end
    | if .screenscraper_password == null then .screenscraper_password = "" else . end
    | if .Retroarch_enabled == null then .Retroarch_enabled = "true" else . end
    | if .Screenscraper_enabled == null then .Screenscraper_enabled = "true" else . end
    | if .Launchbox_enabled == null then .Launchbox_enabled = "true" else . end
    | if .ScrapeInBackground == null then .ScrapeInBackground = "false" else . end
    | if .RetroarchMediaType == null then .RetroarchMediaType = "Named_Boxarts" else . end
    | if .ScreenscraperMediaType == null then .ScreenscraperMediaType = "box-2D" else . end
    | if .LaunchboxMediaType == null then .LaunchboxMediaType = "Box - Front" else . end
    | if .LaunchboxRegion == null then .LaunchboxRegion = "United States" else . end
    | if .ScreenscraperRegion == null then .ScreenscraperRegion = "us" else . end')

# Check if the content has changed
if [ "$config_json" != "$updated_config_json" ]; then
    # Write the updated content to the file
    echo "$updated_config_json" > "$config_file"
	sync
    # echo "Config file fixed."
# else
	# printf "Config file OK."
fi

