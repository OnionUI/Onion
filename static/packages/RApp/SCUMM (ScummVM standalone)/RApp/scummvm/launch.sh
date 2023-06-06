#!/bin/sh
echo $0 $* > scummvm.log
progdir=`dirname "$0"`
HOME=$progdir LD_LIBRARY_PATH=$progdir:$LD_LIBRARY_PATH $progdir/scummvm >> scummvm.log 2>&1
