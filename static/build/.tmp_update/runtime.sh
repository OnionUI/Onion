#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

main() {
    check_device_model
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
    
    if [ $deviceModel -eq 283 ]; then 
        if [ `cat /sys/devices/gpiochip0/gpio/gpio59/value` -eq 1 ]; then
            cd $sysdir
            chargingState
        fi
    elif [ $deviceModel -eq 354 ]; then 
        cd /customer/app/
        batteryStatus=`./axp_test`
        case $batteryStatus in
        *\"charging\":3* ) 
            cd $sysdir
            chargingState
        esac
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

    detectKey 1
    menu_pressed=$?

    if [ $menu_pressed -eq 0 ]; then
        rm -f "$sysdir/cmd_to_run.sh" 2> /dev/null
    fi

    # Auto launch
    if [ ! -f $sysdir/config/.noAutoStart ]; then
        state_change
        check_game
    else
        rm -f "$sysdir/cmd_to_run.sh" 2> /dev/null
    fi

    startup_app=`cat $sysdir/config/startup/app`

    if [ $startup_app -eq 1 ]; then
        echo -e "\n\n:: STARTUP APP: GameSwitcher\n\n"
        touch $sysdir/.runGameSwitcher
    elif [ $startup_app -eq 2 ]; then
        echo -e "\n\n:: STARTUP APP: RetroArch\n\n"
        echo "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v" > $sysdir/cmd_to_run.sh
        touch /tmp/quick_switch
    elif [ $startup_app -eq 3 ]; then
        echo -e "\n\n:: STARTUP APP: AdvanceMENU\n\n"
        touch /tmp/run_advmenu
    fi

    state_change
    check_switcher
    set_startup_tab

    # Main runtime loop
    while true; do
        state_change
        check_main_ui
        state_change
        check_game_menu
        state_change
        check_game
        state_change  
        check_switcher
        
        # Free memory
        #/customer/app/sysmon freemma
        $sysdir/bin/freemma
    done
}

state_change() {
    touch /tmp/state_changed
    sync
}

clear_logs() {
    mkdir -p $sysdir/logs
    
    cd $sysdir
    rm -f \
        ./logs/MainUI.log \
        ./logs/gameSwitcher.log \
        ./logs/game_list_options.log
}

check_main_ui() {
    if [ ! -f $sysdir/cmd_to_run.sh ] ; then
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
    cd $sysdir
    mainUiBatPerc

    check_hide_recents
    check_hide_expert

    # MainUI launch
    cd /mnt/SDCARD/miyoo/app
    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" ./MainUI 2>&1 > /dev/null

    if [ $deviceModel -eq 354 ] && [ -f /tmp/mainui_killed ]; then
        pkill -9 sshd
        pkill -9 wpa_supplicant
        pkill -9 udhcpc
        rm /tmp/mainui_killed
    fi
    
    mv -f /tmp/cmd_to_run.sh $sysdir/cmd_to_run.sh
    
    echo "mainui" > /tmp/prev_state
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
    cd $sysdir
    echo -e "\n\n:: GLO\n\n"
    sync

    $sysdir/script/game_list_options.sh | tee -a ./logs/game_list_options.log

    if [ $? -ne 0 ]; then
        echo -e "\n\n< Back to MainUI\n\n"
        rm -f $sysdir/cmd_to_run.sh 2> /dev/null
        check_off_order "End"
    fi
}

check_game() {
    # Game launch
    if  [ -f $sysdir/cmd_to_run.sh ] ; then
        launch_game
    fi
}

check_is_game() {
    echo "$1" | grep -q "retroarch/cores" || echo "$1" | grep -q "/../../Roms/"
}

launch_game() {
    cmd=`cat $sysdir/cmd_to_run.sh`

    is_game=0
    rompath=""
    romext=""
    romcfgpath=""
    retroarch_core=""

    # TIMER BEGIN
    if check_is_game "$cmd"; then
        rompath=$(echo "$cmd" | awk '{ st = index($0,"\" \""); print substr($0,st+3,length($0)-st-3)}')

        if echo "$rompath" | grep -q ":"; then
            rompath=$(echo "$rompath" | awk '{split($0,a,":"); print a[2]}')
        fi

        romext=`echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}'`
        romcfgpath="$(dirname "$rompath")/.game_config/$(basename "$rompath" ".$romext").cfg"

        if [ "$romext" != "miyoocmd" ]; then
            echo "rompath: $rompath (ext: $romext)"
            echo "romcfgpath: $romcfgpath"
            is_game=1
        fi
    fi

    if [ $is_game -eq 1 ]; then
        if [ -f "$romcfgpath" ]; then
            romcfg=`cat "$romcfgpath"`
            retroarch_core=`get_info_value "$romcfg" core`
            corepath=".retroarch/cores/$retroarch_core.so"

            echo "per game core: $retroarch_core" >> $sysdir/logs/game_list_options.log

            if [ -f "$corepath" ]; then
                if echo "$cmd" | grep -q "$sysdir/reset.cfg"; then
                    echo "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v --appendconfig \"$sysdir/reset.cfg\" -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
                else
                    echo "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
                fi
            fi
        fi

        # Handle dollar sign
        if echo "$rompath" | grep -q "\$"; then
            temp=`cat $sysdir/cmd_to_run.sh`
            echo "$temp" | sed 's/\$/\\\$/g' > $sysdir/cmd_to_run.sh
        fi

        playActivity "init"
    fi

    # Prevent quick switch loop
    rm -f /tmp/quick_switch 2> /dev/null

    echo "----- COMMAND:"
    cat $sysdir/cmd_to_run.sh

    if [ "$romext" == "miyoocmd" ]; then
        if [ -f "$rompath" ]; then
            emupath=`dirname $(echo "$cmd" | awk '{ gsub(/"/, "", $2); st = index($2,".."); if (st) { print substr($2,0,st) } else { print $2 } }')`
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

    echo "cmd retval: $retval"

    if [ $retval -ge 128 ] && [ $retval -ne 143 ] && [ $retval -ne 255 ]; then
        cd $sysdir
        infoPanel --title "Fatal error occurred" --message "The program exited unexpectedly.\n(Error code: $retval)" --auto
    fi

    # Reset CPU frequency
    echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

    # Free memory
    $sysdir/bin/freemma

    # TIMER END + SHUTDOWN CHECK
    if [ $is_game -eq 1 ]; then
        if echo "$cmd" | grep -q "$sysdir/reset.cfg"; then
            echo "$cmd" | sed 's/ --appendconfig \"\/mnt\/SDCARD\/.tmp_update\/reset.cfg\"//g' > $sysdir/cmd_to_run.sh
        fi

        cd $sysdir
        playActivity "$cmd"
        
        echo "game" > /tmp/prev_state
        check_off_order "End_Save"
    else
        echo "app" > /tmp/prev_state
        check_off_order "End"
    fi
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,"="); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n'
}

check_switcher() {
    if [ -f $sysdir/.runGameSwitcher ] ; then
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
    cd $sysdir
    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" gameSwitcher
    rm $sysdir/.runGameSwitcher
    echo "switcher" > /tmp/prev_state
    sync
}

check_off_order() {
    if  [ -f /tmp/.offOrder ] ; then
        cd $sysdir
        bootScreen "$1"

        killall tee
        rm -f /mnt/SDCARD/update.log
                
        sync
        if [ $deviceModel -eq 283 ]; then 
            reboot
        else
            # Allow the bootScreen to be displayed
            sleep 1.5
            poweroff 
        fi
        sleep 10
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

clean_flag=/mnt/SDCARD/miyoo/app/.isClean
expert_flag=/mnt/SDCARD/miyoo/app/.isExpert

check_hide_expert() {
    if [ ! -f $sysdir/config/.showExpert ]; then
        # Should be clean
        if [ ! -f $clean_flag ] || [ -f $expert_flag ] || [ $is_device_model_changed -eq 1 ]; then
            rm /mnt/SDCARD/miyoo/app/MainUI
            rm -f $expert_flag
	        if [ $deviceModel -eq 283 ]; then 
                cp $sysdir/bin/MainUI-283-clean /mnt/SDCARD/miyoo/app/MainUI
            elif [ $deviceModel -eq 354 ]; then 
                cp $sysdir/bin/MainUI-354-clean /mnt/SDCARD/miyoo/app/MainUI
            fi
            touch $clean_flag
        fi
    else
        # Should be expert
        if [ ! -f $expert_flag ] || [ -f $clean_flag ] || [ $is_device_model_changed -eq 1 ]; then
            rm /mnt/SDCARD/miyoo/app/MainUI
            rm -f $clean_flag
	        if [ $deviceModel -eq 283 ]; then 
                cp $sysdir/bin/MainUI-283-expert /mnt/SDCARD/miyoo/app/MainUI
            elif [ $deviceModel -eq 354 ]; then 
                cp $sysdir/bin/MainUI-354-expert /mnt/SDCARD/miyoo/app/MainUI
            fi
            touch $expert_flag
        fi
    fi
    sync
}

check_device_model() {
    
    if [ ! -f /customer/app/axp_test ]; then        
        touch /tmp/deviceModel
        printf "283" > /tmp/deviceModel
        deviceModel=283
    else
        touch /tmp/deviceModel
        printf "354" > /tmp/deviceModel
        deviceModel=354
    fi
    
    # Check if the SD is inserted in a different model
    is_device_model_changed=0
    if [ ! -f /mnt/SDCARD/miyoo/app/lastDeviceModel ]; then
        cp /tmp/deviceModel /mnt/SDCARD/miyoo/app/lastDeviceModel
        is_device_model_changed=1
    else 
        lastDeviceModel=`cat /mnt/SDCARD/miyoo/app/lastDeviceModel` 
        if [ $lastDeviceModel -ne $deviceModel ]; then 
            is_device_model_changed=1
            rm /mnt/SDCARD/miyoo/app/lastDeviceModel
            echo $deviceModel > /mnt/SDCARD/miyoo/app/lastDeviceModel
        fi
    fi    
}


init_system() {
    # init_lcd
    cat /proc/ls
    sleep 0.25
    
    /mnt/SDCARD/miyoo/app/audioserver &

    if [ $deviceModel -eq 283 ]; then 
        # init charger detection
        if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
            echo 59 > /sys/class/gpio/export
            echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
        fi
    fi

    # init backlight
    echo 0 > /sys/class/pwm/pwmchip0/export
    echo 800 > /sys/class/pwm/pwmchip0/pwm0/period
    echo 80 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
    echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
}

update_time() {
    timepath=/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt
    currentTime=0
    # Load current time
    if [ -f $timepath ]; then
        currentTime=`cat $timepath`
    fi
    #Add 4 hours to the current time
    hours=4
    if [ -f $sysdir/config/startup/addHours ]; then
        hours=`cat $sysdir/config/startup/addHours`
    fi
    addTime=$(($hours * 3600))
    currentTime=$(($currentTime + $addTime))
    date +%s -s @$currentTime
}

set_startup_tab() {
    startup_tab=0
    if [ -f $sysdir/config/startup/tab ]; then
        startup_tab=`cat $sysdir/config/startup/tab`
    fi
    
    cd $sysdir
    setState "$startup_tab"
}

main
