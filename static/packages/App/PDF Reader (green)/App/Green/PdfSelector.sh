#!/bin/sh

SelectedPDF=$( (ls /mnt/SDCARD/Media/PDF) | awk '!/^-/ && !/==/' | "/mnt/SDCARD/App/Green/shellect.sh" -t "PDF Selector" -b "Press A to read, Start to exit.")


if [ ! "$SelectedPDF" = " LastRead.pls" ]; then
	echo /mnt/SDCARD/Media/PDF/${SelectedPDF}>"/mnt/SDCARD/Media/PDF/ LastRead.pls"
fi




  
  
 
	
 
