#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
emudir=/mnt/SDCARD/Emu/SEARCH

if [ -f $sysdir/active_search ]; then
    mkdir -p $emudir
    cp $sysdir/res/search_config.json $emudir/config.json
fi
