#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L "$progdir/../../RetroArch/.retroarch/cores/dosbox_pure_0.9.7_libretro.so" "$1"
