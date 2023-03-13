#!/bin/sh


echo -ne "\n\n"
echo "==================================================================="
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

echo "==================================================================="

if [ -f "/tmp/MissingPortFile.tmp" ]; then
	MissingPortFile=`cat /tmp/MissingPortFile.tmp`
	echo "Game Data File not found "
	echo -ne  "===================================================================\n\n"
	rm "/tmp/MissingPortFile.tmp"
    cd /mnt/SDCARD/.tmp_update
	    ./bin/infoPanel --title "Port Launching Error"  --message "Port files not found :\n$MissingPortFile"
	
fi


