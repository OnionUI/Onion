#!/bin/sh
export sysdir=/mnt/SDCARD/.tmp_update
export miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

re_dir="/mnt/SDCARD/Media/Videos/Recorded"
active_file="/tmp/recorder_active"
lock_file="/tmp/screen_recorder.lock"
rec_icon="$sysdir/res/rec.png"

if [ -f "$lock_file" ]; then
    exit 1
fi

touch "$lock_file"

if [ ! -d "$re_dir" ]; then
    mkdir -p "$re_dir"
fi

cd $re_dir

# we'll hopefully never need this, but it checks whether /dev/l has the screen init already (and mi_disp exists, otherwise we can't change the screen colour)
# the function will call, the binary will run but if it detects disp is init it will return
check_disp_init() {
    if [ -z "$sysdir" ] || [ ! -x "$sysdir/bin/disp_init" ]; then
        echo "Error: disp_init not found or not executable"
        exit
    fi

    if ! pgrep -f "/dev/l" >/dev/null; then
        $sysdir/bin/disp_init &
    fi
}

file_write() {
    echo "$2" >"$1"
}

setup_gpio_for_vibration() {
    file_write "/sys/class/gpio/export" "48"
    file_write "/sys/class/gpio/gpio48/direction" "out"
}

# use the vibration motor to countdown
# pulse -> pulse -> pulse ... recording/stopped
#  3        2        1          0

vibrate() {
    file_write "/sys/class/gpio/gpio48/value" "$([ "$1" -eq 1 ] && echo "0" || echo "1")"
}

short_vibration() {
    vibrate 1
    usleep 15000
    vibrate 0
}

strong_vibration() {
    vibrate 1
    usleep 100000
    vibrate 0
}

pulsating_vibration() {
    loop_count="${1:-6}"

    setup_gpio_for_vibration

    if [ ! -f "/tmp/time_warp" ]; then
        usleep_duration=550000
    else
        usleep_duration=450000
        rm -rf /tmp/time_warp
    fi

    i=1
    while [ "$i" -le "$loop_count" ]; do
        short_vibration
        usleep "$usleep_duration"
        i=$((i + 1))
    done

    strong_vibration
}

# use the screen to countdown
# pulse -> pulse -> pulse ... recording/stopped (then back to original screen prop)
#  3        2        1          0
# this display colour change doesn't appear in the video, neither does blue light filter.

show_countdown() {
    if [ $# -gt 0 ] && echo "$1" | grep -qE '^[0-9]+$' && [ "$1" -ge 0 ] && [ "$1" -le 10 ]; then
        count="$1"
        echo $count
    else
        count=0
    fi

    check_disp_init
    default_colour="128 128 128"
    pulse_colour="200 200 200"

    if [ -f "$sysdir/config/.blf" ] && [ -f "/tmp/blueLightOn" ]; then
        combinedRGB=$(cat "$sysdir/config/display/blueLightRGB")
        combinedRGB=$(echo "$combinedRGB" | tr -d '[:space:]/#')

        blfB=$((combinedRGB & 0xFF))
        blfG=$(((combinedRGB >> 8) & 0xFF))
        blfR=$(((combinedRGB >> 16) & 0xFF))
        current_colour="$blfB $blfG $blfR"
    else
        current_colour=$default_colour
    fi

    original_colour=$current_colour

    if [ "$count" -gt 1 ]; then
        pulsating_vibration $(($count * 2)) &
    else
        pulsating_vibration 1 &
    fi

    for i in $(seq 1 "$count"); do
        for step in $(seq 1 10); do
            new_blue=$(echo "$original_colour $pulse_colour" | awk -v step="$step" '{printf "%.0f", $1 + ($4 - $1) * step / 10}')
            new_green=$(echo "$original_colour $pulse_colour" | awk -v step="$step" '{printf "%.0f", $2 + ($5 - $2) * step / 10}')
            new_red=$(echo "$original_colour $pulse_colour" | awk -v step="$step" '{printf "%.0f", $3 + ($6 - $3) * step / 10}')

            echo "colortemp 0 0 0 0 $new_blue $new_green $new_red" >/proc/mi_modules/mi_disp/mi_disp0
            usleep 20000
        done

        for step in $(seq 1 10); do
            new_blue=$(echo "$pulse_colour $original_colour" | awk -v step="$step" '{printf "%.0f", $1 + ($4 - $1) * step / 10}')
            new_green=$(echo "$pulse_colour $original_colour" | awk -v step="$step" '{printf "%.0f", $2 + ($5 - $2) * step / 10}')
            new_red=$(echo "$pulse_colour $original_colour" | awk -v step="$step" '{printf "%.0f", $3 + ($6 - $3) * step / 10}')

            echo "colortemp 0 0 0 0 $new_blue $new_green $new_red" >/proc/mi_modules/mi_disp/mi_disp0
            usleep 20000
        done
    done
}

show_indicator() {
    imgpop 10000 0 $rec_icon 150 435 >/dev/null 2>&1 &
}

toggle_ffmpeg() {
    sync
    if pgrep -f "ffmpeg -f fbdev -nostdin" >/dev/null; then
        cpuclock 1200
        pkill -2 -f "ffmpeg -f fbdev -nostdin"
        killall -9 imgpop

        if [ "$(cat "$sysdir/config/recCountdown")" -lt 1 ]; then
            show_countdown 0
        else
            show_countdown 1
        fi

        rm -f "$active_file"
    else
        if [ "$(cat "$sysdir/config/recCountdown")" -gt 1 ]; then
            show_countdown $(cat "$sysdir/config/recCountdown")
        else
            short_vibration &
        fi

        if [ -f "$sysdir/config/.recIndicator" ]; then
            show_indicator
        fi

        cpuclock 1600
        ffmpeg -f fbdev -nostdin -framerate 25 -i /dev/fb0 -vf "vflip,hflip, format=yuv420p" -c:v libx264 -preset superfast -maxrate 3000k -bufsize 6000k -threads 2 "$(date +%Y%m%d%H%M%S).mp4" >/dev/null 2>&1 &

        sleep 0.5

        if ! pgrep -f "ffmpeg -f fbdev -nostdin" >/dev/null; then
            return 1
        fi

        touch "$active_file"
    fi
    rm -f "$lock_file"
    return 0
}

hardkill_ffmpeg() {
    killall -2 ffmpeg # give it a chance

    sleep 0.5

    if pgrep -f "ffmpeg -f fbdev -nostdin" >/dev/null; then
        killall -9 ffmpeg
        rm -f "$lock_file"
        return 1
    fi

    rm -f "$active_file"
    rm -f "$lock_file"
    killall -9 imgpop
    return 0
}

case "$1" in
toggle)
    toggle_ffmpeg
    exit $?
    ;;
hardkill)
    hardkill_ffmpeg
    exit $?
    ;;
*)
    echo "Invalid argument. Usage: $0 {toggle|hardkill}"
    rm -f "$lock_file"
    exit 1
    ;;
esac

rm -f "$lock_file"
exit 0
