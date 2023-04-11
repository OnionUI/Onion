#!/bin/sh

pkill -9 wpa_supplicant
pkill -9 udhcpc
pkill -9 -f audioserver

killall audioserver.mod 2> /dev/null
