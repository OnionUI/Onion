#!/bin/sh
echo $0 $*

IP=$(ip route get 1 | awk '{print $NF;exit}')

LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/infoPanel -t "Launching http FileBrowser" -m "http FileBrowser has been launched: $IP" --auto &

pkill -9 wpa_supplicant
pkill -9 udhcpc
pkill -9 filebrowser
/mnt/SDCARD/App/FileBrowser/filebrowser config set --branding.name "Onion"
/mnt/SDCARD/App/FileBrowser/filebrowser config set --branding.files "/mnt/SDCARD/App/FileBrowser/theme"
/mnt/SDCARD/App/FileBrowser/filebrowser -p 80 -a $IP  -r /mnt/SDCARD &