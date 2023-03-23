#!/bin/sh

rm -f "/mnt/SDCARD/Roms/SCUMMVM/Shortcuts/~Import games.target" 2> /dev/null

/mnt/SDCARD/.tmp_update/script/reset_list.sh "/mnt/SDCARD/Emu/SCUMMVM/../../Roms/SCUMMVM/Shortcuts"
