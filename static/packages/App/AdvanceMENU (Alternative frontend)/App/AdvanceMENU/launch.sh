#!/bin/sh
echo $0 $*
sysdir=/mnt/SDCARD/.tmp_update

infoPanel -t "AdvanceMENU" -m "LOADING" --persistent &
touch /tmp/dismiss_info_panel
sync

cd $sysdir/bin/adv
./run_advmenu.sh
