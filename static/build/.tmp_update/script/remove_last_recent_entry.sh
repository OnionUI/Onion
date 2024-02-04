#!/bin/sh

sysdir=/mnt/SDCARD/.tmp_update
recentlist=/mnt/SDCARD/Roms/recentlist.json
recentlist_hidden=/mnt/SDCARD/Roms/recentlist-hidden.json
recentlist_temp=/tmp/recentlist-temp.json

if [ ! -f $sysdir/config/.showRecents ]; then
    currentrecentlist=$recentlist_hidden
else
    currentrecentlist=$recentlist
fi

sed -i '1d' $currentrecentlist
sync
