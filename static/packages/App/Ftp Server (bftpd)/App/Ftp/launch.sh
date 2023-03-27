#!/bin/sh
echo $0 $*
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/infoPanel -t "Launching FTP server" -m "FTP server has been launched !" --auto &

cd $(dirname "$0")
pkill -9 bftpd
./bftpd -d

