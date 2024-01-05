#!/bin/sh

sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
unset LD_PRELOAD
export LD_LIBRARY_PATH="/lib:/config/lib:/mnt/SDCARD/miyoo/lib:/mnt/SDCARD/.tmp_update/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

cd $sysdir
mainUiBatPerc >/dev/null 2>&1