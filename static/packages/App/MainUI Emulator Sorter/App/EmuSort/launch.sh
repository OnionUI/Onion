#!/bin/sh
echo $0 $*
cd $(dirname "$0")
./emusort 2>errorlog.txt

# if emulators have been sorted, return to Games menu
if [ $? -eq 0 ]; then
    setState 3
fi
