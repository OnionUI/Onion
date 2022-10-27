#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$sysdir/lib"

main() {
    init_system
    update_time
    clear_logs

    # Start the battery monitor
    ./bin/batmon &
    
    if [ `cat /sys/devices/gpiochip0/gpio/gpio59/value` -eq 1 ]; then
        cd $sysdir
        ./bin/chargingState
    fi

    # Make sure MainUI doesn't show charging animation
    touch /tmp/no_charging_ui

    cd $sysdir
    ./bin/bootScreen "Boot"

    # Start the key monitor
    ./bin/keymon &

    # Init
    rm /tmp/.offOrder
    HOME=/mnt/SDCARD/RetroArch/

    # Auto launch
    if [ ! -f $sysdir/config/.noAutoStart ]; then
        state_change
        check_game
    fi

    startup_app=`cat $sysdir/config/startup/app`

    if [ $startup_app -eq 1 ]; then
        touch $sysdir/.runGameSwitcher
    elif [ $startup_app -eq 2 ]; then
        cd /mnt/SDCARD/RetroArch/
        LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v
    fi

    state_change
    check_switcher

    set_startup_tab

    # Main runtime loop
    while true; do
        state_change
        check_main_ui
        state_change
        check_game
        state_change  
        check_switcher
        
        # Free memory
        $sysdir/bin/freemma
    done
}

state_change() {
    touch /tmp/state_changed
}

clear_logs() {
    mkdir -p $sysdir/logs
    
    cd $sysdir
    rm -f \
        ./logs/MainUI.log \
        ./logs/gameSwitcher.log
}

check_main_ui() {
    if [ ! -f $sysdir/cmd_to_run.sh ] ; then
        launch_main_ui
        check_off_order "End"
    fi
}

launch_main_ui() {
    cd $sysdir
    ./bin/mainUiBatPerc

    check_hide_recents
    check_hide_expert

    # MainUI launch
    cd /mnt/SDCARD/miyoo/app
    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" ./MainUI 2>&1 > /dev/null
    mv /tmp/cmd_to_run.sh $sysdir/cmd_to_run.sh

    echo "mainui" > /tmp/prev_state
}

check_game() {
    # Game launch
    if  [ -f $sysdir/cmd_to_run.sh ] ; then
        launch_game
    fi
}

launch_game() {
    romfile=`cat $sysdir/cmd_to_run.sh`
    is_game=0

    if echo "$romfile" | grep -q "retroarch" || echo "$romfile" | grep -q "/mnt/SDCARD/Emu/" || echo "$romfile" | grep -q "/mnt/SDCARD/RApp/"; then
        if ! echo "$romfile" | grep -q "/mnt/SDCARD/Emu/SEARCH/../../App/Search"; then
            echo "Game found:" $(basename "$romfile")
            is_game=1
        fi
    fi

    # TIMER BEGIN
    if [ $is_game -eq 1 ]; then
        cd $sysdir
        ./bin/playActivity "init"
    fi

    # GAME LAUNCH
    cd /mnt/SDCARD/RetroArch/
    $sysdir/cmd_to_run.sh

    # TIMER END + SHUTDOWN CHECK
    if [ $is_game -eq 1 ]; then
        cd $sysdir
        ./bin/playActivity "$romfile"
        
        echo "game" > /tmp/prev_state        
        check_off_order "End_Save"
    else
        echo "app" > /tmp/prev_state
        check_off_order "End"
    fi
}

check_switcher() {
    if [ -f $sysdir/.runGameSwitcher ] ; then
        launch_switcher
    elif [ -f /tmp/quick_switch ]; then
        # Quick switch
        rm -f /tmp/quick_switch
    else
        # Return to MainUI
        rm $sysdir/cmd_to_run.sh
        sync
    fi
    
    check_off_order "End"
}

launch_switcher() {
    cd $sysdir
    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" ./bin/gameSwitcher
    rm $sysdir/.runGameSwitcher
    echo "switcher" > /tmp/prev_state
    sync
}

check_off_order() {
    if  [ -f /tmp/.offOrder ] ; then
        cd $sysdir
        ./bin/bootScreen "$1"

        killall tee
        rm -f /mnt/SDCARD/update.log
        
        sync
        reboot
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
        if [ ! -f $clean_flag ] || [ -f $expert_flag ]; then
            rm /mnt/SDCARD/miyoo/app/MainUI
            rm -f $expert_flag
	        cp $sysdir/bin/MainUI-clean /mnt/SDCARD/miyoo/app/MainUI
            touch $clean_flag
        fi
    else
        # Should be expert
        if [ ! -f $expert_flag ] || [ -f $clean_flag ]; then
            rm /mnt/SDCARD/miyoo/app/MainUI
            rm -f $clean_flag
	        cp $sysdir/bin/MainUI-expert /mnt/SDCARD/miyoo/app/MainUI
            touch $expert_flag
        fi
    fi
    sync
}

init_system() {
    # init_lcd
    cat /proc/ls
    sleep 0.25
    
    /mnt/SDCARD/miyoo/app/audioserver &

    # init charger detection
    if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
        echo 59 > /sys/class/gpio/export
        echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
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
    ./bin/setState "$startup_tab"
}

main

