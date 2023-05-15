#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo

main() {
    log "Network Checker: Update networking"

    check_wifi    
    check_tzid 
    write_tzid 
    check_ftpstate 
    check_sshstate 
    check_telnetstate 
    check_ntpstate 
    check_httpstate
}


check_wifi() {
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
                /mnt/SDCARD/App/Ftp/bftpd -d -c /mnt/SDCARD/App/Ftp/bftpd.conf &
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
                telnetd -l sh
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
    if flag_enabled HTTPState && [ -f /mnt/SDCARD/App/FileBrowser/filebrowser ]; then
        if is_running filebrowser; then
            if wifi_disabled; then 
                log "Filebrowser: Wifi is turned off, disabling the toggle for HTTP FS and killing the process"
                disable_flag HTTPState
                pkill -9 filebrowser
            fi
        else
            # Checks if the toggle for WIFI is turned on.
            # Starts filebrowser bound to 0.0.0.0 so we don't need to mess around binding different IP's
            # This cuts down heavily on lag in the UI (as we don't need to run commands to check/grab IP's) and allows the menu to work more seamlessly
            if wifi_enabled; then 
                cd /mnt/SDCARD/App/FileBrowser/
                ./filebrowser -p 80 -a 0.0.0.0 -r /mnt/SDCARD &
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
# Get the serial so we can use it for the hotspot password
# Check if the wpa pass is still set to the default pass, if it is change it to the serial number, if it's not then they've set a custom password, leave it alone.
# Starts AP and DHCP
# Turns off NTP as you wont be using it when you're on a hotspot
start_hotspot() { 
    if flag_enabled NTPState; then
        touch /tmp/ntprestore
        disable_flag NTPState
    fi
    
    if is_running hostapd; then
        log "Hotspot: MainUI has taken wlan0 while we're supposed to be in AP mode, killing wpa_supp again."
        sleep 5
        pkill -9 hostapd 
        pkill -9 dnsmasq
        pkill -9 wpa_supplicant 
        pkill -9 udhcpc 
    fi
    
    sleep 5 
    
    serial_number=$( { /config/riu_r 20 18 | awk 'NR==2'; /config/riu_r 20 17 | awk 'NR==2'; /config/riu_r 20 16 | awk 'NR==2'; } | sed 's/0x//g' | tr -d '[:space:]' ) 
    passphrase=$(grep '^wpa_passphrase=' "$sysdir/config/hostapd.conf" | cut -d'=' -f2)

    if [ "$passphrase" = "MiyooMiniApPassword" ]; then 
        sed -i "s/^wpa_passphrase=.*/wpa_passphrase=$serial_number/" "$sysdir/config/hostapd.conf"
        log "Hotspot: Default key removed."
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
    
    log "Hotspot: Started with IP of: $hotspot0addr, subnet of: $subnetmask"
}

# Starts personal hotspot if toggle is set to on
# Calls start_hotspot from above
# IF toggle is disabled, shuts down hotspot and bounces wifi.
# Restores NTP if it was on before we turned the hotspot on.
check_hotspotstate() { 
    if flag_enabled HotspotState; then
        if is_running hostapd; then
            if wifi_disabled; then
                log "Hotspot: Wifi is turned off, disabling the toggle for hotspot and killing the process"
                disable_flag HotspotState
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
            if wifi_disabled; then
                sed -i 's/"wifi":\s*0/"wifi": 1/' /appconfigs/system.json
                /customer/app/axp_test wifion
                sleep 2 
                ifconfig wlan0 up
                log "Hotspot: Requested but WiFi is off, bringing WiFi up now."
                start_hotspot &
            else
                start_hotspot &
            fi
        fi
    else
        if is_running hostapd; then
            log "Hotspot: Killed"
            pkill -9 hostapd 
            pkill -9 dnsmasq
            ifconfig wlan0 up
            $miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf &
            udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
                                  
            if [ -f /tmp/ntprestore ]; then
                enable_flag NTPState
                sync
            fi
        fi
    fi
}

# Get the value of the tz set in tweaks
check_tzid() {
    tzid=$(cat "$sysdir/config/tzselect") 
    echo "$tzid"
}

# Check the value and write it into TZ var - if they look backwards its because they are, ntpd is weird..
write_tzid() {
    case $tzid in 
        0)
            export TZ="UTC+12"
            ;;
        1)
            export TZ="UTC+11"
            ;;
        2)
            export TZ="UTC+10"
            ;;
        3)
            export TZ="UTC+9"
            ;;
        4)
            export TZ="UTC+8"
            ;;
        5)
            export TZ="UTC+7"
            ;;
        6)
            export TZ="UTC+6"
            ;;
        7)
            export TZ="UTC+5"
            ;;
        8)
            export TZ="UTC+4"
            ;;
        9)
            export TZ="UTC+3"
            ;;
        10)
            export TZ="UTC+2"
            ;;
        11)
            export TZ="UTC+1"
            ;;
        12)
            export TZ="UTC"
            ;;
        13)
            export TZ="UTC-1"
            ;;
        14)
            export TZ="UTC-2"
            ;;
        15)
            export TZ="UTC-3"
            ;;
        16)
            export TZ="UTC-4"
            ;;
        17)
            export TZ="UTC-5"
            ;;
        18)
            export TZ="UTC-6"
            ;;
        19)
            export TZ="UTC-7"
            ;;
        20)
            export TZ="UTC-8"
            ;;
        21)
            export TZ="UTC-9"
            ;;
        22)
            export TZ="UTC-10"
            ;;
        23)
            export TZ="UTC-11"
            ;;
        24)
            export TZ="UTC-12"
            ;;
    esac
}

# We need to check if NTP is enabled and then check the state of tzselect in /.tmp_update/config/tzselect, based on the value we'll pass the TZ via the env var to ntpd and get the time (has to be POSIX)
# This will work but it will not export the TZ var across all opens shells so you may find the hwclock (and clock app, retroarch time etc) are correct but terminal time is not.
# It does set TZ on the tty that Main is running in so this is ok
check_ntpstate() { 
    if flag_enabled NTPState; then
        if is_running ntpd; then
            if wifi_enabled; then 
                export new_tz=$(check_tzid)
                if [ "$old_tz" != "$new_tz" ]; then
                    pkill -9 ntpd
                    log "NTP: Killed, TZ has changed"
                    check_tzid
                    write_tzid
                    ntpd -p time.google.com &
                    sleep 1
                    hwclock -w
                    log "NTP2: TZ set to $TZ, Time set to: $(date) and merged to hwclock, which shows: $(hwclock)"
                    export old_tz=$(check_tzid)
                fi
            else
                log "NTP: Wifi is turned off, disabling the toggle for NTP and killing the process"
                disable_flag NTPState
                pkill -9 ntpd
            fi
        else
            if wifi_enabled; then 
                pkill -9 ntpd
                check_tzid
                write_tzid
                log "NTP: Starting NTP with TZ of $TZ"
                ntpd -p time.google.com &
                sleep 1
                hwclock -w
                log "NTP1: TZ set to $TZ, Time set to: $(date) and merged to hwclock, which shows: $(hwclock)"
                export old_tz=$(check_tzid)
            else
                log "NTP: Wifi is turned off, disabling the toggle for NTP and killing the process"
                disable_flag NTPState
                pkill -9 ntpd
            fi
        fi
    else
        if is_running ntpd; then
            pkill -9 ntpd
            log "NTP: Killed by request"
        fi
    fi
}


# Utils

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


main
