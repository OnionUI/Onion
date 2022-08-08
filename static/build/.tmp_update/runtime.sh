#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$sysdir/lib"

main() {
    init_system
    update_time
    clear_logs
    
    is_charging=`cat /sys/devices/gpiochip0/gpio/gpio59/value`

    if [ $is_charging -eq 1 ]; then
        cd $sysdir
        ./bin/chargingState
    fi

    # Make sure MainUI doesn't show charging animation
    touch /tmp/no_charging_ui

    cd $sysdir
    ./bin/bootScreen "Boot"

    cd $sysdir
    ./bin/keymon 2>&1 > ./logs/keymon.log &

    # Init
    rm /tmp/.offOrder
    HOME=/mnt/SDCARD/RetroArch/

    # Auto launch
    if [ ! -f $sysdir/config/.noAutoStart ]; then
        check_game
    fi

    check_switcher

    # Main runtime loop
    while true; do
        check_main_ui
        check_game    
        check_switcher
        
        # Free memory
        $sysdir/bin/freemma
    done
}

clear_logs() {
    mkdir -p $sysdir/logs
    
    cd $sysdir
    rm -f \
        ./logs/mainUiBatPerc.log \
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
    ./bin/mainUiBatPerc 2>&1 >> ./logs/mainUiBatPerc.log

    check_hide_recents

    # MainUI launch
    cd /mnt/SDCARD/miyoo/app
    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" ./MainUI
    mv /tmp/cmd_to_run.sh $sysdir/cmd_to_run.sh
}

check_game() {
    # Game launch
    if  [ -f $sysdir/cmd_to_run.sh ] ; then
        launch_game
        check_off_order "End_Save"
    fi
}

launch_game() {
    # TIMER INIT 
    if  [ -f $sysdir/romName.txt ] ; then
        cd $sysdir
        ./bin/playActivity "init"
    fi

    # GAME LAUNCH
    cd /mnt/SDCARD/RetroArch/
    $sysdir/cmd_to_run.sh

    # TIMER SAVE
    if  [ -f $sysdir/romName.txt ] ; then
        cd $sysdir
        value=$(cat romName.txt);
        ./bin/playActivity "$value"    
    fi
}

check_switcher() {
    if  [ -f /tmp/.trimUIMenu ] ; then
        launch_switcher
    else 
        # Return to MainUI
        rm $sysdir/cmd_to_run.sh
        rm $sysdir/romName.txt
        sync
    fi
    
    check_off_order "End"
}

launch_switcher() {
    rm /tmp/.trimUIMenu
    cd $sysdir
    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" ./bin/gameSwitcher 2>&1 >> ./logs/gameSwitcher.log
    sync
}

check_off_order() {
    if  [ -f /tmp/.offOrder ] ; then
        cd $sysdir
        ./bin/bootScreen "$1"
        
        sync
        reboot
        sleep 10
    fi
}

recentlist=/mnt/SDCARD/Roms/recentlist.json
recentlist_hidden=/mnt/SDCARD/Roms/recentlist-hidden.json

check_hide_recents() {
    # Hide the recents tab by removing the json file
    if [ -f $recentlist ]; then
        cat $recentlist $recentlist_hidden > temp && mv temp $recentlist_hidden
        rm -f $recentlist
    fi
    # Restore recentlist 
    if [ -f $recentlist_hidden ] && [ ! -f $sysdir/config/.hideRecents ]; then
        mv -f $recentlist_hidden $recentlist
    fi
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

    # Start the battery monitor
    ./bin/batmon
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

main

