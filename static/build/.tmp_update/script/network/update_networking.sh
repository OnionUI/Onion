#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
filebrowserbin=$sysdir/bin/filebrowser
filebrowserdb=$sysdir/config/filebrowser/filebrowser.db
netscript=/mnt/SDCARD/.tmp_update/script/network
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

main() {
    set_oldtz
    set_tzid
    case "$1" in
        check) # runs the check function we use in runtime, will be called on boot
            check
            ;;
        ftp|telnet|http|ssh)
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
        ntp|hotspot)
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
    esac
}
# Standard check from runtime for startup.
check() {
    log "Network Checker: Update networking"
    check_wifi
    check_ftpstate 
    check_sshstate 
    check_telnetstate 
    check_ntpstate
    check_httpstate
	
	if flag_enabled ntpWait; then
			sync_time
		else
			sync_time &
	fi
}

# Function to help disable and kill off all processes
disable_all_services() {
	disable_flag sshState
	disable_flag authsshState
	disable_flag telnetState
	disable_flag authtelnetState
	disable_flag ftpState
	disable_flag authftpState
	disable_flag httpState
	disable_flag authhttpState
	
	for process in dropbear bftpd filebrowser telnetd hostapd dnsmasq; do
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
		if ! ifconfig wlan0  || [ -f /tmp/restart_wifi ]; then
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
ssh_authed(){
if flag_enabled sshState; then
	if is_running_exact "dropbear -R -B" || flag_enabled authsshState; then
		killall -9 dropbear
		log "SSH: Starting dropbear with auth"
		file_path="$sysdir/config/.auth.txt"
		password=$(awk 'NR==1 {print $2}' "$file_path")
		dropbear -R -Y $password >> /mnt/SDCARD/sshdebug2.log 2>&1
		log "SSH: Started dropbear with auth"
	else
		log "SSH: Starting dropbear without auth"
		killall -9 dropbear
		dropbear -R -B >> /mnt/SDCARD/sshdebug.log 2>&1
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
				if flag_enabled authtelnetState; then
					telnet_authed
				else
					log "Telnet: Starting telnet without auth"
					telnetd -l $netscript/telnetenv.sh
				fi
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

# Called by above function on boot or when auth state is toggled in tweaks
telnet_authed() {
if flag_enabled telnetState; then
	if is_running_exact "telnetd -l /mnt/SDCARD/.tmp_update/script/telnetenv.sh" || flag_enabled authtelnetState; then
		killall -9 telnetd
		sleep 0.5
		log "Telnet: Starting telnet with auth"
		telnetd -l $netscript/telnetlogin.sh
	else
		killall -9 telnetd
		telnetd -l $netscript/telnetenv.sh
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
	 if [ "$(echo_enabled authhttpState)" -eq 1 ]; then 
			killall -9 filebrowser
			$filebrowserbin config set --auth.method=json -d $filebrowserdb >> $sysdir/logs/http.log
		else
			if [ "$(is_noauth_enabled)" -eq 0 ]; then
				killall -9 filebrowser
				$filebrowserbin config set --auth.method=noauth -d $filebrowserdb >> $sysdir/logs/http.log
			fi
		fi
		$filebrowserbin -p 80 -a 0.0.0.0 -r /mnt/SDCARD -d $filebrowserdb >> /dev/null 2>&1 &
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
		
		while true; do
			if [ ! -f "/tmp/ntp_run_once" ]; then
				break
			fi
			
			if ping -q -c 1 google.com > /dev/null 2>&1 ; then
				get_time
				break
			fi

			attempts=$((attempts+1))
			if [ $attempts -eq $max_attempts ]; then
				log "NTPwait: Ran out of time before we could sync, stopping."
				break
			fi
			sleep 1
		done
		rm /tmp/ntp_run_once
	fi
}

check_ntpstate() { # This function checks if the timezone has changed, we call this in the main loop.
    if flag_enabled ntpState && wifi_enabled && [ ! -f "$sysdir/config/.hotspotState" ] ; then
        current_tz=$(check_tzid)
        if [ "$current_tz" != "$old_tz" ]; then
            log "NTP: timezone changed: current_tz:$current_tz old_tz:$old_tz"
            restart_ntp
        else
            log "NTP: no need to restart_ntp, current_tz:$current_tz old_tz:$old_tz"
            get_time
        fi
    fi
}


restart_ntp() { # If we detect that the timezone has changed we'll restart the ntp process to reset it in the main shell (runtime.sh shell)
    export old_tz=$(check_tzid)
    echo $old_tz > "$sysdir/config/.old_tz"
    set_tzid
    log "NTP: Starting NTP with TZ of $TZ"
    get_time
    log "NTP: TZ set to $TZ, Time set to: $(date) and merged to hwclock, which shows: $(hwclock)"
}
 
# Utility functions

check_tzid() {
    tzid=$(cat "$sysdir/config/.tz") 
    echo "$tzid"
}

set_oldtz() { #export to file because environment variables don't carry over
    export old_tz=$(cat "$sysdir/config/.old_tz") 
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

echo_enabled() {
    flag="$1"
    if [ -f "$sysdir/config/.$flag" ]; then
        echo 1
    else
        echo 0
    fi
}

print_usage() {
    echo "Usage: $0 {check|ftp|telnet|http|ssh|ntp|hotspot|disableall} {toggle|authed}"
    exit 1
}

get_time() { # handles 2 types of network time, instant from an API or longer from an NTP server, if the instant API checks fails it will fallback to the longer ntp
    log "NTP: started time update"
    response=$(curl -s --connect-timeout 3 http://worldtimeapi.org/api/ip.txt |
        grep -o 'utc_datetime: [^.]*' | cut -d ' ' -f2 | sed "s/T/ /")
    if [ -z "$response" ]; then
        log "NTP: Failed to get time from worldtimeapi.org, trying timeapi.io"
        response=$(curl -s -k --connect-timeout 5 https://timeapi.io/api/Time/current/zone?timeZone=UTC |
            grep -o '"dateTime":"[^.]*' | cut -d '"' -f4 | sed 's/T/ /')
    fi
    if [ ! -z "$response" ]; then
        if date -u -s "$response" >/dev/null 2>&1; then
            hwclock -w
            return 0
        fi
    fi
    log "NTP: Failed to get time via timeapi.io as well, falling back to NTP."
    ntpdate -t 3 -u time.google.com
    if [ $? -ne 0 ]; then
        log "NTP: Failed to synchronize time using NTPdate, both methods have failed."
    fi

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
    rm "$sysdir/config/.$flag" 2>&1 /dev/null
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

is_running_exact() {
    process_name="$1"
    pgrep -f "$process_name" > /dev/null
}

log() {
    echo "$(date)" $* >> $sysdir/logs/network.log
}

main "$@"
