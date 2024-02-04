#!/bin/sh
progdir=`dirname "$0"`
rompath="$1"
emupath="$2"
sysdir=/mnt/SDCARD/.tmp_update

infoPanel --persistent &
touch /tmp/dismiss_info_panel
sync

echo "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"$emupath/launch.sh\" \"$rompath\"" > $sysdir/cmd_to_run.sh
touch /tmp/quick_switch

pkill -3 advmenu
echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable
