to_minutes_since_midnight() {
    hour=$(echo "$1" | cut -d: -f1 | xargs)
    minute=$(echo "$1" | cut -d: -f2 | xargs)
    hour=${hour:-0}
    minute=${minute:-0}
    echo "$hour $minute" | awk '{print $1 * 60 + $2}'
}

enable_blue_light_filter() {
    current_blf=$(cat /mnt/SDCARD/.tmp_update/config/display/blueLightLevel)

    openvt -c 17 -- sh -c "sysdir=/mnt/SDCARD/.tmp_update && miyoodir=/mnt/SDCARD/miyoo && export LD_LIBRARY_PATH=\"/lib:/config/lib:\$miyoodir/lib:\$sysdir/lib:\$sysdir/lib/parasyte\" && \$sysdir/bin/tweaks --no_display --bluelightctrl $current_blf" &
    sleep 1 

    tweaks_pid=$(pgrep -f "$sysdir/bin/tweaks --no_display --bluelightctrl $current_blf")
    sleep 2

    if [ -n "$tweaks_pid" ]; then
        kill -SIGINT $tweaks_pid
    fi

    echo ":: Blue Light Filter: Enabled"
    touch /tmp/blueLightOn
}

disable_blue_light_filter() {
    openvt -c 17 -- sh -c 'sysdir=/mnt/SDCARD/.tmp_update && miyoodir=/mnt/SDCARD/miyoo && export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte" && $sysdir/bin/tweaks --no_display --bluelightctrl 0' &
    sleep 1

    tweaks_pid=$(pgrep -f "$sysdir/bin/tweaks --no_display --bluelightctrl 0")
    sleep 2
    
    if [ -n "$tweaks_pid" ]; then
        kill -SIGINT $tweaks_pid
    fi

    echo ":: Blue Light Filter: Disabled"
    rm /tmp/blueLightOn
}

check_blf() {
    sync
    blueLightTimeOn=$(cat /mnt/SDCARD/.tmp_update/config/display/blueLightTime)
    blueLightTimeOff=$(cat /mnt/SDCARD/.tmp_update/config/display/blueLightTimeOff)

    currentTime=$(date +"%H:%M")
    currentTimeMinutes=$(to_minutes_since_midnight "$currentTime")
    blueLightTimeOnMinutes=$(to_minutes_since_midnight "$blueLightTimeOn")
    blueLightTimeOffMinutes=$(to_minutes_since_midnight "$blueLightTimeOff")

    if [ "$blueLightTimeOffMinutes" -lt "$blueLightTimeOnMinutes" ]; then
        if [ "$currentTimeMinutes" -ge "$blueLightTimeOnMinutes" ] || [ "$currentTimeMinutes" -lt "$blueLightTimeOffMinutes" ]; then
            if [ ! -f /tmp/blueLightOn ]; then
                enable_blue_light_filter 
            fi
        else
            if [ -f /tmp/blueLightOn ]; then
                disable_blue_light_filter 
            fi
        fi
    else
        if [ "$currentTimeMinutes" -ge "$blueLightTimeOnMinutes" ] && [ "$currentTimeMinutes" -lt "$blueLightTimeOffMinutes" ]; then
            if [ ! -f /tmp/blueLightOn ]; then
                enable_blue_light_filter 
            fi
        else
            if [ -f /tmp/blueLightOn ]; then
                disable_blue_light_filter 
            fi
        fi
    fi
}

case "$1" in
    enable)
        enable_blue_light_filter
        ;;
    disable)
        disable_blue_light_filter
        ;;
    check)
        check_blf
        ;;
    *)
        echo "Usage: $0 {enable|disable|check}"
        exit 1
        ;;
esac