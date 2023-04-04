#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
export LD_LIBRARY_PATH="/lib:/config/lib:/customer/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:/sbin:/usr/sbin:/bin:/usr/bin:/config:/customer/app"
