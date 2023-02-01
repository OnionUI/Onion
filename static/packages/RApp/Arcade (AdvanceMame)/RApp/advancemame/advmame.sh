#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
romname=`basename $1`

echo ${romname%.*}>/tmp/advmameRunning

# use SIGQUIT to normal exit and saving current position : https://github.com/amadvance/advancemame/blob/384e646234d17b01f89a0e3b38cfdea770d1a1d2/doc/advmenu.d#L1500
pkill -3 advmenu
	

