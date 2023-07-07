#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

MODEL_MM=283
MODEL_MMP=354

main() {
    # Set model ID
    axp 0 > /dev/null
    export DEVICE_ID=$([ $? -eq 0 ] && echo $MODEL_MMP || echo $MODEL_MM)
    echo -n "$DEVICE_ID" > /tmp/deviceModel

    init_system
    update_time
    clear_logs

    # Start the battery monitor
    batmon &

    # Reapply theme
    system_theme="$(/customer/app/jsonval theme)"
    active_theme="$(cat ./config/active_theme)"

    if [ "$system_theme" == "./" ] || [ "$system_theme" != "$active_theme" ] || [ ! -d "$system_theme" ]; then
        themeSwitcher --reapply_icons
    fi

    # Check is charging
    if [ $DEVICE_ID -eq $MODEL_MM ]; then
        is_charging=$(cat /sys/devices/gpiochip0/gpio/gpio59/value)
    elif [ $DEVICE_ID -eq $MODEL_MMP ]; then
        is_charging=$(axp 0 | grep -q "value:c4" && echo 1 || echo 0)
    fi

    # Show charging animation
    if [ $is_charging -eq 1 ]; then
        cd $sysdir
        chargingState
    fi

    # Make sure MainUI doesn't show charging animation
    touch /tmp/no_charging_ui

    cd $sysdir
    bootScreen "Boot"

    # Start the key monitor
    keymon &

    # Init
    rm /tmp/.offOrder 2> /dev/null
    HOME=/mnt/SDCARD/RetroArch/

    # Detect if MENU button is held
    detectKey 1
    menu_pressed=$?

    if [ $menu_pressed -eq 0 ]; then
        rm -f "$sysdir/cmd_to_run.sh" 2> /dev/null
    fi

    if [ $DEVICE_ID -eq $MODEL_MMP ] && [ -f /mnt/SDCARD/RetroArch/retroarch_miyoo354 ]; then
        # Mount miyoo354 RA version
        mount -o bind /mnt/SDCARD/RetroArch/retroarch_miyoo354 /mnt/SDCARD/RetroArch/retroarch
    fi

    # Bind arcade name library to customer path
    mount -o bind $miyoodir/lib/libgamename.so /customer/lib/libgamename.so

    # Set filebrowser branding to "Onion" and apply custom theme
    if [ -f "$sysdir/config/filebrowser/first.run" ]; then
        $sysdir/bin/filebrowser config set --branding.name "Onion" -d $sysdir/config/filebrowser/filebrowser.db
        $sysdir/bin/filebrowser config set --branding.files "$sysdir/config/filebrowser/theme" -d $sysdir/config/filebrowser/filebrowser.db

        rm "$sysdir/config/filebrowser/first.run"
    fi

    start_networking

    # Auto launch
    if [ ! -f $sysdir/config/.noAutoStart ]; then
        state_change check_game
    else
        rm -f "$sysdir/cmd_to_run.sh" 2> /dev/null
    fi

    startup_app=$(cat $sysdir/config/startup/app)

    if [ $startup_app -eq 1 ]; then
        log "\n\n:: STARTUP APP: GameSwitcher\n\n"
        touch $sysdir/.runGameSwitcher
    elif [ $startup_app -eq 2 ]; then
        log "\n\n:: STARTUP APP: RetroArch\n\n"
        echo "LD_PRELOAD=$miyoodir/lib/libpadsp.so ./retroarch -v" > $sysdir/cmd_to_run.sh
        touch /tmp/quick_switch
    elif [ $startup_app -eq 3 ]; then
        log "\n\n:: STARTUP APP: AdvanceMENU\n\n"
        touch /tmp/run_advmenu
    fi

    state_change check_switcher
    set_startup_tab

    # Main runtime loop
    while true; do
        state_change check_main_ui
        state_change check_game_menu
        state_change check_game
        state_change check_switcher
    done
}

state_change() {
    runifnecessary "keymon" keymon
    check_networking
    touch /tmp/state_changed
    sync
    eval "$1"
}

set_prev_state() {
    echo "$1" > /tmp/prev_state
}

clear_logs() {
    mkdir -p $sysdir/logs

    cd $sysdir
    rm -f \
        ./logs/MainUI.log \
        ./logs/gameSwitcher.log \
        ./logs/keymon.log \
        ./logs/game_list_options.log \
        ./logs/network.log \
        ./logs/dnsmasq.log \
        ./logs/ftp.log \
        ./logs/ra_quick_host.log \
        2> /dev/null
}

check_main_ui() {
    if [ ! -f $sysdir/cmd_to_run.sh ]; then
        if [ -f /tmp/run_advmenu ]; then
            rm /tmp/run_advmenu
            $sysdir/bin/adv/run_advmenu.sh
        else
            launch_main_ui
        fi

        check_off_order "End"
    fi
}

launch_main_ui() {
    log "\n:: Launch MainUI"

    cd $sysdir

    # Generate battery percentage image
    mainUiBatPerc

    # Hide any new recents if applicable
    check_hide_recents

    # Ensure we've mounted the correct MainUI binary
    mount_main_ui

    # Wifi state before
    wifi_setting=$(/customer/app/jsonval wifi)

    start_audioserver

    # MainUI launch
    cd $miyoodir/app
    PATH="$miyoodir/app:$PATH" \
        LD_LIBRARY_PATH="$miyoodir/lib:/config/lib:/lib" \
        LD_PRELOAD="$miyoodir/lib/libpadsp.so" \
        ./MainUI 2>&1 > /dev/null

    # Check if wifi setting changed
    if [ $(/customer/app/jsonval wifi) -ne $wifi_setting ]; then
        touch /tmp/network_changed
        rm /tmp/ntp_synced 2> /dev/null
        sync
    fi

    $sysdir/bin/freemma

    mv -f /tmp/cmd_to_run.sh $sysdir/cmd_to_run.sh

    set_prev_state "mainui"
}

check_game_menu() {
    if [ ! -f /tmp/launch_alt ]; then
        return
    fi

    rm -f /tmp/launch_alt

    if [ ! -f $sysdir/cmd_to_run.sh ]; then
        return
    fi

    launch_game_menu
}

launch_game_menu() {
    log "\n\n:: GLO\n\n"

    cd $sysdir
    ./script/game_list_options.sh

    if [ $? -ne 0 ]; then
        log "\n\n< Back to MainUI\n\n"
        rm -f $sysdir/cmd_to_run.sh 2> /dev/null
        check_off_order "End"
    fi
}

check_game() {
    # Game launch
    if [ -f $sysdir/cmd_to_run.sh ]; then
        launch_game
    fi
}

check_is_game() {
    echo "$1" | grep -q "retroarch/cores" || echo "$1" | grep -q "/../../Roms/"
}

launch_game() {
    log "\n:: Launch game"
    cmd=$(cat $sysdir/cmd_to_run.sh)

    is_game=0
    rompath=""
    romext=""
    romcfgpath=""
    retroarch_core=""

    start_audioserver

    # TIMER BEGIN
    if check_is_game "$cmd"; then
        rompath=$(echo "$cmd" | awk '{ st = index($0,"\" \""); print substr($0,st+3,length($0)-st-3)}')

        if echo "$rompath" | grep -q ":"; then
            rompath=$(echo "$rompath" | awk '{split($0,a,":"); print a[2]}')
        fi

        romext=$(echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}')
        romcfgpath="$(dirname "$rompath")/.game_config/$(basename "$rompath" ".$romext").cfg"

        if [ "$romext" != "miyoocmd" ]; then
            log "rompath: $rompath (ext: $romext)"
            log "romcfgpath: $romcfgpath"
            is_game=1
        fi
    fi

    if [ $is_game -eq 1 ]; then
        if [ -f "$romcfgpath" ]; then
            romcfg=$(cat "$romcfgpath")
            retroarch_core=$(get_info_value "$romcfg" core)
            corepath=".retroarch/cores/$retroarch_core.so"

            log "per game core: $retroarch_core" >> $sysdir/logs/game_list_options.log

            if [ -f "/mnt/SDCARD/RetroArch/$corepath" ]; then
                if echo "$cmd" | grep -q "$sysdir/reset.cfg"; then
                    echo "LD_PRELOAD=$miyoodir/lib/libpadsp.so ./retroarch -v --appendconfig \"$sysdir/reset.cfg\" -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
                else
                    echo "LD_PRELOAD=$miyoodir/lib/libpadsp.so ./retroarch -v -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
                fi
            fi
        fi

        # Handle dollar sign
        if echo "$rompath" | grep -q "\$"; then
            temp=$(cat $sysdir/cmd_to_run.sh)
            echo "$temp" | sed 's/\$/\\\$/g' > $sysdir/cmd_to_run.sh
        fi

        playActivity start "$rompath"
    fi

    # Prevent quick switch loop
    rm -f /tmp/quick_switch 2> /dev/null

    log "----- COMMAND:"
    log "$(cat $sysdir/cmd_to_run.sh)"

    if [ "$romext" == "miyoocmd" ]; then
        if [ -f "$rompath" ]; then
            emupath=$(dirname $(echo "$cmd" | awk '{ gsub(/"/, "", $2); st = index($2,".."); if (st) { print substr($2,0,st) } else { print $2 } }'))
            cd "$emupath"

            chmod a+x "$rompath"
            "$rompath" "$rompath" "$emupath"
            retval=$?
        else
            retval=1
        fi
    else
        # GAME LAUNCH
        cd /mnt/SDCARD/RetroArch/
        $sysdir/cmd_to_run.sh
        retval=$?
    fi

    log "cmd retval: $retval"

    if [ $retval -ge 128 ] && [ $retval -ne 143 ] && [ $retval -ne 255 ]; then
        cd $sysdir
        infoPanel --title "Fatal error occurred" --message "The program exited unexpectedly.\n(Error code: $retval)" --auto
    fi

    # Reset CPU frequency
    echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

    # Reset flags
    rm /tmp/stay_awake 2> /dev/null

    # Free memory
    $sysdir/bin/freemma

    # TIMER END + SHUTDOWN CHECK
    if [ $is_game -eq 1 ]; then
        if echo "$cmd" | grep -q "$sysdir/reset.cfg"; then
            echo "$cmd" | sed 's/ --appendconfig \"\/mnt\/SDCARD\/.tmp_update\/reset.cfg\"//g' > $sysdir/cmd_to_run.sh
        fi

        cd $sysdir
        playActivity stop "$rompath"

        set_prev_state "game"
        check_off_order "End_Save"
    else
        set_prev_state "app"
        check_off_order "End"
    fi
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,"="); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n'
}

check_switcher() {
    if [ -f $sysdir/.runGameSwitcher ]; then
        launch_switcher
    elif [ -f /tmp/quick_switch ]; then
        # Quick switch
        rm -f /tmp/quick_switch
    else
        # Return to MainUI
        rm $sysdir/cmd_to_run.sh 2> /dev/null
        sync
    fi

    check_off_order "End"
}

launch_switcher() {
    log "\n:: Launch switcher"
    cd $sysdir
    LD_PRELOAD="$miyoodir/lib/libpadsp.so" gameSwitcher
    rm $sysdir/.runGameSwitcher
    set_prev_state "switcher"
    sync
}

check_off_order() {
    if [ -f /tmp/.offOrder ]; then
        bootScreen "$1" &
        sleep 1 # Allow the bootScreen to be displayed
        shutdown
    fi
}

recentlist=/mnt/SDCARD/Roms/recentlist.json
recentlist_hidden=/mnt/SDCARD/Roms/recentlist-hidden.json
recentlist_temp=/tmp/recentlist-temp.json

check_hide_recents() {
    # Hide recents on
    if [ ! -f $sysdir/config/.showRecents ]; then
        # Hide recents by removing the json file
        if [ -f $recentlist ]; then
            cat $recentlist $recentlist_hidden > $recentlist_temp
            mv -f $recentlist_temp $recentlist_hidden
            rm -f $recentlist
        fi
        # Hide recents off
    else
        # Restore recentlist
        if [ -f $recentlist_hidden ]; then
            cat $recentlist $recentlist_hidden > $recentlist_temp
            mv -f $recentlist_temp $recentlist
            rm -f $recentlist_hidden
        fi
    fi
    sync
}

mainui_target=$miyoodir/app/MainUI

mount_main_ui() {
    mainui_mode=$([ -f $sysdir/config/.showExpert ] && echo "expert" || echo "clean")
    mainui_srcname="MainUI-$DEVICE_ID-$mainui_mode"
    mainui_mount=$(basename "$(cat /proc/self/mountinfo | grep $mainui_target | cut -d' ' -f4)")

    if [ "$mainui_mount" != "$mainui_srcname" ]; then
        if mount | grep -q "$mainui_target"; then
            umount $mainui_target 2> /dev/null
        fi

        if [ ! -f $mainui_target ]; then
            touch $mainui_target
        fi

        mount -o bind "$sysdir/bin/$mainui_srcname" $mainui_target
    fi
}

init_system() {
    log "\n:: Init system"

    # init_lcd
    cat /proc/ls
    sleep 0.25

    if [ $DEVICE_ID -eq $MODEL_MMP ] && [ -f $sysdir/config/.lcdvolt ]; then
        $sysdir/script/lcdvolt.sh 2> /dev/null
    fi

    start_audioserver

    if [ $DEVICE_ID -eq $MODEL_MM ]; then
        # init charger detection
        if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
            echo 59 > /sys/class/gpio/export
            echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
        fi

        if [ $(/customer/app/jsonval vol) -ne 20 ] || [ $(/customer/app/jsonval mute) -ne 0 ]; then
            # Force volume and mute settings
            cat /appconfigs/system.json |
                sed 's/^\s*"vol":\s*[0-9][0-9]*/\t"vol":\t20/g' |
                sed 's/^\s*"mute":\s*[0-9][0-9]*/\t"mute":\t0/g' \
                    > temp
            mv -f temp /appconfigs/system.json
        fi
    fi

    brightness=$(/customer/app/jsonval brightness)
    brightness_raw=$(awk "BEGIN { print int(3 * exp(0.350656 * $brightness) + 0.5) }")
    log "brightness: $brightness -> $brightness_raw"

    # init backlight
    echo 0 > /sys/class/pwm/pwmchip0/export
    echo 800 > /sys/class/pwm/pwmchip0/pwm0/period
    echo $brightness_raw > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
    echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
}

update_time() {
    timepath=/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt
    currentTime=0
    # Load current time
    if [ -f $timepath ]; then
        currentTime=$(cat $timepath)
    fi
    #Add 4 hours to the current time
    hours=4
    if [ -f $sysdir/config/startup/addHours ]; then
        hours=$(cat $sysdir/config/startup/addHours)
    fi
    addTime=$(($hours * 3600))
    if [ ! -f $sysdir/config/.ntpState ]; then
        currentTime=$(($currentTime + $addTime))
    fi
    date +%s -s @$currentTime
}

set_startup_tab() {
    startup_tab=0
    if [ -f $sysdir/config/startup/tab ]; then
        startup_tab=$(cat $sysdir/config/startup/tab)
    fi

    cd $sysdir
    setState "$startup_tab"
}

start_audioserver() {
    defvol=$(echo $(/customer/app/jsonval vol) | awk '{ printf "%.0f\n", 48 * (log(1 + $1) / log(10)) - 60 }')
    runifnecessary "audioserver" $miyoodir/app/audioserver $defvol
}

runifnecessary() {
    cnt=0
    #a=`ps | grep $1 | grep -v grep`
    a=$(pgrep $1)
    while [ "$a" == "" ] && [ $cnt -lt 8 ]; do
        log "try to run: $2"
        $2 $3 &
        sleep 0.5
        cnt=$(expr $cnt + 1)
        a=$(pgrep $1)
    done
}

start_networking() {
    # Loop breaker for NTP
    touch /tmp/ntp_run_once

    rm $sysdir/config/.hotspotState # dont start hotspot at boot

    touch /tmp/network_changed
    sync
}

check_networking() {
    if [ $DEVICE_ID -ne $MODEL_MMP ] || [ ! -f /tmp/network_changed ]; then
        check_timezone
        return
    fi
    rm /tmp/network_changed

    $sysdir/script/network/update_networking.sh check

    check_timezone
}

check_timezone() {
    export TZ=$(cat "$sysdir/config/.tz")
}

scriptname=$(basename "$0" .sh)

log() {
    if [ -f $sysdir/config/.logging ]; then
        echo -e "($scriptname) $(date):" $* | tee -a "$sysdir/logs/$scriptname.log"
    fi
}

if [ -f $sysdir/config/.logging ]; then
    main
else
    main 2>&1 > /dev/null
fi
