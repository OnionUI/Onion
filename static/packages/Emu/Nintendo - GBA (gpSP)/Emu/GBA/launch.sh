#!/bin/sh
echo $0 $*
progdir=$(dirname "$0")
homedir=$(dirname "$1")
savedir=/mnt/SDCARD/Saves/CurrentProfile/saves
statedir=/mnt/SDCARD/Saves/CurrentProfile/states

# extract rom name without extension
romname=$(basename "$1" | sed 's/\.[^.]*$//')
romcfgpath="$homedir/.game_config/$romname.cfg"

gpsp_state="$statedir/gpSP/$romname.state.auto"
mgba_state="$statedir/mGBA/$romname.state.auto"

gpsp_save="$savedir/gpSP/$romname.srm"
mgba_save="$savedir/mGBA/$romname.srm"

default_core=gpsp

# check if gpSP save states exist
if [ ! -f "$gpsp_state" ] && [ ! -f "$gpsp_save" ]; then
    if [ -f "$mgba_save" ]; then
        LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
            -t "GBA CORE CHANGED" \
            -m "Default GBA core is now gpSP.\nDo you want to transfer your\nmGBA save file for this game?" \
            "Transfer save to gpSP" \
            "Keep playing with mGBA"

        retcode=$?

        if [ $retcode -eq 0 ]; then
            # remove weird gpSP file if it exists
            if [ -f "$savedir/gpSP" ]; then
                rm -f "$savedir/gpSP"
            fi

            # create gpSP save directory
            mkdir -p "$savedir/gpSP"

            # transfer save file
            cp "$mgba_save" "$gpsp_save"
        else
            default_core=mgba
        fi
    elif [ -f "$mgba_state" ]; then
        default_core=mgba
    fi
fi

# if core is mgba, create a config file
if [ "$default_core" = "mgba" ]; then
    mkdir -p "$homedir/.game_config"
    echo "core = \"mgba_libretro\"" > "$romcfgpath"
fi

cd /mnt/SDCARD/RetroArch
HOME=/mnt/SDCARD/RetroArch ./retroarch -v -L .retroarch/cores/${default_core}_libretro.so "$1"
