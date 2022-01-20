#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
cd $progdir
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$progdir
HOME=/mnt/SDCARD $progdir/OpenBOR.ss
