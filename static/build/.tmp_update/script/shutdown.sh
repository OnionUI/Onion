#!/bin/sh

task_killer() {
	r=0
	for p in $1; do
		if [ -d "/proc/$p" ] && [ $p -ne $$ ]; then
			kill $2 $p
			r=1
		fi
	done
	return $r
}

kill_hooked_tasks() {
	c=0
	while [ $c -lt 5 ]; do
		pids=$(fuser -m /mnt/SDCARD)
		if task_killer "$pids" $1; then
			return
		fi
		sleep 0.05
		c=$((c + 1))
	done
}

if [ "$0" = "/tmp/_shutdown" ]; then

	fuser -m /mnt/SDCARD >> /appconfigs/shutdown.log
	killall -9 main # mandatory to avoid the suppression of .tmp_update !
	killall -9 updater
	killall -9 runtime.sh
	
	kill_hooked_tasks
	sleep 0.1
	kill_hooked_tasks -9
	
	sync
	umount /mnt/SDCARD/RetroArch/retroarch
	umount /customer/lib/libgamename.so
	swapoff /mnt/SDCARD/cachefile
	umount -r /mnt/SDCARD
	umount /mnt/SDCARD
	
	############# DEBUG #############
	# fuser -m /mnt/SDCARD > /appconfigs/shutdown.log
	# lsof /mnt/SDCARD >> /appconfigs/shutdown.log
	# mount >> /appconfigs/shutdown.log
	#################################
	
	if [ "$1" = "-r" ]; then
		/sbin/reboot
	else
		/sbin/poweroff
	fi
	
fi

if [ ! -f /tmp/_shutdown ]; then
	cp -f /mnt/SDCARD/.tmp_update/script/shutdown.sh /tmp/_shutdown
fi

# run the script totally detached from current shell
pgrep -f /tmp/_shutdown || (set -m; su root -c "/usr/bin/nohup /tmp/_shutdown $1 >/dev/null 2>&1 &")
while true; do
	sleep 10
done