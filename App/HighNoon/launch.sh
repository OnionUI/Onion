#!/bin/sh
echo $0 $*
time=$(cat time.txt);
date +%T -s $time;
./printstr "Changed clock to $time";