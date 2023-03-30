#!/bin/sh
emupath="$2"
sysdir=/mnt/SDCARD/.tmp_update
advmenu_rc_path=/mnt/SDCARD/BIOS/.advance/advmenu.rc

grep -vwE '^\s*(emulator_include|menu_base|menu_rel)' "$advmenu_rc_path" > temp
echo "emulator_include \"$(basename "$emupath")\"" >> temp
mv -f temp "$advmenu_rc_path"

cd $sysdir/bin/adv
./run_advmenu.sh
