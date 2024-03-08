#!/bin/sh
system_theme="$(/customer/app/jsonval theme)"

bootScreen="/mnt/SDCARD/Saves/CurrentProfile/romScreens/bootScreen.png"
[ ! -f "$bootScreen" ] && bootScreen="${system_theme}skin/extra/bootScreen.png"
[ ! -f "$bootScreen" ] && bootScreen="/mnt/SDCARD/.tmp_update/res/bootScreen.png"

echo ":: Chosen bootscreen: $bootScreen"
/mnt/SDCARD/.tmp_update/bin/imgfb "$bootScreen"

rm -f "/mnt/SDCARD/Saves/CurrentProfile/romScreens/bootScreen.png"