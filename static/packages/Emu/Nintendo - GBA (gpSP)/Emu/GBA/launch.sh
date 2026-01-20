#!/bin/sh
echo $0 $*
progdir=$(dirname "$0")
homedir=$(dirname "$1")
save_dir=/mnt/SDCARD/Saves/CurrentProfile/saves
state_dir=/mnt/SDCARD/Saves/CurrentProfile/states
migration_log="$save_dir/gpSP/migration_status.log"

# Ensure migration log exists
touch "$migration_log"

# Extract ROM name without the extension
romname="${1##*/}"
romname="${romname%.*}"

config_dir="$homedir/.game_config"
romcfgpath="$config_dir/$romname.cfg"
auto_core_marker="$config_dir/${romname}.auto_core_switched"

gpsp_state="$state_dir/gpSP/$romname.state.auto"
mgba_state="$state_dir/mGBA/$romname.state.auto"

mgba_save="$save_dir/mGBA/$romname.srm"

default_core=gpsp

# Switch to mGBA if no gpSP state but mGBA state exists
if [ ! -f "$auto_core_marker" ]; then
    if [ ! -f "$gpsp_state" ] && [ -f "$mgba_state" ]; then
        default_core=mgba
        # Mark this switch as completed so it's only done once
        touch "$auto_core_marker"
    fi
fi

# Helper: Check if the ROM exists in the migration log
needs_migration() {
    grep -Fxq "$romname" "$migration_log" && return 0 || return 1
}

# Show info panel about migration
if [ "$default_core" = "gpsp" ] && [ -f "$mgba_save" ] && needs_migration; then
    infoPanel \
        --title "mGBA to gpSP migration" \
        --message "Conflicting GBA saves found.\n\
Default core is gpSP.\n\
\n\
In GLO (press Y in game list):\n\
- Transfer mGBA save: Overwrite gpSP save\n\
- Game core: Use mGBA for this game\n\
\n\
Press OK to load gpSP save."
    if [ $? -eq 0 ]; then
        # If not canceled, remove from migration log
        sed -i "/^$romname$/d" "$migration_log"
    else
        # If canceled, exit without launching
        exit 0
    fi
fi

# Write core setting to rom config if not default
if [ "$default_core" != "gpsp" ]; then
    mkdir -p "$config_dir"
    echo "core = \"${default_core}_libretro\"" > "$romcfgpath"
fi

cd /mnt/SDCARD/RetroArch
HOME=/mnt/SDCARD/RetroArch ./retroarch -v -L .retroarch/cores/${default_core}_libretro.so "$1"
