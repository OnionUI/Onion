#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
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
	
	# Loop breaker for NTP
	touch /tmp/ntp_run_once

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

    if [ $deviceModel -eq 354 ] && [ -f /mnt/SDCARD/RetroArch/retroarch_miyoo354 ]; then
        # Mount miyoo354 RA version
        mount -o bind /mnt/SDCARD/RetroArch/retroarch_miyoo354 /mnt/SDCARD/RetroArch/retroarch
    fi

    # Bind arcade name library to customer path
    mount -o bind /mnt/SDCARD/miyoo/lib/libgamename.so /customer/lib/libgamename.so
	
	if [ -f "$sysdir/config/filebrowser/first.run" ]; then
		# Set filebrowser branding to "Onion" and apply custom theme
		$sysdir/bin/filebrowser config set --branding.name "Onion" -d $sysdir/config/filebrowser/filebrowser.db
		$sysdir/bin/filebrowser config set --branding.files "$sysdir/config/filebrowser/theme" -d $sysdir/config/filebrowser/filebrowser.db
		
		rm "$sysdir/config/filebrowser/first.run"
	fi

    start_networking
		
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
        echo "LD_PRELOAD=$miyoodir/lib/libpadsp.so ./retroarch -v" > $sysdir/cmd_to_run.sh
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

        check_networking
		
        state_change
        check_game_menu

        state_change
        check_game
        
		check_networking
		
        state_change
        check_switcher
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
        ./logs/keymon.log \
        ./logs/game_list_options.log \
        ./logs/network.log \
        ./logs/dnsmasq.log \
		./logs/ftp.log \
		./logs/ra_quick_host.log \
        2> /dev/null
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
    echo -e "\n:: Launch MainUI"

    cd $sysdir
    mainUiBatPerc

    check_hide_recents
    check_hide_expert

    wifi_setting=$(/customer/app/jsonval wifi)

    start_audioserver

    # MainUI launch
    cd $miyoodir/app
    PATH="$miyoodir/app:$PATH" \
    LD_LIBRARY_PATH="$miyoodir/lib:/config/lib:/lib" \
    LD_PRELOAD="$miyoodir/lib/libpadsp.so" \
    ./MainUI 2>&1 > /dev/null

    if [ $(/customer/app/jsonval wifi) -ne $wifi_setting ]; then
        touch /tmp/network_changed
        sync
    fi

    $sysdir/bin/freemma
    
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
    echo -e "\n\n:: GLO\n\n"

    cd $sysdir
    if [ -f ./config/.logging ]; then
        ./script/game_list_options.sh >> ./logs/game_list_options.log
    else
        ./script/game_list_options.sh
    fi

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
    echo -e "\n:: Launch game"
    cmd=`cat $sysdir/cmd_to_run.sh`

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
            temp=`cat $sysdir/cmd_to_run.sh`
            echo "$temp" | sed 's/\$/\\\$/g' > $sysdir/cmd_to_run.sh
        fi

        playActivity start "$rompath"
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
    echo -e "\n:: Launch switcher"
    cd $sysdir
    LD_PRELOAD="$miyoodir/lib/libpadsp.so" gameSwitcher
    rm $sysdir/.runGameSwitcher
    echo "switcher" > /tmp/prev_state
    sync
}

check_off_order() {
    if  [ -f /tmp/.offOrder ] ; then
        bootScreen "$1" &
        sleep 1			# Allow the bootScreen to be displayed
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
clean_flag=$miyoodir/app/.isClean
expert_flag=$miyoodir/app/.isExpert

check_hide_expert() {
    if [ ! -f $sysdir/config/.showExpert ]; then
        # Should be clean
        if [ ! -f $clean_flag ] || [ -f $expert_flag ] || [ $is_device_model_changed -eq 1 ] || [ ! -f $mainui_target ]; then
            rm -f $mainui_target 2> /dev/null
            rm -f $expert_flag 2> /dev/null
            cp "$sysdir/bin/MainUI-$deviceModel-clean" $mainui_target
            touch $clean_flag
        fi
    else
        # Should be expert
        if [ ! -f $expert_flag ] || [ -f $clean_flag ] || [ $is_device_model_changed -eq 1 ] || [ ! -f $mainui_target ]; then
            rm -f $mainui_target 2> /dev/null
            rm -f $clean_flag 2> /dev/null
            cp "$sysdir/bin/MainUI-$deviceModel-expert" $mainui_target
            touch $expert_flag
        fi
    fi
    sync
}


deviceModel=0
last_device_model=$miyoodir/app/lastDeviceModel
is_device_model_changed=0

check_device_model() {
    echo -e "\n:: Check device model"

    if axp 0; then
        touch /tmp/deviceModel
        printf "354" > /tmp/deviceModel
        deviceModel=354
    else
        touch /tmp/deviceModel
        printf "283" > /tmp/deviceModel
        deviceModel=283
    fi

    # Check if the SD is inserted in a different model
    is_device_model_changed=0
    if [ ! -f $last_device_model ]; then
        cp /tmp/deviceModel $last_device_model
        is_device_model_changed=1
    else
        lastDeviceModel=`cat $last_device_model`
        if [ $lastDeviceModel -ne $deviceModel ]; then
            is_device_model_changed=1
            echo $deviceModel > $last_device_model
        fi
    fi
}


init_system() {
    echo -e "\n:: Init system"

    # init_lcd
    cat /proc/ls
    sleep 0.25

    if [ $deviceModel -eq 354 ] && [ -f $sysdir/config/.lcdvolt ]; then
        $sysdir/script/lcdvolt.sh 2> /dev/null
    fi
    
    start_audioserver

    if [ $deviceModel -eq 283 ]; then
        # init charger detection
        if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
            echo 59 > /sys/class/gpio/export
            echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
        fi

        if [ $(/customer/app/jsonval vol) -ne 20 ] || [ $(/customer/app/jsonval mute) -ne 0 ]; then
            # Force volume and mute settings
            cat /appconfigs/system.json \
                | sed 's/^\s*"vol":\s*[0-9][0-9]*/\t"vol":\t20/g' \
                | sed 's/^\s*"mute":\s*[0-9][0-9]*/\t"mute":\t0/g' \
                > temp
            mv -f temp /appconfigs/system.json
        fi
    fi

    brightness=`/customer/app/jsonval brightness`
    brightness_raw=`awk "BEGIN { print int(3 * exp(0.350656 * $brightness) + 0.5) }"`
    echo "brightness: $brightness -> $brightness_raw"

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
        currentTime=`cat $timepath`
    fi
    #Add 4 hours to the current time
    hours=4
    if [ -f $sysdir/config/startup/addHours ]; then
        hours=`cat $sysdir/config/startup/addHours`
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
        startup_tab=`cat $sysdir/config/startup/tab`
    fi
    
    cd $sysdir
    setState "$startup_tab"
}

start_audioserver() {
    defvol=`echo $(/customer/app/jsonval vol) | awk '{ printf "%.0f\n", 48 * (log(1 + $1) / log(10)) - 60 }'`
    runifnecessary "audioserver" $miyoodir/app/audioserver $defvol
}

runifnecessary() {
    cnt=0
    #a=`ps | grep $1 | grep -v grep`
    a=`pgrep $1`
    while [ "$a" == "" ] && [ $cnt -lt 8 ] ; do
        echo try to run $2
        $2 $3 &
        sleep 0.5
        cnt=`expr $cnt + 1`
        a=`pgrep $1`
    done
}


start_networking() {
    rm $sysdir/config/.hotspotState  # dont start hotspot at boot
    
    touch /tmp/network_changed
    sync
    check_networking
}

check_networking() {
    if [ $deviceModel -ne 354 ] || [ ! -f /tmp/network_changed ]; then
        check_timezone
        return
    fi
	rm /tmp/network_changed

    $sysdir/script/network/update_networking.sh check
        
    check_timezone
}

check_timezone() {
	if [ -f /tmp/timezone_update ]; then
		export TZ=$(cat "$sysdir/config/.tz")
		rm /tmp/timezone_update
	fi
}

    
main