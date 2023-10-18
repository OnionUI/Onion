#!/bin/sh

if [ "$1" = "0" ]; then
    ifconfig wlan0 down
    killall -15 wpa_supplicant
    killall -15 udhcpc
    sleep 0.5
    /customer/app/axp_test wifioff
elif [ "$1" = "1" ]; then
    /customer/app/axp_test wifion
    sleep 2
    ifconfig wlan0 down
    killall -15 wpa_supplicant
    killall -15 udhcpc
    ifconfig wlan0 up
    wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
    udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
else
    echo "Usage: $0 [0|1]"
    exit 1
fi