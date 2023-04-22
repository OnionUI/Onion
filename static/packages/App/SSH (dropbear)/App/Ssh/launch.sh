#!/bin/sh
echo $0 $*
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/infoPanel -t "Launching SSH Server" -m "SSH server has been launched !" --auto &

cd $(dirname "$0")

pkill -9 gesftpserver
pkill -9 dropbear

/mnt/SDCARD/App/Ssh/dropbear -R -B -z > /dev/null 2>&1 &

