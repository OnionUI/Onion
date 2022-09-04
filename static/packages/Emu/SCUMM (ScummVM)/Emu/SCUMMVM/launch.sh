#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

rompath="$1"
filename=`basename "$rompath"`
gamename="${filename%.*}"

if [ -d "$homedir/$gamename" ] ; then
    rompath="$homedir/$gamename/$gamename.scummvm0"
    cp "$1" "$rompath"
    echo "scummvm file: $rompath"
fi

# Timer initialisation
cd /mnt/SDCARD/App/PlayActivity
./playActivity "init"

cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ $progdir/../../RetroArch/retroarch -v -L $progdir/../../RetroArch/.retroarch/cores/scummvm_libretro.so "$rompath"

# Timer registration
cd /mnt/SDCARD/App/PlayActivity
./playActivity "$rompath"
