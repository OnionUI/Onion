#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

cd "$progdir"

export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

HOME=/mnt/SDCARD/Saves/CurrentProfile/states ./gngeo --rompath=/mnt/SDCARD/BIOS "$1"
