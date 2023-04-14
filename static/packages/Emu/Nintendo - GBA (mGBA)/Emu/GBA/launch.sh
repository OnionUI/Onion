#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`
savedir=/mnt/SDCARD/Saves/CurrentProfile/saves

# migration information gpSP to mGBA

if ! [ -f "$savedir/mGBA/.gpspImportDone" ]; then 
    mkdir $savedir/mGBA
    
    ls $savedir/gpSP/*.sav
    retVal=$?

    if [ $retVal -eq 0 ]; then
        LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
            -t "GBA CORE CHANGED" \
            -m "mGBA offers improved game compatibility.\nRefer to the Onion Wiki.\n \nDo you want to import saves from gpSP?" \
            "Yes" \
            "No"

        retcode=$?

        if [ $retcode -eq 0 ]; then
            cp $savedir/gpSP/*.sav $savedir/mGBA
            
            for file in $savedir/mGBA/*.sav; do
                mv -n -- "$file" "${file%.sav}.srm"
            done

            rm $savedir/mGBA/*.sav
        fi
    fi

    touch "$savedir/mGBA/.gpspImportDone"
fi

cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ ./retroarch -v -L .retroarch/cores/mgba_libretro.so "$1"
