#!/bin/sh
echo $0 $*
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/infoPanel -t "Launching OpenSSH" -m "SSH server has been launched !" --auto &

cd $(dirname "$0")

if ! [ -f /appconfigs/ssh_host_ed25519_key ] ; then
	cp ./ssh_host_ed25519_key /appconfigs/
	chmod 600 /appconfigs/ssh_host_ed25519_key
fi

pkill -9 sshd

mkdir /var/empty

/mnt/SDCARD/App/Ssh/sshd -f /mnt/SDCARD/App/Ssh/sshd_config
