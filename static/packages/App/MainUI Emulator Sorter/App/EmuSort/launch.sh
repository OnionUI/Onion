#!/bin/sh
echo $0 $*
cd $(dirname "$0")
./emusort 2>errorlog.txt

# set MainUI state to Games menu
setState 3
