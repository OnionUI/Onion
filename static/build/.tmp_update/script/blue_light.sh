sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo 
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
blf_key=$sysdir/config/.blf
blf_key_on_user=$sysdir/config/.blfOn
blf_key_on=/tmp/.blfOn
ignore_schedule=/tmp/.blfIgnoreSchedule

sync
lockfile="/tmp/blue_light_script.lock"

if [ -f "$lockfile" ]; then
    exit 1
fi

touch "$lockfile"
trap 'rm -f "$lockfile"; exit' INT TERM EXIT

setRGBValues() {
    value=$1
    case "$value" in
        0)
            endB=110; endG=125; endR=140 ;;
        1)
            endB=100; endG=120; endR=140 ;;
        2)
            endB=90;  endG=115; endR=140 ;;
        3)
            endB=80;  endG=110; endR=140 ;;
        4)
            endB=70;  endG=105; endR=140 ;;
        *)
            endB=128; endG=128; endR=128 ;;
    esac
}

set_intensity() {
    reset_to_default=${1:-1}
    sync

    value=$(cat $sysdir/config/display/blueLightLevel)

    if [ "$reset_to_default" -eq 0 ]; then
        echo "8421504" > $sysdir/config/display/blueLightRGB
        return
    fi
    
    currentRGB=$(cat $sysdir/config/display/blueLightRGB)
    echo $currentRGB > $sysdir/config/display/blueLightRGBtemp

    setRGBValues "$value"
    endB=$endB
    endG=$endG
    endR=$endR

    newCombinedRGB=$(( (endR << 16) | (endG << 8) | endB ))
    echo $newCombinedRGB > $sysdir/config/display/blueLightRGB
    sync

    # echo ":: Blue Light Filter Intensity Set to $value, ready for next toggle." 
    if [ -f "$blf_key_on" ] || [ -f "$blf_key_on_user" ]; then
        touch $blf_key_on
        touch /tmp/blueLightIntensityChange
        blueLightStart
    fi
    rm $lockfile
}

disable_blue_light_filter() {   
    sync
    
    combinedBGR=$(cat $sysdir/config/display/blueLightRGB)
    combinedBGR=$(echo "$combinedBGR" | tr -d '[:space:]/#')

    echo $combinedBGR > $sysdir/config/display/blueLightRGBtemp

    lastR=$(( (combinedBGR >> 16) & 0xFF ))
    lastG=$(( (combinedBGR >> 8) & 0xFF ))
    lastB=$(( combinedBGR & 0xFF ))
    
    # echo "Last BGR: B: $lastB, G: $lastG, R: $lastR"
    # echo "Target BGR: B: 128, G: 128, R: 128"

    for i in $(seq 0 20); do
        newR=$(( lastR + (128 - lastR) * i / 20 ))
        newG=$(( lastG + (128 - lastG) * i / 20 ))
        newB=$(( lastB + (128 - lastB) * i / 20 ))

        echo "colortemp 0 0 0 0 $newB $newG $newR" > /proc/mi_modules/mi_disp/mi_disp0
        usleep 50000
    done

    echo ":: Blue Light Filter: Disabled"
    rm -f $blf_key_on
    rm -f $blf_key_on_user
}

check_disp_init() {
    if [ -z "$sysdir" ] || [ ! -x "$sysdir/bin/disp_init" ]; then
        echo "Error: disp_init not found or not executable"
        exit 
    fi
    
    if ! pgrep -f "/dev/l" > /dev/null || [ ! -e "/proc/mi_modules/mi_disp/mi_disp0" ]; then
        $sysdir/bin/disp_init &
        sleep 2.5
    fi
}

blueLightStart() {
    sync
    value=$(cat $sysdir/config/display/blueLightLevel)

    if [ -f $blf_key_on ]; then
        if [ -f /tmp/blueLightIntensityChange ]; then
            combinedRGB=$(cat $sysdir/config/display/blueLightRGBtemp)
        else
            combinedRGB=$(cat $sysdir/config/display/blueLightRGB)
        fi
    else
        combinedRGB="8421504"
    fi
    combinedRGB=$(echo "$combinedRGB" | tr -d '[:space:]/#')
    lastR=$(( (combinedRGB >> 16) & 0xFF ))
    lastG=$(( (combinedRGB >> 8) & 0xFF ))
    lastB=$(( combinedRGB & 0xFF ))

    setRGBValues "$value"
    
    # echo "Last BGR: B: $lastB, G: $lastG, R: $lastR"
    # echo "Target BGR: B: $endB, G: $endG, R: $endR"

    # Fading loop
    for i in $(seq 0 20); do
        newB=$(( lastB + (endB - lastB) * i / 20 ))
        newG=$(( lastG + (endG - lastG) * i / 20 ))
        newR=$(( lastR + (endR - lastR) * i / 20 ))

        echo "colortemp 0 0 0 0 $newB $newG $newR" > /proc/mi_modules/mi_disp/mi_disp0
        usleep 50000
    done

    newCombinedRGB=$(( (endR << 16) | (endG << 8) | endB ))
    echo $newCombinedRGB > $sysdir/config/display/blueLightRGB
    rm -f /tmp/blueLightIntensityChange
}


to_minutes_since_midnight() {
    hour=$(echo "$1" | cut -d: -f1 | xargs)
    minute=$(echo "$1" | cut -d: -f2 | xargs)
    hour=${hour:-0}
    minute=${minute:-0}
    echo "$hour $minute" | awk '{print $1 * 60 + $2}'
}

enable_blue_light_filter() {   
    check_disp_init
    sync
    blueLightStart
    
    echo ":: Blue Light Filter: Enabled"
    touch $blf_key_on
    touch $blf_key_on_user
    sync
}

disable_blue_light_filter() {    
    check_disp_init
    sync
    
    combinedBGR=$(cat $sysdir/config/display/blueLightRGB)
    combinedBGR=$(echo "$combinedBGR" | tr -d '[:space:]/#')

    lastR=$(( (combinedBGR >> 16) & 0xFF ))
    lastG=$(( (combinedBGR >> 8) & 0xFF ))
    lastB=$(( combinedBGR & 0xFF ))
    
    # echo "Last BGR: B: $lastB, G: $lastG, R: $lastR"
    # echo "Target BGR: B: 128, G: 128, R: 128"

    for i in $(seq 0 20); do
        newR=$(( lastR + (128 - lastR) * i / 20 ))
        newG=$(( lastG + (128 - lastG) * i / 20 ))
        newB=$(( lastB + (128 - lastB) * i / 20 ))

        echo "colortemp 0 0 0 0 $newB $newG $newR" > /proc/mi_modules/mi_disp/mi_disp0
        usleep 50000
    done
    
    echo ":: Blue Light Filter: Disabled"
    rm -f $blf_key_on
    rm -f $sysdir/config/.blfOn
}

check_blf() {
    sync
    if [ ! -f "$ignore_schedule" ]; then
        if [ ! -f "$sysdir/config/.ntpState" ]; then
            return
        fi
        
        blueLightTimeOnFile="$sysdir/config/display/blueLightTime"
        blueLightTimeOffFile="$sysdir/config/display/blueLightTimeOff"

        if [ ! -f "$blueLightTimeOnFile" ] || [ ! -f "$blueLightTimeOffFile" ]; then
            rm -f "$lockfile"
            return
        fi

        blueLightTimeOn=$(cat "$blueLightTimeOnFile")
        blueLightTimeOff=$(cat "$blueLightTimeOffFile")

        currentTime=$(date +"%H:%M")
        currentTimeMinutes=$(to_minutes_since_midnight "$currentTime")
        blueLightTimeOnMinutes=$(to_minutes_since_midnight "$blueLightTimeOn")
        blueLightTimeOffMinutes=$(to_minutes_since_midnight "$blueLightTimeOff")

        if [ -f "$blf_key" ]; then
            if [ "$blueLightTimeOffMinutes" -lt "$blueLightTimeOnMinutes" ]; then
                if [ "$currentTimeMinutes" -ge "$blueLightTimeOnMinutes" ] || [ "$currentTimeMinutes" -lt "$blueLightTimeOffMinutes" ]; then
                    if [ ! -f $blf_key_on ]; then
                        enable_blue_light_filter 
                        touch $blf_key_on
                    fi
                else
                    if [ -f $blf_key_on ]; then
                        disable_blue_light_filter 
                        rm $blf_key_on
                    fi
                fi
            else
                if [ "$currentTimeMinutes" -ge "$blueLightTimeOnMinutes" ] && [ "$currentTimeMinutes" -lt "$blueLightTimeOffMinutes" ]; then
                    if [ ! -f $blf_key_on ]; then
                        enable_blue_light_filter 
                        touch $blf_key_on
                    fi
                else
                    if [ -f $blf_key_on ]; then
                        disable_blue_light_filter 
                        rm $blf_key_on
                    fi
                fi
            fi
        fi
    fi
    rm -f "$lockfile"
}

set_default() {
    disable_blue_light_filter
    set_intensity 0
    rm -f "$ignore_schedule"
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
    set_intensity)
        set_intensity "$2"
        ;;
    set_default)
        set_default
        ;;
    *)
        echo "Usage: $0 {enable|disable|check|set_intensity [reset_arg]|set_default}"
        exit 1
        ;;
esac