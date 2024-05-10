#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

logfile=$(basename "$0" .sh)
. $sysdir/script/log.sh

MODEL_MM=283
MODEL_MMP=354
screen_resolution="640x480"

main() {
    # Set model ID
    axp 0 > /dev/null
    export DEVICE_ID=$([ $? -eq 0 ] && echo $MODEL_MMP || echo $MODEL_MM)
    echo -n "$DEVICE_ID" > /tmp/deviceModel

    SERIAL_NUMBER=$(read_uuid)
    echo -n "$SERIAL_NUMBER" > /tmp/deviceSN

    touch /tmp/is_booting
    check_installer
    clear_logs

    init_system
    update_time

    # Remount passwd/group to add our own users
    mount -o bind $sysdir/config/passwd /etc/passwd
    mount -o bind $sysdir/config/group /etc/group

    # Start the battery monitor
    batmon &

    # Reapply theme
    system_theme="$(/customer/app/jsonval theme)"
    active_theme="$(cat $sysdir/config/active_theme)"

    if [ "$system_theme" == "./" ] || [ "$system_theme" != "$active_theme" ] || [ ! -d "$system_theme" ]; then
        themeSwitcher --reapply_icons
    fi

    # Check is charging
    if [ $DEVICE_ID -eq $MODEL_MM ]; then
        is_charging=$(cat /sys/devices/gpiochip0/gpio/gpio59/value)
    elif [ $DEVICE_ID -eq $MODEL_MMP ]; then
        axp_status="0x$(axp 0 | cut -d':' -f2)"
        is_charging=$([ $(($axp_status & 0x4)) -eq 4 ] && echo 1 || echo 0)
    fi

    # Show charging animation
    if [ $is_charging -eq 1 ]; then
        cd $sysdir
        chargingState
    fi

    # Make sure MainUI doesn't show charging animation
    touch /tmp/no_charging_ui

    # Check if blf needs enabling
    if [ -f $sysdir/config/.blfOn ]; then
        /mnt/SDCARD/.tmp_update/script/blue_light.sh enable &
    fi

    cd $sysdir
    bootScreen "Boot"

    # Set filebrowser branding to "Onion" and apply custom theme
    if [ -f "$sysdir/config/filebrowser/first.run" ]; then
        $sysdir/bin/filebrowser config set --branding.name "Onion" -d $sysdir/config/filebrowser/filebrowser.db
        $sysdir/bin/filebrowser config set --branding.files "$sysdir/config/filebrowser/theme" -d $sysdir/config/filebrowser/filebrowser.db

        rm "$sysdir/config/filebrowser/first.run"
    fi

    # Start networking (Checks networking, checks timezone)
    start_networking

    # Start the key monitor
    keymon &

    # Init
    rm /tmp/.offOrder 2> /dev/null
    HOME=/mnt/SDCARD/RetroArch/

    # Disable VNC server flag at boot
    if [ -f "$sysdir/config/.vncServer" ]; then
        rm "$sysdir/config/.vncServer"
    fi

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

    rm -rf /tmp/is_booting

    #EmuDeck - Startup scripts
    mkdir -p "$sysdir/startup"
    mkdir -p "$sysdir/checkoff"
    startup_scripts=$(find "$sysdir/startup" -type f -name "*.sh")

    for startup_script in $startup_scripts; do
        sh "$startup_script"
    done

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
    log "state change: $1"
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

    cd $sysdir/logs
    rm -f \
        ./MainUI.log \
        ./gameSwitcher.log \
        ./keymon.log \
        ./game_list_options.log \
        ./dnsmasq.log \
        ./ftp.log \
        ./runtime.log \
        ./update_networking.log \
        ./easy_netplay.log \
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

    mute_theme_bgm

    # MainUI launch
    cd $miyoodir/app
    PATH="$miyoodir/app:$PATH" \
        LD_LIBRARY_PATH="$miyoodir/lib:/config/lib:/lib" \
        LD_PRELOAD="$miyoodir/lib/libpadsp.so" \
        ./MainUI 2>&1 > /dev/null

    # Merge the last game launched into the recent list
    check_hide_recents

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
    echo "$1" | grep -q "retroarch/cores" || echo "$1" | grep -q "/../../Roms/" || echo "$1" | grep -q "/mnt/SDCARD/Roms/"
}

change_resolution() {
    res_x=""
    res_y=""

    if [ -n "$1" ]; then
        res_x=$(echo "$1" | cut -d 'x' -f 1)
        res_y=$(echo "$1" | cut -d 'x' -f 2)
    else
        res_x=$(echo "$screen_resolution" | cut -d 'x' -f 1)
        res_y=$(echo "$screen_resolution" | cut -d 'x' -f 2)
    fi
    log "Changing resolution to $res_x x $res_y"

    fbset -g "$res_x" "$res_y" "$res_x" "$((res_y * 2))" 32
    # inform batmon and keymon of resolution change
    killall -SIGUSR1 batmon
    killall -SIGUSR1 keymon
}

launch_game() {
    log "\n:: Launch game"
    cmd=$(cat $sysdir/cmd_to_run.sh)
    TZ_VALUE=$(cat "$sysdir/config/.tz")

    is_game=0
    rompath=""
    romext=""
    romcfgpath=""
    retroarch_core=""
    full_resolution_path=""

    start_audioserver
    save_settings

    if check_is_game "$cmd"; then
        rompath=$(echo "$cmd" | awk '{ st = index($0,"\" \""); print substr($0,st+3,length($0)-st-3)}')

        if echo "$rompath" | grep -q ":"; then
            launch=$(echo "$rompath" | awk '{split($0,a,":"); print a[1]}')
            rompath=$(echo "$rompath" | awk '{split($0,a,":"); print a[2]}')
            echo "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"$launch\" \"$rompath\"" > $sysdir/cmd_to_run.sh
        fi

        orig_path="$rompath"
        romext=$(echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}')

        if [ "$romext" != "miyoocmd" ]; then
            if [ -f "$rompath" ]; then
                rompath=$(realpath "$rompath")
            fi
            if [ "$rompath" != "$orig_path" ]; then
                temp=$(cat $sysdir/cmd_to_run.sh)
                cmd_replaced=$(echo "$temp" | rev | sed 's/^"[^"]*"//g' | rev)"\"$rompath\""
                echo "$cmd_replaced" > $sysdir/cmd_to_run.sh
            fi
            romcfgpath="$(dirname "$rompath")/.game_config/$(basename "$rompath" ".$romext").cfg"
            log "rompath: $rompath (ext: $romext)"
            log "romcfgpath: $romcfgpath"
            is_game=1
        fi
    fi

    full_resolution_path="$(get_full_resolution_path)"

    if [ $is_game -eq 1 ]; then
        if [ -f "$romcfgpath" ]; then
            override_game_core "$romcfgpath"
        fi

        # Handle dollar sign
        if echo "$rompath" | grep -q "\$"; then
            temp=$(cat $sysdir/cmd_to_run.sh)
            echo "$temp" | sed 's/\$/\\\$/g' > $sysdir/cmd_to_run.sh
        fi

        # Kill services for maximum performance
        if [ ! -f $sysdir/config/.keepServicesAlive ]; then
            for process in dropbear bftpd filebrowser telnetd smbd; do
                if is_running $process; then
                    killall -9 $process
                fi
            done
        fi

        playActivity start "$rompath"
    fi

    # Prevent quick switch loop
    rm -f /tmp/quick_switch 2> /dev/null

    log "----- COMMAND:"
    log "$(cat $sysdir/cmd_to_run.sh)"

    if [ $is_game -eq 0 ] || [ -f "$rompath" ]; then
        if [ "$romext" == "miyoocmd" ]; then
            /mnt/SDCARD/.tmp_update/script/remove_last_recent_entry.sh
            emupath=$(dirname $(echo "$cmd" | awk '{ gsub(/"/, "", $2); st = index($2,".."); if (st) { print substr($2,0,st) } else { print $2 } }'))
            cd "$emupath"
            chmod a+x "$rompath"
            "$rompath" "$rompath" "$emupath"
            retval=$?
        else
            # Change resolution if needed
            if [ -f /tmp/new_res_available ] && [ -f "$full_resolution_path" ]; then
                log "Found full_resolution file, changing resolution to 560p"
                change_resolution
            fi

            # Free memory
            $sysdir/bin/freemma

            # GAME LAUNCH
            cd /mnt/SDCARD/RetroArch/

            # make the cmd_to_run shell env aware of the new timezone
            TZ="$TZ_VALUE" $sysdir/cmd_to_run.sh
            retval=$?

            if [ -f /tmp/new_res_available ]; then
                # Restore resolution
                change_resolution "640x480"
            fi

            if [ $is_game -eq 1 ] && [ ! -f /tmp/.offOrder ] && [ -f /tmp/.displaySavingMessage ]; then
                rm /tmp/.displaySavingMessage
                infoPanel --title " " --message "Saving ..." --persistent --no-footer &
                touch /tmp/dismiss_info_panel
                sync
            fi
        fi
    else
        retval=404
    fi

    log "cmd retval: $retval"

    if [ $retval -eq 404 ]; then
        infoPanel --title "File not found" --message "The requested file was not found." --auto
    elif [ $retval -ge 128 ] && [ $retval -ne 143 ] && [ $retval -ne 255 ]; then
        infoPanel --title "Fatal error occurred" --message "The program exited unexpectedly.\n(Error code: $retval)" --auto
    fi

    launch_game_postprocess $is_game "$cmd" "$rompath"
}

override_game_core() {
    romcfgpath="$1"
    romcfg=$(cat "$romcfgpath")
    retroarch_core=$(get_info_value "$romcfg" core)
    corepath=".retroarch/cores/$retroarch_core.so"

    log "per game core: $retroarch_core" >> $sysdir/logs/game_list_options.log

    if [ -f "/mnt/SDCARD/RetroArch/$corepath" ] && echo "$cmd" | grep -qv "retroarch/cores"; then # Do not override game core when launching from GS
        if echo "$cmd" | grep -q "$sysdir/reset.cfg"; then
            echo "LD_PRELOAD=$miyoodir/lib/libpadsp.so ./retroarch -v --appendconfig \"$sysdir/reset.cfg\" -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
        else
            echo "LD_PRELOAD=$miyoodir/lib/libpadsp.so ./retroarch -v -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
        fi
    fi
}

launch_game_postprocess() {
    is_game=$1
    cmd="$2"
    rompath="$3"

    # Reset CPU frequency
    echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

    # Reset flags
    rm /tmp/stay_awake 2> /dev/null

    # TIMER END + SHUTDOWN CHECK
    if [ $is_game -eq 1 ]; then
        if echo "$cmd" | grep -q "$sysdir/reset.cfg"; then
            echo "$cmd" | sed 's/ --appendconfig \"\/mnt\/SDCARD\/.tmp_update\/reset.cfg\"//g' > $sysdir/cmd_to_run.sh
        fi

        cd $sysdir
        playActivity stop "$rompath"

        if [ -f /tmp/.lowBat ]; then
            bootScreen lowBat
            sleep 3
            touch /tmp/.offOrder
        fi

        # Reset networking if needed
        if [ ! -f "$sysdir/config/.keepServicesAlive" ]; then
            for service in smbd http ssh ftp telnet; do
                if [ -f "$sysdir/config/.${service}State" ]; then
                    touch /tmp/network_changed
                    break
                fi
            done
        fi

        set_prev_state "game"
        check_off_order "End_Save"
    else
        set_prev_state "app"
        check_off_order "End"
    fi
}

get_full_resolution_path() {
    if [ -f /tmp/new_res_available ]; then
        # Check if the program to be launched supports 560p
        # Different programs need different checks (Apps vs Ports vs the rest)
        if grep -qF "/mnt/SDCARD/App/" $sysdir/cmd_to_run.sh; then
            # ----- App launch ----- #

            echo "$(cat $sysdir/cmd_to_run.sh | cut -d' ' -f 2 | sed 's/;/\/full_resolution/')"

        elif grep -qF "/mnt/SDCARD/Roms/PORTS/" $sysdir/cmd_to_run.sh; then
            # ----- Port launch ----- #

            dot_port_path=$(grep -o '\/mnt\/SDCARD\/Roms\/PORTS.*\.port' $sysdir/cmd_to_run.sh)

            if grep -qF "FullResolution=1" "$dot_port_path"; then
                # Look for FullResolution=1 in the .port file
                # set full_resolution_path to a file that will always exist
                echo "/tmp/new_res_available"
            fi

        else
            # ----- Everything else ----- #
            echo "$(grep -o '".*launch\.sh"' $sysdir/cmd_to_run.sh | sed 's/"//g; s/launch\.sh/full_resolution/')"
        fi
    fi
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
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
    start_audioserver
    LD_PRELOAD="$miyoodir/lib/libpadsp.so" gameSwitcher
    rm $sysdir/.runGameSwitcher
    set_prev_state "switcher"
    sync
}

check_off_order() {
    if [ -f /tmp/.offOrder ]; then
        touch /tmp/shutting_down

        #EmuDeck - CheckOff scripts
        check_off_scripts=$(find "$sysdir/checkoff" -type f -name "*.sh")

        for check_off_script in $check_off_scripts; do
            sh "$check_off_script"
        done

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

#
#   attempts to retrieve the physical screen resolution from mi_fb module
#   resolution is stored in /tmp/screen_resolution
#   times out after 5 seconds, defaults to 640x480
#
get_screen_resolution() {
    max_attempts=10
    attempt=0

    log "get_screen_resolution: start"
    while [ "$attempt" -lt "$max_attempts" ]; do
        screen_resolution=$(grep 'Current TimingWidth=' /proc/mi_modules/fb/mi_fb0 | sed 's/Current TimingWidth=\([0-9]*\),TimingWidth=\([0-9]*\),.*/\1x\2/')
        if [ -n "$screen_resolution" ]; then
            log "get_screen_resolution: success, resolution: $screen_resolution"
            break
        fi
        log "get_screen_resolution: attempt $attempt failed"
        attempt=$((attempt + 1))
        sleep 0.5
    done

    if [ -z "$screen_resolution" ]; then
        log "get_screen_resolution: failed to get screen resolution, fall back to 640x480"
        touch /tmp/get_screen_resolution_failed
    fi

    if [ "$screen_resolution" = "752x560" ] && [ "$(/etc/fw_printenv miyoo_version)" = "miyoo_version=202310271401" ]; then
        touch /tmp/new_res_available
    else
        # can't use 752x560 without appropriate firmware or screen
        screen_resolution="640x480"
    fi

    echo -n "$screen_resolution" > /tmp/screen_resolution
}

mute_theme_bgm() {
    system_theme="$(/customer/app/jsonval theme)"
    bgm_file="${system_theme}sound/bgm.mp3"
    muted_bgm_file="${system_theme}sound/bgm_muted.mp3"

    if [ -f "$sysdir/config/.bgmMute" ]; then
        if [[ -f "$bgm_file" ]]; then
            mv -f "$bgm_file" "$muted_bgm_file"
        fi
    else
        if [[ -f "$muted_bgm_file" ]]; then
            mv -f "$muted_bgm_file" "$bgm_file"
        fi
    fi
}

create_swap() {
    swapfile="/mnt/SDCARD/cachefile"
    if [ ! -e "$swapfile" ]; then
        log "Creating swap file"
        dd if=/dev/zero of="$swapfile" bs=1M count=128
        mkswap "$swapfile"
    fi
    log "Enabling swap"
    swapon "$swapfile"
}

init_system() {
    log "\n:: Init system"

    create_swap
    load_settings

    # init_lcd
    cat /proc/ls
    sleep 0.25

    # setup loopback interface for RetroArch CMDs
    ip addr add 127.0.0.1/8 dev lo
    ifconfig lo up

    if [ $DEVICE_ID -eq $MODEL_MMP ] && [ -f $sysdir/config/.lcdvolt ]; then
        $sysdir/script/lcdvolt.sh 2> /dev/null
    fi

    start_audioserver

    brightness=$(/customer/app/jsonval brightness)
    brightness_raw=$(awk "BEGIN { print int(3 * exp(0.350656 * $brightness) + 0.5) }")
    log "brightness: $brightness -> $brightness_raw"

    # init backlight
    echo 0 > /sys/class/pwm/pwmchip0/export
    pwmfile="$sysdir/config/.pwmfrequency"
    if [ -s $pwmfile ]; then
        # 0 - 9 = 100 - 1000 Hz
        frequency=$((($(cat "$pwmfile") + 1) * 100))
    else
        frequency=800
    fi
    echo $frequency > /sys/class/pwm/pwmchip0/pwm0/period
    echo $brightness_raw > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
    echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable

    get_screen_resolution
}

device_uuid=$(read_uuid)
device_settings="/mnt/SDCARD/.tmp_update/config/system/$device_uuid.json"

load_settings() {
    if [ -f "$device_settings" ]; then
        cp -f "$device_settings" /mnt/SDCARD/system.json
    fi

    # make sure MainUI settings exist
    if [ ! -f /mnt/SDCARD/system.json ]; then
        if [ -f /appconfigs/system.json ]; then
            cp -f /appconfigs/system.json /mnt/SDCARD/system.json
        else
            cp -f $sysdir/res/miyoo${DEVICE_ID}_system.json /mnt/SDCARD/system.json
        fi
    fi

    # link /appconfigs/system.json to SD card
    if [ -L /appconfigs/system.json ] && [ "$(readlink /appconfigs/system.json)" == "/mnt/SDCARD/system.json" ]; then
        rm /appconfigs/system.json
    fi
    ln -s /mnt/SDCARD/system.json /appconfigs/system.json

    if [ $DEVICE_ID -eq $MODEL_MM ]; then
        # init charger detection
        if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
            echo 59 > /sys/class/gpio/export
            echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
        fi

        if [ $(/customer/app/jsonval vol) -ne 20 ] || [ $(/customer/app/jsonval mute) -ne 0 ]; then
            # Force volume and mute settings
            cat /mnt/SDCARD/system.json |
                sed 's/^\s*"vol":\s*[0-9][0-9]*/\t"vol":\t20/g' |
                sed 's/^\s*"mute":\s*[0-9][0-9]*/\t"mute":\t0/g' \
                    > temp
            mv -f temp /mnt/SDCARD/system.json
        fi
    fi

    default_volume="${sysdir}/config/.defaultVolume-${device_uuid}"
    if [ -f "$default_volume" ]; then
        volume=$(printf '%d' "$(cat "$default_volume")")
        if [ $? -eq 0 ]; then
            cat /mnt/SDCARD/system.json |
                sed 's/^\s*"vol":\s*[0-9][0-9]*/\t"vol":\t'$volume'/g' > temp
            mv -f temp /mnt/SDCARD/system.json
        fi
    fi
}

save_settings() {
    if [ -f /mnt/SDCARD/system.json ]; then
        cp -f /mnt/SDCARD/system.json "$device_settings"
    fi
}

update_time() {
    # Give hardware modders an option to disable time restore
    if [ -f $sysdir/config/.noTimeRestore ]; then
        return
    fi
    timepath=/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt
    currentTime=0
    # Load current time
    if [ -f $timepath ]; then
        currentTime=$(cat $timepath)
    fi
    date +%s -s @$currentTime

    # Ensure that all play activities are closed
    playActivity stop_all

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
    rm $sysdir/config/.hotspotState # dont start hotspot at boot

    touch /tmp/network_changed
    sync

    check_networking
}

check_networking() {
    if pgrep -f update_networking.sh; then
        log "update_networking already running"
    else
        if [ -f /tmp/network_changed ]; then
            rm /tmp/network_changed
        fi
        $sysdir/script/network/update_networking.sh check
    fi
}

check_installer() {
    # Check if installer is present
    if [ -d $miyoodir/app/.tmp_update ] && fgrep -q "#!/bin/sh" "$miyoodir/app/MainUI"; then
        echo "Installer detected!"
        cd $miyoodir/app
        ./MainUI
        reboot
        sleep 10
        exit
    fi
}

if [ -f $sysdir/config/.logging ]; then
    main
else
    main 2>&1 > /dev/null
fi
