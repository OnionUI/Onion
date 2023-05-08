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

    touch /tmp/network_changed
    sync
	
 
    check_networking 
	check_tzid 
	write_tzid 
	check_ftpstate 
	check_sshstate 
	check_telnetstate 
	check_ntpstate 
	check_httpstate
	
	rm $sysdir/config/.HotspotState  # dont start hotspot at boot
	
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
	
	# Set filebrowser branding to onion
	CURRENT_DIR=$(pwd)
	cd /mnt/SDCARD/App/FileBrowser/
	/mnt/SDCARD/App/FileBrowser/filebrowser config set --branding.name "Onion" 
	/mnt/SDCARD/App/FileBrowser/filebrowser config set --branding.files "/mnt/SDCARD/App/FileBrowser/theme" 
	cd "$OLDPWD"


    # Main runtime loop
    while true; do
        state_change
        check_main_ui

		check_tzid 
		write_tzid  
        check_networking   
		check_ftpstate 
		check_sshstate 
		check_telnetstate 
		check_httpstate & 
		check_ntpstate &
              
        state_change
        check_game_menu

        state_change
        check_game

		check_tzid
		write_tzid
        
		check_ftpstate 
		check_sshstate 
		check_telnetstate 
		check_httpstate & 
		check_hotspotstate &
		check_ntpstate &
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
    PATH="$sysdir/script/redirect:$PATH" \
    LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib" \
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
    ./script/game_list_options.sh | tee -a ./logs/game_list_options.log

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
    echo -e "\n:: Launch switcher"
    cd $sysdir
    LD_PRELOAD="$miyoodir/lib/libpadsp.so" gameSwitcher
    rm $sysdir/.runGameSwitcher
    echo "switcher" > /tmp/prev_state
    sync
}

check_off_order() {
    if  [ -f /tmp/.offOrder ] ; then
        pkill -9 sshd
        pkill -9 wpa_supplicant
        pkill -9 udhcpc
        sync

        cd $sysdir
        bootScreen "$1" &

        # Allow the bootScreen to be displayed
        sleep 1.5

        if [ $deviceModel -eq 283 ]; then 
            reboot
        else
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

# Starts bftpd if the toggle is set to on
check_ftpstate() { 
    if [ ! -f $sysdir/config/.FTPState ]; then
        if pgrep bftpd > /dev/null ; then
			killall -9 bftpd 
			echo "$(date) FTP: Killed" >> $sysdir/logs/network.log
        else
            return
        fi
    else
        if pgrep bftpd > /dev/null ; then
			if [ $(/customer/app/jsonval wifi) -eq 0 ]; then
				echo "$(date) FTP: Wifi is turned off, disabling the toggle for FTP and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.FTPState
				killall -9 bftpd
			else
				return
			fi
        else
			if [ $(/customer/app/jsonval wifi) -eq 1 ]; then
				echo "$(date) FTP: Starting bftpd" >> $sysdir/logs/network.log
				/mnt/SDCARD/App/Ftp/bftpd -d -c /mnt/SDCARD/App/Ftp/bftpd.conf &
			else
				rm $sysdir/config/.FTPState
			fi
        fi
        
    fi
}

# Starts dropbear if the toggle is set to on
check_sshstate() { 
    if [ ! -f $sysdir/config/.SSHState ]; then
        if pgrep dropbear > /dev/null ; then
            killall -9 dropbear
			echo "$(date) Dropbear: Killed" >> $sysdir/logs/network.log
        else
            return
        fi
    else
        if pgrep dropbear > /dev/null ; then
			if [ $(/customer/app/jsonval wifi) -eq 0 ]; then 
				echo "$(date) Dropbear: Wifi is turned off, disabling the toggle for dropbear and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.SSHState
				killall -9 dropbear
			else
				return
			fi
        else
			if [ $(/customer/app/jsonval wifi) -eq 1 ]; then 
				echo "$(date) Dropbear: Starting dropbear" >> $sysdir/logs/network.log
				dropbear -R
			else
				rm $sysdir/config/.SSHState
			fi
        fi
    fi
}


# Starts telnet if the toggle is set to on
# Telnet is generally already running when you boot your MMP, you won't see this hit logs unless you bounce it
check_telnetstate() { 
    if [ ! -f $sysdir/config/.TelnetState ]; then
        if pgrep telnetd > /dev/null ; then
            killall -9 telnetd
			echo "$(date) Telnet: Killed" >> $sysdir/logs/network.log
        else
            return
        fi
    else
        if pgrep telnetd > /dev/null ; then
			if [ $(/customer/app/jsonval wifi) -eq 0 ]; then
				echo "$(date) Telnet: Wifi is turned off, disabling the toggle for Telnet and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.TelnetState
				killall -9 telnetd
			else
				return
			fi
        else
			if [ $(/customer/app/jsonval wifi) -eq 1 ]; then 
				echo "$(date) Telnet: Starting telnet" >> $sysdir/logs/network.log 
				cd /mnt/SDCARD 
				telnetd -l sh
			else
				rm $sysdir/config/.TelnetState
			fi
        fi
    fi
}

# Starts Filebrowser if the toggle in tweaks is set on
check_httpstate() { 
    if [ ! -f $sysdir/config/.HTTPState ]; then
        if pgrep filebrowser > /dev/null ; then
            killall -9 filebrowser
			echo "$(date) Filebrowser: Killed" >> $sysdir/logs/network.log
        else
            return
        fi
    else
        if pgrep filebrowser > /dev/null ; then
			if [ $(/customer/app/jsonval wifi) -eq 0 ]; then 
				echo "$(date) Filebrowser: Wifi is turned off, disabling the toggle for HTTP FS and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.HTTPState
				pkill -9 filebrowser
				return
			else
				return
			fi
        else
			# Checks if the toggle for WIFI is turned on.
			# Starts filebrowser bound to 0.0.0.0 so we don't need to mess around binding different IP's
			# This cuts down heavily on lag in the UI (as we don't need to run commands to check/grab IP's) and allows the menu to work more seamlessly
			if [ $(/customer/app/jsonval wifi) -eq 1 ]; then 
				cd /mnt/SDCARD/App/FileBrowser/
					  
																																			 
				/mnt/SDCARD/App/FileBrowser/filebrowser -p 80 -a 0.0.0.0 -r /mnt/SDCARD &
				echo "$(date) Filebrowser: Starting filebrowser listening on 0.0.0.0 to accept all traffic" >> $sysdir/logs/network.log
			else
				rm $sysdir/config/.HTTPState
			fi
        fi
    fi
}

# Starts the hotspot based on the results of check_hotspotstate, called twice saves repeating
# We have to sleep a bit or sometimes supllicant starts before we can get the hotspot logo
# Get the serial so we can use it for the hotspot password
# Check if the wpa pass is still set to the default pass, if it is change it to the serial number, if it's not then they've set a custom password, leave it alone.
# Starts AP and DHCP
# Turns off NTP as you wont be using it when you're on a hotspot
start_hotspot() { 
	if [ -f $sysdir/config/.NTPState ]; then
		touch /tmp/ntprestore
		rm $sysdir/config/.NTPState
	fi
	
	if pgrep hostapd >/dev/null; then
			echo "$(date) Hotspot: MainUI has taken wlan0 while we're supposed to be in AP mode, killing wpa_supp again." >> $sysdir/logs/network.log
		sleep 5
		pkill -9 wpa_supplicant 
		pkill -9 udhcpc 
	fi
	
	sleep 5 
	
	serial_number=$( { /config/riu_r 20 18 | awk 'NR==2'; /config/riu_r 20 17 | awk 'NR==2'; /config/riu_r 20 16 | awk 'NR==2'; } | sed 's/0x//g' | tr -d '[:space:]' ) 
	passphrase=$(grep '^wpa_passphrase=' "$sysdir/config/hostapd.conf" | cut -d'=' -f2)

	if [ "$passphrase" = "MiyooMiniApPassword" ]; then 
		sed -i "s/^wpa_passphrase=.*/wpa_passphrase=$serial_number/" "$sysdir/config/hostapd.conf"
		echo "$(date) Hotspot: Default key removed." >> $sysdir/logs/network.log
	fi

	pkill -9 wpa_supplicant 
	pkill -9 udhcpc 
	# Start AP and dhcp server
	ifconfig wlan0 up 
	$sysdir/bin/hostapd -P /var/run/hostapd.pid -B -i wlan0 $sysdir/config/hostapd.conf &
	hotspot0addr=$(grep -E '^dhcp-range=' "$sysdir/config/dnsmasq.conf" | cut -d',' -f1 | cut -d'=' -f2) 
	hotspot0addr=$(echo $hotspot0addr | awk -F'.' -v OFS='.' '{$NF-=1; print}') 
	gateway0addr=$(grep -E '^dhcp-option=3' $sysdir/config/dnsmasq.conf | awk -F, '{print $2}')
	subnetmask=$(grep -E '^dhcp-range=.*,[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+,' "$sysdir/config/dnsmasq.conf" | cut -d',' -f3) 
	ifconfig wlan0 $hotspot0addr netmask $subnetmask 
	ip route add default via $gateway0addr
	$sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root & 
	echo "$(date) Hotspot: Started with gateway of: $hotspot0addr, subnet of: $subnetmask" >> $sysdir/logs/network.log
}

# Starts personal hotspot if toggle is set to on
# Calls start_hotspot from above
# IF toggle is disabled, shuts down hotspot and bounces wifi.
# Restores NTP if it was on before we turned the hotspot on.
check_hotspotstate() { 
    if [ ! -f $sysdir/config/.HotspotState ]; then
        if pgrep hostapd >/dev/null; then
			echo "$(date) Hotspot: Killed" >> $sysdir/logs/network.log
			pkill -9 hostapd 
			pkill -9 dnsmasq
            ifconfig wlan0 up
            $miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf &
            udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
   
			if [ -f /tmp/ntprestore ]; then
				touch $sysdir/config/.NTPState
				sync
			fi
			
		else
			return
        fi
    else
        if pgrep hostapd >/dev/null; then
			if [ $(/customer/app/jsonval wifi) -eq 0 ]; then
				echo "$(date) Hotspot: Wifi is turned off, disabling the toggle for hotspot and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.HotspotState
				pkill -9 hostapd
				pkill -9 dnsmasq
			else
				# Hotspot is turned on, closing apps restarts the supp.. 
				# lets check if managed mode has taken over the adaptor before hotspot could grab it again, if it does we need to reset it for access & logos
				# Hotspot will come back up it just takes a little longer.
				sleep 10 
				if $sysdir/bin/iw dev wlan0 info | grep type | grep -q "type managed"; then
																														 
					start_hotspot &
				else
					return
				fi
			fi
        else
			if [ $(/customer/app/jsonval wifi) -eq 0 ]; then
				sed -i 's/"wifi":\s*0/"wifi": 1/' /appconfigs/system.json
				/customer/app/axp_test wifion
				sleep 2 
				ifconfig wlan0 up
				echo "$(date) Hotspot: Requested but WiFi is off, bringing WiFi up now." >> $sysdir/logs/network.log
				start_hotspot &
			else
				start_hotspot &
			fi
        fi
    fi
}

# Get the value of the tz set in tweaks
check_tzid() {
tzselect_file="$sysdir/config/tzselect"
tzid=$(cat "$tzselect_file") 
echo "$tzid"
}

# Check the value and write it into TZ var - if they look backwards its because they are, ntpd is weird..
write_tzid() {
check_tzid 
case $tzid in 0)export TZ="UTC+12";;1)export TZ="UTC+11";;2)export TZ="UTC+10";;3)export TZ="UTC+9";; \
4)export TZ="UTC+8";;5)export TZ="UTC+7";;6)export TZ="UTC+6";;7)export TZ="UTC+5";;8)export TZ="UTC+4";; \
9)export TZ="UTC+3";;10)export TZ="UTC+2";;11)export TZ="UTC+1";;12)export TZ="UTC";;13)export TZ="UTC-1";; \
14)export TZ="UTC-2";;15)export TZ="UTC-3";;16)export TZ="UTC-4";;17)export TZ="UTC-5";;18)export TZ="UTC-6";; \
19)export TZ="UTC-7";;20)export TZ="UTC-8";;21)export TZ="UTC-9";;22)export TZ="UTC-10";;23)export TZ="UTC-11";;
24)export TZ="UTC-12";;esac
}

# We need to check if NTP is enabled and then check the state of tzselect in /.tmp_update/config/tzselect, based on the value we'll pass the TZ via the env var to ntpd and get the time (has to be POSIX)
# This will work but it will not export the TZ var across all opens shells so you may find the hwclock (and clock app, retroarch time etc) are correct but terminal time is not.
# It does set TZ on the tty that Main is running in so this is ok
check_ntpstate() { 
	if [ ! -f $sysdir/config/.NTPState ]; then
        if pgrep ntpd > /dev/null ; then
            pkill -9 ntpd
			echo "$(date) NTP: Killed by request" >> $sysdir/logs/network.log
        else
            return
        fi
    else
        if pgrep ntpd > /dev/null; then
			if [ $(/customer/app/jsonval wifi) -eq 1 ]; then 
				export new_tz=$(check_tzid)
				if [ "$old_tz" != "$new_tz" ]; then
					pkill -9 ntpd
					echo "$(date)NTP: Killed, TZ has changed" >> "$sysdir/logs/network.log"
					check_tzid
					write_tzid
					ntpd -p time.google.com &
					sleep 1
					hwclock -w
					echo "$(date)NTP2: TZ set to $TZ, Time set to: $(date) and merged to hwclock, which shows: $(hwclock)" >> $sysdir/logs/network.log
					export old_tz=$(check_tzid)
				else 
					return
				fi
			else
				echo "$(date) NTP: Wifi is turned off, disabling the toggle for NTP and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.NTPState
				pkill -9 ntpd
			fi
        else
			if [ $(/customer/app/jsonval wifi) -eq 1 ]; then 
				pkill -9 ntpd
				check_tzid
				write_tzid
				echo "$(date) NTP: Starting NTP with TZ of $TZ" >> $sysdir/logs/network.log 
				ntpd -p time.google.com &
				sleep 1
				hwclock -w
				echo "$(date) NTP1: TZ set to $TZ, Time set to: $(date) and merged to hwclock, which shows: $(hwclock)" >> $sysdir/logs/network.log
				export old_tz=$(check_tzid)
			else
				echo "$(date) NTP: Wifi is turned off, disabling the toggle for NTP and killing the process" >> $sysdir/logs/network.log
				rm $sysdir/config/.NTPState
				pkill -9 ntpd
			fi
        fi
    fi
}


check_networking() {
    if [ ! -f /tmp/network_changed ]; then
        return
    fi
	rm /tmp/network_changed

    echo "$(date) Network Checker: Update networking" >> $sysdir/logs/network.log
	

	if [ $(/customer/app/jsonval wifi) -eq 1 ]; then
		if ! ifconfig wlan0 || [ -f /tmp/restart_wifi ]; then
			if [ -f /tmp/restart_wifi ]; then
				pkill -9 wpa_supplicant
				pkill -9 udhcpc
				rm /tmp/restart_wifi
			fi
 
			echo "$(date) Network Checker: Initializing Wifi..." >> $sysdir/logs/network.log
			/customer/app/axp_test wifion
			sleep 2 
			ifconfig wlan0 up
			$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
			udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
		fi
	else
		pkill -9 wpa_supplicant
		pkill -9 udhcpc
		/customer/app/axp_test wifioff
	fi

}

main