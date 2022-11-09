#!/bin/sh
mydir=`dirname "$0"`

export LD_LIBRARY_PATH=$mydir/libs:/mnt/SDCARD/.tmp_update/lib/parasyte:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$mydir/libs:/mnt/SDCARD/.tmp_update/lib/parasyte:$LD_LIBRARY_PATH

if ! [ -f "/mnt/SDCARD/App/Terminal/st" ]; then 
	cp -Rf "/mnt/SDCARD/miyoo/packages/App/Terminal (Developer tool)/App/" "/mnt/SDCARD/"
fi

while :
do
	cd $mydir
	echo "    We run Terminal app with a script in parameter to have a kind of selector menu"
	"/mnt/SDCARD/App/Terminal/st" -q -e "/mnt/SDCARD/App/Green/PdfSelector.sh"   # -e to run a script without help at start -q does not display the keyboard at start (thanks Eggs)
	retVal=$?
	
	# if we catch that terminal app has been killed then we exit
	echo ================= $retVal
	if [ $retVal -eq 137 ]; then
		exit
	fi

	# we retrieve the current movie create by the script PdfSelector.sh
	SelectedPDF=$(cat "/mnt/SDCARD/Media/PDF/ LastRead.pls")


cd $mydir
echo ./green "$SelectedPDF"
./green "$SelectedPDF"


done