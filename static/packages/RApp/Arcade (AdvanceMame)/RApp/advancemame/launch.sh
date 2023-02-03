#!/bin/sh
echo $0 $*
sysdir=/mnt/SDCARD/.tmp_update
progdir=`dirname "$0"`
homedir=`dirname "$1"`
romname=`basename $1`
rompath="$1"
filename=`basename "$rompath"`

cd $progdir


if [ "$filename" = "~Run advmenu.shortcut" ]; then
	echo "Running advancemenu now !"
	
	while : ; do
		HOME=$progdir ./advmenu
		# Free memory
        $sysdir/bin/freemma
	
		if [ -f /tmp/advmameRunning ] ; then
			# Set CPU performance mode
			./cpufreq.sh
			# Run AdvanceMame
			value=`cat /tmp/advmameRunning`
			HOME=$progdir ./advmame "$value"
			# Free memory
			$sysdir/bin/freemma
			rm /tmp/advmameRunning
			echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
		else
			break
		fi
	done

	
else
	echo "Running game : \"$rompath\""
	# set CPU performance mode
	./cpufreq.sh
	# Running advancemame
	HOME=$progdir ./advmame "${romname%.*}" 
	
fi






