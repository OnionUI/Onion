#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo

update() {
    log "Network Checker: Update networking"

    check_wifi
    check_ftpstate 
    check_sshstate 
    check_telnetstate 
    check_ntpstate
    check_httpstate
	check_hotspotstate
}


check_wifi() {
if is_running hostapd; then
	return
else
	if wifi_enabled; then
		if ! ifconfig wlan0 || [ -f /tmp/restart_wifi ]; then
			if [ -f /tmp/restart_wifi ]; then
				pkill -9 wpa_supplicant
				pkill -9 udhcpc
				rm /tmp/restart_wifi
			fi

			log "Network Checker: Initializing wifi..."

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
fi
}


# Starts bftpd if the toggle is set to on
check_ftpstate() { 
    if flag_enabled FTPState; then
        if is_running bftpd; then
            if wifi_disabled; then
                log "FTP: Wifi is turned off, disabling the toggle for FTP and killing the process"
                disable_flag FTPState
                killall -9 bftpd
            fi
        else
            if wifi_enabled; then
                log "FTP: Starting bftpd"
                bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf &
            else
                disable_flag FTPState
            fi
        fi
    else
        if is_running bftpd; then
            killall -9 bftpd 
            log "FTP: Killed"
        fi
    fi
}

# Starts dropbear if the toggle is set to on
check_sshstate() { 
    if flag_enabled SSHState; then
        if is_running dropbear; then
            if wifi_disabled; then 
                log "Dropbear: Wifi is turned off, disabling the toggle for dropbear and killing the process"
                disable_flag SSHState
                killall -9 dropbear
            fi
        else
            if wifi_enabled; then 
                log "Dropbear: Starting dropbear"
                dropbear -R
            else
                disable_flag SSHState
            fi
        fi
    else
        if is_running dropbear; then
            killall -9 dropbear
            log "Dropbear: Killed"
        fi
    fi
}


# Starts telnet if the toggle is set to on
# Telnet is generally already running when you boot your MMP, you won't see this hit logs unless you bounce it
check_telnetstate() { 
    if flag_enabled TelnetState; then
        if is_running telnetd; then
            if wifi_disabled; then
                log "Telnet: Wifi is turned off, disabling the toggle for Telnet and killing the process"
                disable_flag TelnetState
                killall -9 telnetd
            fi
        else
            if wifi_enabled; then 
                log "Telnet: Starting telnet"
                cd /mnt/SDCARD 
                $sysdir/script/launchtelnet.sh
            else
                disable_flag TelnetState
            fi
        fi
    else
        if is_running telnetd; then
            killall -9 telnetd
            log "Telnet: Killed"
        fi
    fi
}

# Starts Filebrowser if the toggle in tweaks is set on
check_httpstate() { 
    if flag_enabled HTTPState && [ -f /mnt/SDCARD/.tmp_update/bin/filebrowser/filebrowser ]; then
        if is_running filebrowser; then
            if wifi_disabled; then 
                log "Filebrowser: Wifi is turned off, disabling the toggle for HTTP FS and killing the process"
                disable_flag HTTPState
                pkill -9 filebrowser
            fi
        else
            # Checks if the toggle for WIFI is turned on.
            # Starts filebrowser bound to 0.0.0.0 so we don't need to mess around binding different IP's
            if wifi_enabled; then 
                $sysdir/bin/filebrowser/filebrowser -p 80 -a 0.0.0.0 -r /mnt/SDCARD -d $sysdir/bin/filebrowser/filebrowser.db &
                log "Filebrowser: Starting filebrowser listening on 0.0.0.0 to accept all traffic"
            else
                disable_flag HTTPState
            fi
        fi
    else
        if is_running filebrowser; then
            killall -9 filebrowser
            log "Filebrowser: Killed"
        fi
    fi
}


# Starts the hotspot based on the results of check_hotspotstate, called twice saves repeating
# We have to sleep a bit or sometimes supllicant starts before we can get the hotspot logo
# Starts AP and DHCP
start_hotspot() { 
	
	ifconfig wlan1 up 
	ifconfig wlan0 down
		
	# IP setup
	hotspot0addr=$(grep -E '^dhcp-range=' "$sysdir/config/dnsmasq.conf" | cut -d',' -f1 | cut -d'=' -f2) 
	hotspot0addr=$(echo $hotspot0addr | awk -F'.' -v OFS='.' '{$NF-=1; print}') 
	gateway0addr=$(grep -E '^dhcp-option=3' $sysdir/config/dnsmasq.conf | awk -F, '{print $2}')
	subnetmask=$(grep -E '^dhcp-range=.*,[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+,' "$sysdir/config/dnsmasq.conf" | cut -d',' -f3) 
	
	# Set IP route / If details
	ifconfig wlan1 $hotspot0addr netmask $subnetmask 
	
	# Start
	$sysdir/bin/hostapd -P /var/run/hostapd.pid -B -i wlan1 $sysdir/config/hostapd.conf &
	$sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root & 
	ip route add default via $gateway0addr
	
	if is_running hostapd; then
		log "Hotspot: Started with IP of: $hotspot0addr, subnet of: $subnetmask"
	else
		log "Hotspot: Failed to start, please try turning off/on. If this doesn't resolve the issue reboot your device."
		disable_flag HotspotState
	fi

}

# Starts personal hotspot if toggle is set to on
# Calls start_hotspot from above
# IF toggle is disabled, shuts down hotspot and bounces wifi.
check_hotspotstate() { 
    if [ ! -f $sysdir/config/.HotspotState ]; then
        if is_running hostapd; then
			log "Hotspot: Killed"
			pkill -9 hostapd 
			pkill -9 dnsmasq
            ifconfig wlan0 up
			ifconfig wlan1 down
            check_wifi			
		else
			return
        fi
    else
        if is_running hostapd; then
			if wifi_disabled; then 
				log "Hotspot: Wifi is turned off, disabling the toggle for hotspot and killing the process"
				rm $sysdir/config/.HotspotState
				pkill -9 hostapd
				pkill -9 dnsmasq
			fi
        else
			# if wifi_disabled; then 
				# sed -i 's/"wifi":\s*0/"wifi": 1/' /appconfigs/system.json
				# check_wifi
				# sleep 15
				# log "Hotspot: Requested but WiFi is off, bringing WiFi up now."
				# start_hotspot &
			# else
			start_hotspot &
			# fi
        fi
    fi
}

# We need to check if NTP is enabled and then check the state of tzselect in /.tmp_update/config/tzselect, based on the value we'll pass the TZ via the env var to ntpd and get the time (has to be POSIX)
# This will work but it will not export the TZ var across all opens shells so you may find the hwclock (and clock app, retroarch time etc) are correct but terminal time is not.
# It does set TZ on the tty that Main is running in so this is ok

check_tzid() {
    tzid=$(cat "$sysdir/config/tzselect") 
    echo "$tzid"
}

check_ntpstate() { 
    if flag_enabled NTPState && wifi_enabled; then
        current_tz=$(check_tzid)
        if [ "$current_tz" != "$old_tz" ]; then
            export old_tz="$current_tz"
            restart_ntp &
        fi
    fi
}

restart_ntp() {
    export old_tz=$(check_tzid)
    set_tzid
    log "NTP: Starting NTP with TZ of $TZ"
    ntpdate time.google.com
    hwclock -w
	touch /tmp/time_update
	sync
    log "NTP: TZ set to $TZ, Time set to: $(date) and merged to hwclock, which shows: $(hwclock)"
}

set_tzid() {
    check_tzid
    case $tzid in 
        0)  export TZ="UTC+12" ;;
        1)  export TZ="UTC+11" ;;
        2)  export TZ="UTC+10" ;;
        3)  export TZ="UTC+9" ;;
        4)  export TZ="UTC+8" ;;
        5)  export TZ="UTC+7" ;;
        6)  export TZ="UTC+6" ;;
        7)  export TZ="UTC+5" ;;
        8)  export TZ="UTC+4" ;;
        9)  export TZ="UTC+3" ;;
        10) export TZ="UTC+2" ;;
        11) export TZ="UTC+1" ;;
        12) export TZ="UTC" ;;
        13) export TZ="UTC-1" ;;
        14) export TZ="UTC-2" ;;
        15) export TZ="UTC-3" ;;
        16) export TZ="UTC-4" ;;
        17) export TZ="UTC-5" ;;
        18) export TZ="UTC-6" ;;
        19) export TZ="UTC-7" ;;
        20) export TZ="UTC-8" ;;
        21) export TZ="UTC-9" ;;
        22) export TZ="UTC-10" ;;
        23) export TZ="UTC-11" ;;
        24) export TZ="UTC-12" ;;
    esac
    echo "$TZ" > "$sysdir/config/T.Z"
}

set_tzid

wifi_enabled() {
    [ $(/customer/app/jsonval wifi) -eq 1 ]
}

wifi_disabled() {
    [ $(/customer/app/jsonval wifi) -eq 0 ]
}

flag_enabled() {
    flag="$1"
    [ -f "$sysdir/config/.$flag" ]
}

enable_flag() {
    flag="$1"
    touch "$sysdir/config/.$flag"
}

disable_flag() {
    flag="$1"
    rm "$sysdir/config/.$flag" 2>&1 /dev/null
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

log() {
    echo "$(date)" $* >> $sysdir/logs/network.log
}


update