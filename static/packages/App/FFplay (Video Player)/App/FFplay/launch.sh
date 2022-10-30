#!/bin/sh
mydir=`dirname "$0"`

export LD_LIBRARY_PATH=/mnt/SDCARD/.tmp_update/lib/parasyte:$LD_LIBRARY_PATH
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor


if ! [ -f "/mnt/SDCARD/App/Terminal/st" ]; then 
	cp -Rf "/mnt/SDCARD/miyoo/packages/App/Terminal (Developer tool)/App/" "/mnt/SDCARD/"
fi

while :
do
	cd $mydir
	echo "    We run Terminal app with a script in parameter to have a kind of selector menu"
	"/mnt/SDCARD/App/Terminal/st" -q -e "/mnt/SDCARD/App/FFplay/VideoSelector.sh"   # -e to run a script without help at start -q does not display the keyboard at start (thanks Eggs)
	retVal=$?
	
	# if we catch that terminal app has been killed then we exit
	echo ================= $retVal
	if [ $retVal -eq 137 ]; then
		exit
	fi

	# we retrieve the current movie create by the script VideoSelector.sh
	SelectedVideo=$(cat "/mnt/SDCARD/Media/Videos/ LastPlay.pls")

	# attempt to restore resume of other video -> doesn't work
	#(sleep 2 ; mv /mnt/SDCARD/App/FFplay/pos.bck /mnt/SDCARD/App/FFplay/pos.cfg ;) &

	# attempt to avoid resume of other video -> doesn't work, the parameter is not considered
	if [ -f "/mnt/SDCARD/App/FFplay/.noResume" ]; then 
		startTimer=" -ss 0"
		rm "/mnt/SDCARD/App/FFplay/.noResume"
	fi

	touch /tmp/stay_awake
	ffplay  -autoexit -vf "hflip,vflip" -i "$SelectedVideo" $startTimer
	rm -f /tmp/stay_awake
done