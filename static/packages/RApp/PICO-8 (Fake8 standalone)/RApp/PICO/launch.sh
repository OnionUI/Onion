#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
cd $progdir
./cpufreq.sh

# Timer initialisation
cd /mnt/SDCARD/App/PlayActivity
./playActivity "init"

killall audioserver
killall audioserver.mod

HOME=$homedir $progdir/FAKE08 "$1"

/mnt/SDCARD/miyoo/app/audioserver &

# Timer registration
cd /mnt/SDCARD/App/PlayActivity
./playActivity "$1"