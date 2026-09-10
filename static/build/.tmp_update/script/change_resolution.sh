#!/bin/sh

sysdir=/mnt/SDCARD/.tmp_update
logfile=$(basename "$0" .sh)
. $sysdir/script/log.sh

if [ -n "$1" ]; then
    res_x=$(echo "$1" | cut -d 'x' -f 1)
    res_y=$(echo "$1" | cut -d 'x' -f 2)
else
    res_x=640
    res_y=480
fi
log "Changing resolution to $res_x x $res_y"

if [ -x "$sysdir/bin/fbmode" ]; then
    probe=$($sysdir/bin/fbmode --probe 2> /dev/null)
    current_res=$(echo "$probe" | cut -d' ' -f1)
    current_pages=$(echo "$probe" | awk '{print $NF}')

    # Keep RetroArch-facing transitions at stock's two-page layout.
    if [ "$current_res" != "${res_x}x${res_y}" ] || [ "$current_pages" != "2" ]; then
        if ! "$sysdir/bin/fbmode" "${res_x}x${res_y}" --pages 2 \
            --preclear --linger 120 --timeout 500; then
            log "fbmode failed, falling back to fbset"
            fbset -g "$res_x" "$res_y" "$res_x" "$((res_y * 2))" 32
        else
            log "Committed ${res_x}x${res_y} (2 pages), was ${current_res:-unknown} (${current_pages:-?} pages)"
        fi
    else
        log "Already at ${res_x}x${res_y} (2 pages), skipping mode change"
    fi
else
    fbset -g "$res_x" "$res_y" "$res_x" "$((res_y * 2))" 32
fi

killall -SIGUSR1 batmon
killall -SIGUSR1 keymon
