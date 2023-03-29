#!/bin/sh
emupath=`dirname "$0"`
rompath="$1"
sysdir=/mnt/SDCARD/.tmp_update

$sysdir/bin/adv/advexec.sh "$rompath" "$emupath"
