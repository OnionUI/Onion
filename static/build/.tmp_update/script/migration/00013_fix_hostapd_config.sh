#!/bin/sh
# Fix for missing ctrl interface call in the config, will cause failures to some easynetplay features without this.

hostapdconfdir=/mnt/SDCARD/.tmp_update/config/hostapd.conf
if ! grep -q "ctrl_interface=/var/run/hostapd" $hostapdconfdir; then
    sed -i '1ictrl_interface=/var/run/hostapd' $hostapdconfdir
fi