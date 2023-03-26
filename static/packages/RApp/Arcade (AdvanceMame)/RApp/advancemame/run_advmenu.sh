#!/bin/sh
progdir=`dirname "$0"`
sysdir=/mnt/SDCARD/.tmp_update

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
