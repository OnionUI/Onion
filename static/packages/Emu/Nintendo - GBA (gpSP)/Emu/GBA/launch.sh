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
if [ ! -f "$gpsp_state" ] && [ ! -f "$gpsp_save" ] && [ [ -f "$mgba_state" ] || [ -f "$mgba_save" ] ]; then
    default_core=mgba

    mkdir -p "$homedir/.game_config"
    echo "core = \"mgba_libretro\"" > "$romcfgpath"
fi

cd /mnt/SDCARD/RetroArch
HOME=/mnt/SDCARD/RetroArch ./retroarch -v -L .retroarch/cores/${default_core}_libretro.so "$1"
