#!/bin/sh
echo "> randomGamePicker $*"
sysdir=/mnt/SDCARD/.tmp_update

result=`randomGamePicker $*`

if [ $? -eq 99 ]; then
    infoPanel --title "RANDOM GAME" --message "NO GAMES FOUND\n \nIt looks like you don't have\nany valid roms." --auto
    exit 1
fi

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,"="); print a[2]}'
}

echo "$result"

emuname=`get_info_value "$result" emu`
label=`get_info_value "$result" label`
rompath=`get_info_value "$result" path`
imgpath=`get_info_value "$result" img`

infoPanel -t "RANDOM GAME" -i "$imgpath" -m "$(echo "$label" | fold -s -w 35)\n \n$emuname"
retcode=$?

echo "retcode: $retcode"

if [ $retcode -ne 0 ]; then
    echo "canceling random game..."
    rm -f $sysdir/cmd_to_run.sh 2> /dev/null
    exit 1
fi
