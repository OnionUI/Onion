#!/bin/sh
my_dir=`dirname $0`

/mnt/SDCARD/.tmp_update/script/stop_audioserver.sh

cd $my_dir
./launch2.sh
