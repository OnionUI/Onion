#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
confdir=/mnt/SDCARD/.tmp_update/config
jsonfile="$confdir/.net_service_restart.json"
filebrowserbin=$sysdir/bin/filebrowser
filebrowserdb=$sysdir/config/filebrowser/filebrowser.db
netscript=/mnt/SDCARD/.tmp_update/script/network
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"
is_booting=$([ -f /tmp/is_booting ] && echo 1 || echo 0)

# add service flags here to be remembered when wifi change is detected
services="httpState ftpState smbdState sshState authsshState authftpState authhttpState"

logfile=$(basename "$0" .sh)
. $sysdir/script/log.sh

main() {
    init_json
    set_tzid
    get_password
    case "$1" in
        check) # called by runtime.sh::check_networking
            check
            ;;
        ftp | telnet | http | ssh | smbd)
            service=$1
            case "$2" in
                toggle)
                    check_${service}state &
                    store_state & # if anything is toggled, store the state
                    ;;
                authed)
                    ${service}_authed &
                    store_state &
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
        save) # Save the state before wifi goes down so we can quick restore on re-enabling
            store_state
            ;;
        restore) # Restore the net service state after restarting
            restore_state
            ;;
        full_reset) # Do a full reset of the json file
            full_reset
            ;;
        *)
            print_usage
            ;;
    esac
}

# Standard check from runtime for startup.
check() {
    log "Network Checker: Update networking"
    local force_wifi_on_startup=$([ -f /customer/app/axp_test ] && [ -f $sysdir/config/.ntpForce ] && echo 1 || echo 0)
    local has_wifi=$(wifi_enabled && echo 1 || echo 0)

    check_wifi
    check_ftpstate &
    check_sshstate &
    check_telnetstate &
    check_httpstate &
    check_smbdstate &

    if [ "$is_booting" -eq 1 ]; then
        if [ "$has_wifi" -eq 0 ]; then
            if [ "$force_wifi_on_startup" -eq 1 ]; then
                bootScreen Boot "Turning on Wi-Fi..."
                wifi_on
                has_wifi=1
            fi
        else
            bootScreen Boot "Waiting for network..."
        fi
    fi

    if [ "$has_wifi" -eq 1 ] && flag_enabled ntpWait && [ $is_booting -eq 1 ]; then
        bootScreen Boot "Syncing time..."
        check_ntpstate && bootScreen Boot "Time synced: $(date +"%H:%M")" || bootScreen Boot "Time sync failed"
        sleep 1
    elif wifi_enabled; then
        check_ntpstate &
    fi

    if [ -f "$sysdir/.updateAvailable" ] && [ $is_booting -eq 1 ]; then
        bootScreen Boot "Update available!"
        sleep 1
    elif [ "$has_wifi" -eq 1 ] && [ ! -f /tmp/update_checked ]; then
        touch /tmp/update_checked
        $sysdir/script/ota_update.sh check &
    fi

    if [ "$is_booting" -eq 1 ] && [ "$force_wifi_on_startup" -eq 1 ] && wifi_disabled; then
        bootScreen Boot "Turning off Wi-Fi..."
        wifi_off
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
    # Fixes lockups entering some apps after enabling wifi (because wpa_supp/udhcpc are preloaded with libpadsp.so)
    libpadspblocker &

    if is_running hostapd; then
        return
    else
        if wifi_enabled; then
            if ! ifconfig wlan0 >> /dev/null 2>&1 || [ -f /tmp/restart_wifi ]; then
                restore_state
                if [ -f /tmp/restart_wifi ]; then
                    pkill -9 wpa_supplicant
                    pkill -9 udhcpc
                    rm /tmp/restart_wifi
                fi
                wifi_on
            fi
        else
            if [ ! -f "/tmp/dont_restart_wifi" ]; then
                store_state
                wifi_off
            fi
        fi
    fi
}

wifi_on() {
    log "Network Checker: Turning on Wi-Fi..."
    /customer/app/axp_test wifion
    sleep 2
    ifconfig wlan0 up
    $miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
    udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
    iw dev wlan0 set power_save off
}

wifi_off() {
    log "Network Checker: Turning off Wi-Fi..."
    pkill -9 wpa_supplicant
    pkill -9 udhcpc
    /customer/app/axp_test wifioff
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

                $netscript/start_smbd.sh $PASS &
                log "Samba: Starting smbd.."
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
                mkdir -p $sysdir/etc/dropbear
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

check_ntpstate() {
    ret_val=0
    if flag_enabled ntpState && [ ! -f "$sysdir/config/.hotspotState" ]; then
        set_tzid
        [ -f /tmp/ntp_synced ] && return 0

        if [ -f /tmp/ntp_failed ]; then
            # only run once on boot, but don't prevent more checks later on state_change
            # effectively only running every second time this is called while off network
            rm /tmp/ntp_failed
            return 1
        fi

        attempts=0
        max_wait_ip=10
        max_attempts=3
        ret_val=1
        got_ip=0
        # wait for an ip address from dhcp before we start
        while true; do
            ip=$(ifconfig wlan0 | grep 'inet addr:' | cut -d: -f2 | cut -d' ' -f1)
            if [ -z "$ip" ]; then
                attempts=$((attempts + 1))
                log "NTPwait: Waiting for IP address since $attempts seconds"
                if [ $attempts -ge $max_wait_ip ]; then
                    log "NTPwait: Could not aquire an IP address"
                    ret_val=1
                    got_ip=0
                    break
                fi
            else
                log "NTPwait: IP address aquired: $ip"
                got_ip=1
                break
            fi
            sleep 1
        done
        attempts=0
        if [ "$got_ip" -eq 1 ]; then
            while true; do
                log "NTPwait: get_time attempt $attempts"
                if ping -q -c 1 -W 1 worldtimeapi.org > /dev/null 2>&1; then
                    if get_time; then
                        ret_val=0
                        break
                    fi
                else
                    log "NTPwait: Can't reach network."
                fi
                attempts=$((attempts + 1))
                if [ $attempts -eq $max_attempts ]; then
                    log "NTPwait: Ran out of time before we could sync, stopping."
                    ret_val=1
                    touch /tmp/ntp_failed
                    break
                fi
                sleep 1
            done
        fi
    fi
    return "$ret_val"
}

get_time() { # handles 2 types of network time, instant from an API or longer from an NTP server, if the instant API checks fails it will fallback to the longer ntp
    log "NTP: started time update"

    response=$(curl -s -m 3 http://worldtimeapi.org/api/ip.txt)
    utc_datetime=$(echo "$response" | grep -o 'utc_datetime: [^.]*' | cut -d ' ' -f2 | sed "s/T/ /")
    if ! flag_enabled "manual_tz"; then
        utc_offset="UTC$(echo "$response" | grep -o 'utc_offset: [^.]*' | cut -d ' ' -f2)"
    fi

    if [ -z "$utc_datetime" ]; then
        log "NTP: Failed to get time from worldtimeapi.org, trying timeapi.io"
        utc_datetime=$(curl -s -k -m 5 https://timeapi.io/api/Time/current/zone?timeZone=UTC | grep -o '"dateTime":"[^.]*' | cut -d '"' -f4 | sed 's/T/ /')
        if ! flag_enabled "manual_tz"; then
            ip_address=$(curl -s -k -m 5 https://api.ipify.org)
            utc_offset_seconds=$(curl -s -k -m 5 https://timeapi.io/api/TimeZone/ip?ipAddress=$ip_address | jq '.currentUtcOffset.seconds')
            utc_offset="$(convert_seconds_to_utc_offset $utc_offset_seconds)"
        fi
    fi

    if [ -n "$utc_datetime" ]; then
        playActivity stop_all

        if [ -n "$utc_offset" ]; then
            echo "$utc_offset" | sed 's/\+/_/' | sed 's/-/+/' | sed 's/_/-/' > $sysdir/config/.tz
            cp $sysdir/config/.tz $sysdir/config/.tz_sync
            sync
            set_tzid
        fi

        if date -u -s "$utc_datetime" > /dev/null 2>&1; then
            hwclock -w
            log "NTP: Time successfully aquired using API"
            touch /tmp/ntp_synced
            playActivity resume
            return 0
        fi

        playActivity resume
    fi

    log "NTP: Failed to get time via timeapi.io as well, falling back to NTP."
    rm $sysdir/config/.tz_sync 2> /dev/null

    ntpdate -t 3 -u time.google.com
    if [ $? -eq 0 ]; then
        log "NTP: Time successfully aquired using NTP"
        return 0
    fi

    log "NTP: Failed to synchronize time using NTPdate, both methods have failed."
    return 1
}

# Utility functions

# create/pop the json file for remembering states
init_json() {
    if [ ! -f "$jsonfile" ]; then
        echo '{}' > "$jsonfile"
    fi

    if [ ! -s "$jsonfile" ]; then
        echo '{}' > "$jsonfile"
    fi
}

# unhook libpadsp.so on the wifi servs
libpadspblocker() {
    wpa_pid=$(ps -e | grep "[w]pa_supplicant" | awk 'NR==1{print $1}')
    udhcpc_pid=$(ps -e | grep "[u]dhcpc" | awk 'NR==1{print $1}')
    if [ -n "$wpa_pid" ] && [ -n "$udhcpc_pid" ]; then
        if grep -q "libpadsp.so" /proc/$wpa_pid/maps || grep -q "libpadsp.so" /proc/$udhcpc_pid/maps; then
            echo "Network Checker: $wpa_pid(WPA) and $udhcpc_pid(UDHCPC) found preloaded with libpadsp.so"
            unset LD_PRELOAD
            killall -9 wpa_supplicant
            killall -9 udhcpc
            $miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf &
            udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &
            echo "Network Checker: Removing libpadsp.so preload on wpa_supp/udhcpc"
        fi
    fi
}

# Store the network service state currently
store_state() {
    service_key="{"
    first_item=true

    for service in $services; do
        if [ "$first_item" = true ]; then
            first_item=false
        else
            service_key="$service_key,"
        fi

        if [ -f "$sysdir/config/.$service" ]; then
            service_key="$service_key \"$service\": \"1\""
        else
            service_key="$service_key \"$service\": \"0\""
        fi
    done

    service_key="$service_key }"
    echo "$service_key" > "$jsonfile"
}

# Restore the network service state, don't use jq it's too slow and holds the UI thread until it returns
# (you can't background this, you'll miss the checks for net servs as the flags won't be set)
restore_state() {
    [ ! -f "$jsonfile" ] && return
    log "Network Checker: Restoring state from $jsonfile"

    grep -E '"(httpState|ftpState|smbdState|sshState|authsshState|authftpState|authhttpState)":\s*"[01]"' "$jsonfile" | while IFS=":" read -r key value; do
        key=$(echo "$key" | tr -d ' "{}')
        value=$(echo "$value" | tr -d ' ",')

        if echo "$services" | grep -wq "$key"; then
            if [ "$value" = "1" ]; then
                enable_flag "$key"
            elif [ "$value" = "0" ]; then
                disable_flag "$key"
            fi
        fi
    done
}

full_reset() {
    for service in $services; do
        disable_flag "$service"
    done
    rm -f "$jsonfile"
}

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
    echo "Usage: $0 {check|ftp|telnet|http|ssh|smbd|disableall|save|restore|full_reset} [toggle|authed]"
    echo "       - For {ftp|telnet|http|ssh|smbd}, [toggle|authed] can be specified."
    echo "       - Commands {ntp|hotspot} only accept 'toggle'."
    echo "       - Commands {save|restore|full_reset} do not require additional args."
    echo "e.g:"
    echo "ftp toggle       - Will toggle the current FTP state"
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
    mv "$sysdir/config/.$flag" "$sysdir/config/.$flag_"
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

is_running_exact() {
    process_name="$1"
    pgrep -f "$process_name" > /dev/null
}

get_password() {
    # Get password from file for use with network services authentication
    PASS=$(cat "$sysdir/config/.password.txt")
}

if [ -f $sysdir/config/.logging ]; then
    main "$@"
else
    main "$@" 2>&1 > /dev/null
fi
