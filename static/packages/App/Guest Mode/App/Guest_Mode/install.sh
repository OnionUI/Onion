#!/bin/sh
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

cd $progdir

if [ -d /mnt/SDCARD/Saves/MainProfile ] ; then
	# The guest profile is the current one
	cp ./data/configON.json ./config.json
fi
