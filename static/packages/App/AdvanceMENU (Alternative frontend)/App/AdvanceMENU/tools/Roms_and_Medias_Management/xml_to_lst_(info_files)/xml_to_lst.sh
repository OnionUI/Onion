#!/bin/sh

#This is a sample script for Linux to convert xml to lst.
# Credits : Schmurtz - Onion Team

xml2info < advmame_miyoo.xml > advmame.lst


#   Alternative way to create lst file on Onion : 
#   --------------------------------------------

# sysdir=/mnt/SDCARD/.tmp_update
# export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$sysdir/lib:$sysdir/lib/parasyte"
# cd /mnt/SDCARD/RApp/advancemame/
# HOME=/mnt/SDCARD/RApp/advancemame ./advmame -listxml | ./tools/xml2info > ./.advance/advmame.lst