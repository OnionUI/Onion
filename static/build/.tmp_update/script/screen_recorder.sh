#!/bin/sh
export sysdir=/mnt/SDCARD/.tmp_update
export miyoodir=/mnt/SDCARD/miyoo
export PATH="$sysdir/bin:$PATH"
re_dir="/mnt/SDCARD/Media/Videos/Recorded"
active_file="/tmp/recorder_active"
lock_file="/tmp/screen_recorder.lock"

echo "Starting screen recorder script..."

if [ -f "$lock_file" ]; then
    exit 1
fi

touch "$lock_file"

if [ ! -d "$re_dir" ]; then
    mkdir -p "$re_dir"
fi

cd $re_dir

case "$1" in
    toggle)
        if pgrep -f "ffmpeg -f fbdev -nostdin" > /dev/null; then
            pkill -2 -f "ffmpeg -f fbdev -nostdin"
            rm -f "$active_file"
            rm -f "$lock_file"
            exit 0
        else
            ffmpeg -f fbdev -nostdin -framerate 15 -i /dev/fb0 -vf "vflip,hflip, format=yuv420p" -c:v libx264 -preset faster -tune zerolatency -maxrate 12000k -bufsize 36000k -threads 0 "$(date +%Y%m%d%H%M%S).mp4" > /dev/null 2>&1 &
            sleep 0.5
            if pgrep -f "ffmpeg -f fbdev -nostdin" > /dev/null; then
                touch "$active_file"
                rm -f "$lock_file"
                exit 0
            else
                rm -f "$lock_file"
                exit 1
            fi
        fi
        ;;
    hardkill)
        killall -9 ffmpeg
        sleep 0.5
        if pgrep -f "ffmpeg -f fbdev -nostdin" > /dev/null; then
            rm -f "$lock_file"
            exit 1
        else
            rm -f "$active_file"
            rm -f "$lock_file"
            exit 0
        fi
        ;;
    *)
        echo "Invalid argument. Usage: $0 {toggle|hardkill}"
        rm -f "$lock_file"
        exit 1
esac

rm -f "$lock_file"
exit 0