#!/bin/sh

echo $0 $*
progdir=`dirname "$0"`


cd $progdir
HOME=$progdir
a=`ps | grep audioserver | grep -v grep`
if [ "$a" == "" ] ; then
  export LD_LIBRARY_PATH=$progdir:$LD_LIBRARY_PATH
fi
$progdir/gnp.elf

