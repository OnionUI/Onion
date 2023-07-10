#!/bin/sh

pkill -9 -f audioserver
killall audioserver.mod 2> /dev/null
