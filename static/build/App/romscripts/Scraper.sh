#!/bin/sh
scriptlabel="Scraper (%LIST%)"
scriptinfo="Launches the scraper\nfor the selected system."
require_networking=1
echo $0 $*

sysdir=/mnt/SDCARD/.tmp_update
rm -f /tmp/scraper_script.sh

pressMenu2Kill st &

cd $sysdir
#./bin/st -q -e 	"/mnt/SDCARD/scrap_screenscraper.sh" "MD"   # quick alternative
./bin/st -q -e "$sysdir/script/scraper/menu.sh" "$1" "$2"

pkill -9 pressMenu2Kill


# background scraping :
if [ -f /tmp/scraper_script.sh ]; then
    chmod a+x /tmp/scraper_script.sh
    sh /tmp/scraper_script.sh &
fi

