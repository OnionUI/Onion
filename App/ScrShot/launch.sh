#!/bin/sh
echo $0 $*
cd $(dirname "$0")
HOME=/mnt/SDCARD
a=`ps | grep screenshot | grep -v grep`
if [ "$a" == "" ] ; then
    ./screenshot &
    ./printstr "L2+R2 Screenshot Ready"
else
    killall screenshot
    ./printstr "Screenshot Released"
fi
