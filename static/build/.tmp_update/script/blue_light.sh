sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo 
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
blf_key=$sysdir/config/.blf

sync
lockfile="/tmp/blue_light_script.lock"

if [ -f "$lockfile" ]; then
    # echo "Another instance of the script is already running."
    exit 1
fi

touch "$lockfile"
trap 'rm -f "$lockfile"; exit' INT TERM EXIT

setRGBValues() {
    value=$1
    case "$value" in
        0)
            endB=128; endG=128; endR=128 ;;
        1)
            endB=110; endG=125; endR=140 ;;
        2)
            endB=100; endG=120; endR=140 ;;
        3)
            endB=90;  endG=115; endR=140 ;;
        4)
            endB=80;  endG=110; endR=140 ;;
        5)
            endB=70;  endG=105; endR=140 ;;
        *)
            endB=128; endG=128; endR=128 ;;
    esac
}

set_intensity() {
    sync
    value=$(cat $sysdir/config/display/blueLightLevel)

    setRGBValues "$value"
    endB=$endB
    endG=$endG
    endR=$endR

    newCombinedRGB=$(( (endR << 16) | (endG << 8) | endB ))
    echo $newCombinedRGB > $sysdir/config/display/blueLightRGB

    echo ":: Blue Light Filter Intensity Set to $value, ready for next toggle."
    if [ -f "$sysdir/config/.blf" ]; then
        touch /tmp/blueLightIntensityChange
    fi
}

disable_blue_light_filter() {

    if [ -f /tmp/runningBLF ]; then
        echo "Another instance of the script is already running."
        exit 1
    fi  
    
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
    rm -f /tmp/blueLightOn
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
    
    if [ ! -f /tmp/blueLightOn ] && [ ! -f /tmp/blueLightIntensityChange ]; then
        lastR=128
        lastG=128
        lastB=128
    elif [ -f /tmp/blueLightIntensityChange ]; then
        combinedRGB=$(cat $sysdir/config/display/blueLightRGBtemp)
        combinedRGB=$(echo "$combinedRGB" | tr -d '[:space:]/#')
        lastR=$(( (combinedRGB >> 16) & 0xFF ))
        lastG=$(( (combinedRGB >> 8) & 0xFF ))
        lastB=$(( combinedRGB & 0xFF ))
    else
        combinedRGB=$(cat $sysdir/config/display/blueLightRGB)
        combinedRGB=$(echo "$combinedRGB" | tr -d '[:space:]/#')
        lastR=$(( (combinedRGB >> 16) & 0xFF ))
        lastG=$(( (combinedRGB >> 8) & 0xFF ))
        lastB=$(( combinedRGB & 0xFF ))
    fi

    setRGBValues "$value"
    endB=$endB
    endG=$endG
    endR=$endR
    
    # echo "Last BGR: B: $lastB, G: $lastG, R: $lastR"
    # echo "Target BGR: B: $endB, G: $endG, R: $endR"

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
    value=$(cat $sysdir/config/display/blueLightLevel)
    if [ "$value" -eq 0 ]; then
        return
    fi
    
    check_disp_init
    sync
    blueLightStart
    
    echo ":: Blue Light Filter: Enabled"
    touch /tmp/blueLightOn
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
    rm -f /tmp/blueLightOn
}

check_blf() {
    sync
    
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
    fi

    rm -f "$lockfile"
}


case "$1" in
    enable)
        touch /tmp/runningBLF
        enable_blue_light_filter
        rm -rf /tmp/runningBLF
        ;;
    disable)
        touch /tmp/runningBLF
        disable_blue_light_filter
        rm -rf /tmp/runningBLF
        ;;
    check)
        touch /tmp/runningBLF
        check_blf
        rm -rf /tmp/runningBLF
        ;;
    set_intensity)
        set_intensity
        ;;
    *)
        echo "Usage: $0 {enable|disable|check|set_intensity}"
        exit 1
        ;;
esac