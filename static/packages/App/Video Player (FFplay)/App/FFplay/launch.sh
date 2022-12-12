#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
mediadir=/mnt/SDCARD/Media/Videos
appdir=/mnt/SDCARD/App/FFplay

while :
do
	cd $sysdir
	./bin/st -q -e "$sysdir/script/file_selector.sh" "Video Player" "$mediadir" "3g2 3gp asf avi flv h264 m2t m2ts m4v mkv mod mov mp4 mpg tod vob webm wmv"
	retVal=$?
	
	# if we catch that terminal app has been killed then we exit
	echo "=================" "$retVal"
	if [ $retVal -eq 137 ] || [ ! -f "$mediadir/.selected.pls" ]; then
		exit
	fi

	selected_file=$(cat "$mediadir/.selected.pls")

	if [ ! -f "$selected_file" ]; then
		continue
	fi

	# attempt to restore resume of other video -> doesn't work
	#(sleep 2 ; mv /mnt/SDCARD/App/FFplay/pos.bck /mnt/SDCARD/App/FFplay/pos.cfg ;) &

	# attempt to avoid resume of other video -> doesn't work, the parameter is not considered
	if [ -f "$appdir/.noResume" ]; then 
		startTimer=" -ss 0"
		rm "$appdir/.noResume"
	fi

	echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

	cd $sysdir
	touch /tmp/stay_awake
	./bin/ffplay -autoexit -vf "hflip,vflip" -i "$selected_file" $startTimer
	rm -f /tmp/stay_awake
done
