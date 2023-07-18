#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
filebrowserbin=$sysdir/bin/filebrowser
filebrowserdb=$sysdir/config/filebrowser/filebrowser.db
netscript=/mnt/SDCARD/.tmp_update/script/network
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

main() {
    set_tzid
    get_password
    case "$1" in
        check) # runs the check function we use in runtime, will be called on boot
            check
            ;;
        ftp | telnet | http | ssh | smbd)
            service=$1
            case "$2" in
                toggle)
                    check_${service}state &
                    ;;
                authed)
                    ${service}_authed &
                    ;;
                *)
                    print_usage
                    ;;
            esac
            ;;
        ntp | hotspot)
            service=$1
            case "$2" in
                toggle)
                    check_${service}state
                    ;;
                *)
                    echo "Usage: $0 {ntp|hotspot} toggle"
                    exit 1
                    ;;
            esac
            ;;
        disableall)
            disable_all_services
            ;;
        *)
            print_usage
            ;;
    esac
}

# Standard check from runtime for startup.
check() {
    log "Network Checker: Update networking"

    if wifi_enabled; then
        bootScreen Boot "Waiting for network..."
    fi

    check_wifi
    check_ftpstate
    check_sshstate
    check_telnetstate
    check_ntpstate
    check_httpstate
    check_smbdstate

    if wifi_enabled && flag_enabled ntpWait; then
        bootScreen Boot "Syncing time..."
        sync_time && bootScreen Boot "Time synced! $(date +"%H:%M")" || bootScreen Boot "Time sync failed!"
    else
        sync_time &
    fi

    if wifi_enabled && flag_enabled checkUpdates; then
        bootScreen Boot "Checking for updates..."
        $sysdir/script/ota_update.sh check
        if [ $? -eq 0 ]; then
            bootScreen Boot "Update available!"
        else
            bootScreen Boot "No update found"
        fi
        sleep 2
    fi
}

# Function to help disable and kill off all processes
disable_all_services() {
    disable_flag sshState
    disable_flag authsshState
    disable_flag telnetState
    disable_flag ftpState
    disable_flag authftpState
    disable_flag httpState
    disable_flag authhttpState
    disable_flag smbdState

    for process in dropbear bftpd filebrowser telnetd hostapd dnsmasq smbd; do
        if is_running $process; then
            killall -9 $process
        fi
    done
}

# Core function
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
            if [ ! -f "/tmp/dont_restart_wifi" ]; then
                pkill -9 wpa_supplicant
                pkill -9 udhcpc
                /customer/app/axp_test wifioff
            fi
        fi
    fi
}

# Starts the samba daemon if the toggle is set to on
check_smbdstate() {
    if flag_enabled smbdState; then
        if is_running smbd; then
            if wifi_disabled; then
                log "Samba: Wifi is turned off, disabling the toggle for smbd and killing the process"
                disable_flag smbdState
                killall -9 smbd
            fi
        else
            if wifi_enabled; then
                sync

                mkdir -p \
                    /var/lib/samba \
                    /var/run/samba/ncalrpc \
                    /var/private \
                    /var/log/

                update_smbconf
                $netscript/start_smbd.sh $PASS &
                log "Samba: Starting smbd at exit of tweaks.."
            else
                disable_flag smbdState
            fi
        fi
    else
        if is_running smbd; then
            killall -9 smbd
            log "Samba: Killed"
        fi
    fi
}

update_smbconf() {
    if flag_enabled authsmbdState; then
        sed -i -e '/guest only/s/1/0/g' -e 's/#alid users = onion/valid users = onion/g' $sysdir/config/smb.conf
    else
        sed -i -e '/guest only/s/0/1/g' -e 's/valid users = onion/#alid users = onion/g' $sysdir/config/smb.conf
    fi
}

smbd_authed() {
    if flag_enabled smbdState; then
        update_smbconf
        if is_running smbd; then
            killall -9 smbd
        fi
        $netscript/start_smbd.sh $PASS &
    fi
}

# Starts bftpd if the toggle is set to on
check_ftpstate() {
    if flag_enabled ftpState; then
        if is_running bftpd; then
            if wifi_disabled; then
                log "FTP: Wifi is turned off, disabling the toggle for FTP and killing the process"
                disable_flag ftpState
                killall -9 bftpd
            fi
        else
            if wifi_enabled; then
                log "FTP: Starting bftpd"
                sync
                if flag_enabled authftpState; then
                    ftp_authed
                else
                    bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf
                    log "FTP: Starting bftpd without auth"
                fi
            else
                disable_flag ftpState
            fi
        fi
    else
        if is_running bftpd; then
            killall -9 bftpd
            log "FTP: Killed"
        fi
    fi
}

# Called by above function on boot or when auth state is toggled in tweaks
ftp_authed() {
    if flag_enabled ftpState; then
        if is_running_exact "bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf" || flag_enabled authftpState; then
            killall -9 bftpd
            bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpdauth.conf
            log "FTP: Starting bftpd with auth"
        else
            killall -9 bftpd
            bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf
        fi
    fi
}

# Starts dropbear if the toggle is set to on
check_sshstate() {
    if flag_enabled sshState; then
        if is_running dropbear; then
            if wifi_disabled; then
                log "SSH: Wifi is turned off, disabling the toggle for dropbear and killing the process"
                disable_flag sshState
                killall -9 dropbear
            fi
        else
            if wifi_enabled; then
                sync
                if flag_enabled authsshState; then
                    ssh_authed
                else
                    log "SSH: Starting dropbear without auth"
                    dropbear -R -B
                fi
            else
                disable_flag sshState
            fi
        fi
    else
        if is_running dropbear; then
            killall -9 dropbear
            log "SSH: Killed"
        fi
    fi
}

# Called by above function on boot or when auth state is toggled in tweaks
ssh_authed() {
    if flag_enabled sshState; then
        if is_running_exact "dropbear -R -B" || flag_enabled authsshState; then
            killall -9 dropbear
            log "SSH: Starting dropbear with auth"
            dropbear -R
        else
            log "SSH: Starting dropbear without auth"
            killall -9 dropbear
            dropbear -R -B
        fi
    fi
}

# Starts telnet if the toggle is set to on
# Telnet is generally already running when you boot your MMP. This will kill the firmware version and launch a passworded version if auth is turned on, if auth is off it will launch a version with env vars set
check_telnetstate() {
    if is_running_exact "telnetd -l sh"; then
        pkill -9 -f "telnetd -l sh"
        log "Telnet: Killing firmware telnetd process"
        sleep 1 # Wait for the process to die
    fi

    if flag_enabled telnetState; then
        if is_running telnetd; then
            if wifi_disabled; then
                log "Telnet: Wifi is turned off, disabling the toggle for Telnet and killing the process"
                disable_flag telnetState
                killall -9 telnetd
            fi
        else
            if wifi_enabled; then
                sync
                log "Telnet: Starting telnet"
                telnetd -l $netscript/telnetenv.sh
            else
                disable_flag telnetState
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
    if flag_enabled httpState && [ -f $filebrowserbin ]; then
        if is_running_exact "$filebrowserbin -p 80 -a 0.0.0.0 -r /mnt/SDCARD -d $filebrowserdb"; then
            if wifi_disabled; then
                log "Filebrowser(HTTP server): Wifi is turned off, disabling the toggle for HTTP FS and killing the process"
                disable_flag httpState
                pkill -9 filebrowser
            fi
        else
            # Checks if the toggle for WIFI is turned on.
            if wifi_enabled; then
                # Check if authhttpState is enabled/set to json, if not set noauth
                sync
                if flag_enabled authhttpState; then
                    http_authed
                else
                    $filebrowserbin -p 80 -a 0.0.0.0 -r /mnt/SDCARD -d $filebrowserdb >> /dev/null 2>&1 &
                    log "Filebrowser(HTTP server): Starting filebrowser listening on 0.0.0.0"
                fi
            else
                disable_flag httpState
            fi
        fi
    else
        if is_running filebrowser; then
            killall -9 filebrowser
        fi
    fi
}

# Called by above function on boot or when auth state is toggled in tweaks
http_authed() {
    if flag_enabled httpState; then
        if flag_enabled authhttpState; then
            killall -9 filebrowser
            $filebrowserbin config set --auth.method=json -d $filebrowserdb 2>&1 > /dev/null
        else
            if [ "$(is_noauth_enabled)" -eq 0 ]; then
                killall -9 filebrowser
                $filebrowserbin config set --auth.method=noauth -d $filebrowserdb 2>&1 > /dev/null
            fi
        fi
        $filebrowserbin -p 80 -a 0.0.0.0 -r /mnt/SDCARD -d $filebrowserdb 2>&1 > /dev/null &
    fi
}

# Starts the hotspot based on the results of check_hotspotstate, called twice saves repeating
# We have to sleep a bit or sometimes supllicant starts before we can get the hotspot logo
# Starts AP and DHCP

start_hotspot() {
    ifconfig wlan1 up >> /dev/null 2>&1
    ifconfig wlan0 down >> /dev/null 2>&1

    sleep 1
    # IP setup
    hotspot0addr=$(grep -E '^dhcp-range=' "$sysdir/config/dnsmasq.conf" | cut -d',' -f1 | cut -d'=' -f2)
    hotspot0addr=$(echo $hotspot0addr | awk -F'.' -v OFS='.' '{$NF-=1; print}')
    gateway0addr=$(grep -E '^dhcp-option=3' $sysdir/config/dnsmasq.conf | awk -F, '{print $2}')
    subnetmask=$(grep -E '^dhcp-range=.*,[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+,' "$sysdir/config/dnsmasq.conf" | cut -d',' -f3)

    # Set IP route / If details
    ifconfig wlan1 $hotspot0addr netmask $subnetmask >> /dev/null 2>&1

    # Start

    $sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root &
    $sysdir/bin/hostapd -i wlan1 $sysdir/config/hostapd.conf >> /dev/null 2>&1 &

    ip route add default via $gateway0addr

    if is_running hostapd; then
        log "Hotspot: Started with IP of: $hotspot0addr, subnet of: $subnetmask"
    else
        log "Hotspot: Failed to start, please try turning off/on. If this doesn't resolve the issue reboot your device."
        disable_flag hotspotState
    fi

}

# Starts personal hotspot if toggle is set to on
# Calls start_hotspot from above
# IF toggle is disabled, shuts down hotspot and bounces wifi.
check_hotspotstate() {
    sync
    if [ ! -f $sysdir/config/.hotspotState ]; then
        if is_running hostapd; then
            log "Hotspot: Killed"
            pkill -9 hostapd
            pkill -9 dnsmasq
            ifconfig wlan0 up
            ifconfig wlan1 down
            killall -9 udhcpc
            udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
            check_wifi
        else
            return
        fi
    else
        if is_running hostapd; then
            if wifi_disabled; then
                log "Hotspot: Wifi is turned off, disabling the toggle for hotspot and killing the process"
                rm $sysdir/config/.hotspotState
                pkill -9 hostapd
                pkill -9 dnsmasq
            fi
        else
            start_hotspot &
        fi
    fi
}

# This is the fallback!
# We need to check if NTP is enabled and then check the state of tzselect in /.tmp_update/config/tzselect, based on the value we'll pass the TZ via the env var to ntpd and get the time (has to be POSIX)
# This will work but it will not export the TZ var across all opens shells so you may find the hwclock (and clock app, retroarch time etc) are correct but terminal time is not.
# It does set TZ on the tty that Main is running in so this is ok

sync_time() {
    if [ -f "$sysdir/config/.ntpState" ] && wifi_enabled; then
        attempts=0
        max_attempts=20
        ret_val=1
        while true; do
            if [ ! -f "/tmp/ntp_run_once" ]; then
                break
            fi

            if ping -q -c 1 google.com > /dev/null 2>&1; then
                if get_time; then
                    ret_val=0
                    touch /tmp/ntp_synced
                fi
                break
            fi
            attempts=$((attempts + 1))
			log "NTPwait: Attempt $attempts"
            if [ $attempts -eq $max_attempts ]; then
                log "NTPwait: Ran out of time before we could sync, stopping."
                ret_val=1
                break
            fi
            sleep 1
        done
        rm /tmp/ntp_run_once
    fi
    return "$ret_val"
}

check_ntpstate() { # This function checks if the timezone has changed, we call this in the main loop.
    if flag_enabled ntpState && wifi_enabled && [ ! -f "$sysdir/config/.hotspotState" ]; then
        set_tzid
        if [ ! -f /tmp/ntp_synced ] && get_time; then
            touch /tmp/ntp_synced
        fi
    fi
}

get_time() { # handles 2 types of network time, instant from an API or longer from an NTP server, if the instant API checks fails it will fallback to the longer ntp
    log "NTP: started time update"

    # get_time() is sometimes called to early - make sure we have connectivity
    if ! ping -c 1 1.1.1.1; then
        log "NTP: no internet connectivity - exiting"
        return 1
    fi

    response=$(curl -s --connect-timeout 3 http://worldtimeapi.org/api/ip.txt)
    utc_datetime=$(echo "$response" | grep -o 'utc_datetime: [^.]*' | cut -d ' ' -f2 | sed "s/T/ /")
    if ! flag_enabled "manual_tz"; then
        utc_offset="UTC$(echo "$response" | grep -o 'utc_offset: [^.]*' | cut -d ' ' -f2)"
    fi

    if [ -z "$utc_datetime" ]; then
        log "NTP: Failed to get time from worldtimeapi.org, trying timeapi.io"
        utc_datetime=$(curl -s -k --connect-timeout 5 https://timeapi.io/api/Time/current/zone?timeZone=UTC | grep -o '"dateTime":"[^.]*' | cut -d '"' -f4 | sed 's/T/ /')
        if ! flag_enabled "manual_tz"; then
            ip_address=$(curl -s -k --connect-timeout 5 https://api.ipify.org)
            utc_offset_seconds=$(curl -s -k --connect-timeout 5 https://timeapi.io/api/TimeZone/ip?ipAddress=$ip_address | jq '.currentUtcOffset.seconds')
            utc_offset="$(convert_seconds_to_utc_offset $utc_offset_seconds)"
        fi
    fi

    if [ ! -z "$utc_datetime" ]; then
        if [ ! -z "$utc_offset" ]; then
            echo "$utc_offset" | sed 's/\+/_/' | sed 's/-/+/' | sed 's/_/-/' > $sysdir/config/.tz
            cp $sysdir/config/.tz $sysdir/config/.tz_sync
            sync
            set_tzid
        fi
        if date -u -s "$utc_datetime" > /dev/null 2>&1; then
            hwclock -w
            return 0
        fi
    fi

    log "NTP: Failed to get time via timeapi.io as well, falling back to NTP."
    rm $sysdir/config/.tz_sync 2> /dev/null

    ntpdate -t 3 -u time.google.com
    if [ $? -eq 0 ]; then
        return 0
    fi

    log "NTP: Failed to synchronize time using NTPdate, both methods have failed."
    return 1
}

# Utility functions

convert_seconds_to_utc_offset() {
    seconds=$(($1))
    if [ $seconds -ne 0 ]; then
        printf "UTC%s%02d%s" \
            $([[ $seconds -lt 0 ]] && echo -n "-" || echo -n "+") \
            $(abs $(($seconds / 3600))) \
            $([[ $(($seconds % 3600)) -eq 0 ]] && echo -n ":00" || echo -n ":30")
    else
        echo -n "UTC"
    fi
}

abs() {
    [[ $(($@)) -lt 0 ]] && echo "$((($@) * -1))" || echo "$(($@))"
}

set_tzid() {
    export TZ=$(cat "$sysdir/config/.tz")
}

is_noauth_enabled() { # Used to check authMethod val for HTTPFS
    DB_PATH="$filebrowserdb"
    OUTPUT=$(cat $DB_PATH)

    if echo $OUTPUT | grep -q '"authMethod":"noauth"'; then
        echo 1
    else
        echo 0
    fi
}

print_usage() {
    echo "Usage: $0 {check|ftp|telnet|http|ssh|ntp|hotspot|smbd|disableall} {toggle|authed} - {ntp|hotspot|telnet} only accept toggle."
    exit 1
}

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
    rm "$sysdir/config/.$flag" /dev/null 2>&1
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

is_running_exact() {
    process_name="$1"
    pgrep -f "$process_name" > /dev/null
}

LOGGING=$([ -f $sysdir/config/.logging ] && echo 1 || echo 0)
scriptname=$(basename "$0" .sh)

log() {
    if [ $LOGGING -eq 1 ]; then
        echo -e "($scriptname) $(date):" $* | tee -a "$sysdir/logs/$scriptname.log"
    fi
}

get_password() {
    # Get password from file for use with network services authentication
    PASS=$(cat "$sysdir/config/.password.txt")
}

if [ $LOGGING -eq 1 ]; then
    main "$@"
else
    main "$@" 2>&1 > /dev/null
fi
