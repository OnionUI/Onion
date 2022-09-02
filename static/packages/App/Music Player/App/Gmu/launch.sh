#!/bin/sh
my_dir=`dirname $0`

killall audioserver
killall audioserver.mod

cd $my_dir
./launch2.sh

/mnt/SDCARD/miyoo/app/audioserver &

