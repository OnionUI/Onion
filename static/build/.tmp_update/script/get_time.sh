#!/bin/sh

[ "$1" = "--12h" ] && FORMAT="+%I:%M %P" || FORMAT="+%H:%M"

TZ="$(cat "/mnt/SDCARD/.tmp_update/config/.tz")" date "$FORMAT"