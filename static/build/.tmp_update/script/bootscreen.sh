#!/bin/sh
system_theme="$(/customer/app/jsonval theme)"

bootScreen="${system_theme}skin/extra/bootScreen.png"
[ ! -f "$bootScreen" ] && bootScreen="/mnt/SDCARD/.tmp_update/res/bootScreen.png"

/mnt/SDCARD/.tmp_update/bin/imgfb "$bootScreen"
