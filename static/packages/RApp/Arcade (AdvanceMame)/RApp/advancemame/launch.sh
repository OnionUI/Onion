#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
rompath="$1"
romname=`basename "$rompath"`
sysdir=/mnt/SDCARD/.tmp_update
homedir=/mnt/SDCARD/BIOS

echo -e "\n\n:: [AdvanceMAME] Running game: ${romname%.*}\n\n"

# set CPU performance mode
$progdir/cpufreq.sh

# Running advancemame
cd $sysdir/bin/adv
HOME=$homedir ./advmame "${romname%.*}" 
