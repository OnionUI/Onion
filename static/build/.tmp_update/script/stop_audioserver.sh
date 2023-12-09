#!/bin/sh

curvol=$(cat /proc/mi_modules/mi_ao/mi_ao0 | awk '/LineOut/ {if (!printed) {print $8; printed=1}}' | sed 's/,//')

pkill -9 -f audioserver
killall audioserver.mod 2> /dev/null


set_snd_level() {
    for i in $(seq 1 500); do
        if [ -e /proc/mi_modules/mi_ao/mi_ao0 ]; then
            echo "set_ao_volume 0 ${curvol}dB" > /proc/mi_modules/mi_ao/mi_ao0
            echo "set_ao_volume 1 ${curvol}dB" > /proc/mi_modules/mi_ao/mi_ao0
            echo "Volume set to ${curvol}dB"
            break
        fi
        sleep 0.1
    done
}

set_snd_level &
