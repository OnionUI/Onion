#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

cd $progdir
./cpufreq.sh

killall audioserver
killall audioserver.mod

HOME=$homedir $progdir/FAKE08 "$1"

/mnt/SDCARD/miyoo/app/audioserver &

