#!/bin/sh


echo -ne "\n\n"
echo --------------------------------------------------------------------
echo ":: MAIN SCRIPT PORT LAUNCH"
echo --------------------------------------------------------------------


echo ":: PORT_LAUNCH" $0 $*
echo --------------------------------------------------------------------

cd "`dirname "$1"`"

chmod a+x "$1"

# Launch port file
"$1"

retcode=$?

if [ $retcode -eq 66 ]; then
	echo "Game Data File not found "
    cd /mnt/SDCARD/.tmp_update
	MissingPortFile=`cat /tmp/MissingPortFile.tmp`
    ./bin/infoPanel --title "Port Launching Error"  --message "Port files not found :\n$MissingPortFile"
fi