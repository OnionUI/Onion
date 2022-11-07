#!/bin/sh

SelectedVideo=$( (ls /mnt/SDCARD/Media/Videos) | awk '!/^-/ && !/==/' | "/mnt/SDCARD/App/FFplay/shellect.sh" -t "Video Selector" -b "Press A to play, Start to exit.")

 
echo ============
echo /mnt/SDCARD/Media/Videos/${SelectedVideo}

if [ ! "$SelectedVideo" = " LastPlay.pls" ]; then
	echo /mnt/SDCARD/Media/Videos/${SelectedVideo}>"/mnt/SDCARD/Media/Videos/ LastPlay.pls"
fi


 # Mychoice=$( echo -e "Yes\nNo" | /mnt/SDCARD/App/FFplay/shellect.sh -t "Do you want to resume the playback of \"${SelectedVideo}\" ?" -b "Press start to validate your choice.")
 
 # if [ "$Mychoice" = "No" ]; then
	## mv /mnt/SDCARD/App/FFplay/pos.cfg /mnt/SDCARD/App/FFplay/pos.bck
	# touch /mnt/SDCARD/App/FFplay/.noResume
 # fi
  