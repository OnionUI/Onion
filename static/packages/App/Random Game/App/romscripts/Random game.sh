#!/bin/sh
scriptlabel="Random game (%LIST%)"
scriptinfo="Launches a random game\nfrom the selected list."

TAB_GAMES=1
TAB_FAVORITES=2
TAB_APPS=3
TAB_RECENTS=10
TAB_EXPERT=16
current_tab=$(cat /tmp/state.json | grep "\"type\":" | sed -e 's/^.*:\s*//g' | sed -e 's/\s*,$//g' | xargs | awk '{ print $2 }')

if [ $current_tab -eq $TAB_GAMES ] || [ $current_tab -eq $TAB_EXPERT ]; then
    arg=`pwd -P`
elif [ $current_tab -eq $TAB_RECENTS ]; then
    arg="--recents"
elif [ $current_tab -eq $TAB_FAVORITES ]; then
    arg="--favorites"
else
    exit
fi

cd /mnt/SDCARD/App/RandomGamePicker

./random.sh "$arg"
