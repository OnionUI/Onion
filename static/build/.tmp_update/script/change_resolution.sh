#!/bin/sh

res_x=""
res_y=""

if [ -n "$1" ]; then
    res_x=$(echo "$1" | cut -d 'x' -f 1)
    res_y=$(echo "$1" | cut -d 'x' -f 2)
else
    res_x=640
    res_y=480
fi
echo "Changing resolution to $res_x x $res_y"

fbset -g "$res_x" "$res_y" "$res_x" "$((res_y * 2))" 32

# inform batmon and keymon of resolution change
killall -SIGUSR1 batmon
killall -SIGUSR1 keymon
