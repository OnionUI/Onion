## Add your menulabel and tooltip for tweaks below, don't put them at the top of the file.

menulabel="System log snapshot"
tooltip="Take a snapshot of the system logs \n and export them to SD:log_export.7z \n \n Attach these logs to your issue \n or send directly to a dev"

# Tweaks diags script to pull a system snapshot

##################
## SETUP ##
##################
sysdir="/mnt/SDCARD/.tmp_update"
workingdir="$sysdir/logdump"
diagsdir="$sysdir/script/diagnostics"

mkdir -p "$workingdir/sysinfo" "$workingdir/directories" "$workingdir/network" "$workingdir/ra_cfg" "$sysdir/logdump"
sysinfo_file="$workingdir/sysinfo/sys_summary.log"
networkinfo_file="$workingdir/network/net_summary.log"
ra_cfg_file="/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
backup_cfg_file="/mnt/SDCARD/RetroArch/.retroarch/backup_retroarch.cfg"
dnsmasq_conf_path="$sysdir/config/dnsmasq.conf"
wpa_conf_path="/appconfigs/wpa_supplicant.conf"
base_dir="/mnt/SDCARD"
log_dir="$workingdir/directories" # grabs directory listings for SD dirs
sync

##################
## LOGGING FUNCTION ##
##################

## Source global utils
logfile=util_snapshot
. $sysdir/script/log.sh
program=$(basename "$0" .sh)

##################
## MAIN ##
##################

main() {
    log "Generating system snapshot, please wait"
    snapshot
    log "Starting exporter"
    $diagsdir/util_exporter.sh
}

##################
## GROUPS FOR FUNCS ##
##################

snapshot() {
    system_healthcheck
    wifi_healthcheck
    # export_ra_cfg
    create_dir_logs
    create_appcfg_list
    log "Snapshot generated"
    sleep 0.5
}

##################
## WRITER/MOVER ##
##################

write_info() {
    write_header "$1" $3
    echo -e "$1\n" >> $3
    log "Executing command $2 and appending output to $3"
    eval $2 >> $3
    echo -e "\n\n" >> $3
    log "Finished writing information for $1 to $3"
}

write_header() {
    echo -e "############################ $1 ############################\n\n" >> $2
}

##################
## SYSTEM LOGS ##
##################

get_lcd_voltage() { # Check LCD voltage incase it's been changed by user
    conf_dir="$sysdir/config/.lcdvolt"
    enabled="$conf_dir"
    disabled="${conf_dir}_"

    if [ -f "$enabled" ]; then
        registerValueHex=$(cat "$enabled")
    elif [ -f "$disabled" ]; then
        registerValueHex=$(cat "$disabled")
    else
        echo "No LCD voltage file found."
        return 1
    fi

    if [ -z "$registerValueHex" ]; then
        echo "LCD voltage config file is empty."
        return 1
    fi

    registerValueDec=$(printf "%d" "0x$registerValueHex")
    voltage_tenths=$((16 + registerValueDec))
    echo "$((voltage_tenths / 10)).$((voltage_tenths % 10)) volts."
}

actual_uptime() {
    uptime=$(cut -d. -f1 /proc/uptime)

    uptime_days=$((uptime / 60 / 60 / 24))
    uptime_hours=$((uptime / 60 / 60 % 24))
    uptime_minutes=$((uptime / 60 % 60))
    uptime_seconds=$((uptime % 60))

    echo "Uptime: $uptime_days days, $uptime_hours hours, $uptime_minutes minutes, $uptime_seconds seconds"
}

system_healthcheck() {
    log "Generating system healthcheck"
    write_info "Onion version:" "cat $sysdir/onionVersion/version.txt" $sysinfo_file
    write_info "Firmware version:" "/etc/fw_printenv miyoo_version" $sysinfo_file
    write_info "Device model:" "cat /tmp/deviceModel" $sysinfo_file
    write_info "Charging/battery state:" "/customer/app/axp_test" $sysinfo_file
    write_info "System uptime:" "actual_uptime" $sysinfo_file
    write_info "CPU Information:" "cat /proc/cpuinfo" $sysinfo_file
    write_info "CPU Temperature:" "cat /sys/devices/virtual/mstar/msys/TEMP_R" $sysinfo_file
    write_info "RAM Information:" "cat /proc/meminfo" $sysinfo_file
    write_info "Disk Space Information:" "df -h" $sysinfo_file
    write_info "Mount points" "mount" $sysinfo_file
    write_info "System swapfile" "cat /proc/swaps" $sysinfo_file
    write_info "List of Running Processes:" "ps aux" $sysinfo_file
    write_info "LCD Voltage:" "get_lcd_voltage" $sysinfo_file
    write_info "Framebuffer info:" "cat /proc/mi_modules/fb/mi_fb0" $sysinfo_file
    write_info "More framebuffer info:" "fbset" $sysinfo_file
    write_info "System.json state:" "cat /mnt/SDCARD/system.json" $sysinfo_file
    write_info "Keymap state:" "cat $sysdir/config/keymap.json" $sysinfo_file
    write_info "Config folder dump" "ls -alhR $sysdir/config/" $sysinfo_file
    dmesg > "$workingdir/sysinfo/dmesg.log"
    log "Finished generating healthcheck"
}

##################
## NETWORKING ##
##################

get_netserv_status() { # Just checks if these exist - will need upkeep in this format
    log "Checking system services"
    services=".smbdState_ .authftpState_ .authhttpState_ .authsshState_ .ftpState_ .hotspotState_ .httpState_ .ntpState_ .smbdState_ .sshState_ .telnetState_"

    for file in $services; do
        if [ -e "$sysdir/config/$file" ]; then
            status="${file%State_}"
            echo "$status:     Disabled"
        else
            status="${file%State_}"
            echo "$status:     Enabled"
        fi
    done
}

#We can't grab the contents wpa_supplicant.conf file (for situations where people have issues with wifi) as we'll pull peoples passwords. But we can check whether the file exists and contains the headers needed to function correctly
# If this returns as failed, it means the wpa_supplicant.conf file is invalid and will cause wpa_supplicant to error out when it's called.

wpa_supplicant_health_check() {
    log "Running wpa_supplicant health checks"
    if [[ -f "$wpa_conf_path" ]]; then
        echo "File $wpa_conf_path exists."
        if grep -Fxq "ctrl_interface=/var/run/wpa_supplicant" "$wpa_conf_path" && grep -Fxq "update_config=1" "$wpa_conf_path"; then
            echo "Wpa_supplicant.conf contains the correct info"
        else
            echo "Wpa_supplicant.conf is missing the required headers"
        fi
    else
        echo "Wpa_supplicant.conf is not present"
    fi
}

wpa_supplicant_contains_networks() {
    if [[ -f "$wpa_conf_path" ]]; then
        num_ssids=$(grep -c "ssid=" "$wpa_conf_path")
        num_disabled=$(grep -c "disabled=" "$wpa_conf_path")

        echo "$num_ssids SSIDs configured, $num_disabled SSIDs disabled"
    else
        echo "Wpa_supplicant.conf is not present"
    fi
}

get_wpa_supplicant() { # print the file aswell but remove the users ssid/psk - worth doing a full formatting check on the file incase syntax is broken by someone manually editing.
    if [ ! -f "$wpa_conf_path" ]; then
        echo "File not found: $wpa_conf_path"
    fi

    sed 's/ssid="[^"]*"/ssid="redacted"/g; s/psk="[^"]*"/psk="redacted"/g' "$wpa_conf_path"
}

check_hostapd_conf() {
    log "Checking hostapd.conf"
    file_path="$sysdir/config/hostapd.conf"
    file_size=$(stat -c%s "$file_path")

    if [[ "$file_size" -eq 245 ]]; then
        echo "Filesize matches; hostapd.conf has not been modified."
    else
        echo "Mismatch; hostapd.conf has been modified."
    fi
}

check_dnsmasq_conf() { # This file can either be 251 or 187 depending on the state of .logging. (In a future feature where the dnsmasq log entry is removed)
    log "Checking dnsmasq.conf"
    file_path="$sysdir/config/dnsmasq.conf"
    file_size=$(stat -c%s "$file_path")

    if [[ "$file_size" -eq 251 || "$file_size" -eq 187 ]]; then
        echo "Filesize matches; dnsmasq.conf has not been modified."
    else
        echo "Mismatch; dnsmasq.conf has been modified."
    fi
}

wifi_healthcheck() {
    log "Generating wifi healthcheck"
    write_info "Network service:" "get_netserv_status" $networkinfo_file
    write_info "Checking size of hostapd.conf:" "check_hostapd_conf" $networkinfo_file
    write_info "Checking size of dnsmasq.conf" "check_dnsmasq_conf" $networkinfo_file
    write_info "Wpa_supplicant.conf health:" "wpa_supplicant_health_check" $networkinfo_file
    write_info "Wpa_supplicant.conf ssid count:" "wpa_supplicant_contains_networks" $networkinfo_file
    write_info "Wpa_supplicant.conf sanitised dump:" "get_wpa_supplicant" $networkinfo_file
    write_info "WiFi Status:" "ifconfig" $networkinfo_file
    write_info "Internet Access State:" "ping -q -c 3 -W 1 google.com >/dev/null && echo The Internet is up. || echo The Internet seems down." $networkinfo_file
    write_info "WiFi connection health:" "cat /proc/net/wireless" $networkinfo_file
    write_info "WiFi Adaptors detailed:" "iw phy" $networkinfo_file
    log "Finished generating wifi healthcheck"
}

##################
## FILES AND DIRECTORIES ##
##################

export_ra_cfg() {
    cp $ra_cfg_file $workingdir/ra_cfg/retroarch_config.cfg
}

create_appcfg_list() {
    appcfg_dir="/appconfigs"
    log "Generating directory listing for $appcfg_dir"

    if [ -d "$appcfg_dir" ]; then
        log_file="$log_dir/appconfigs.log"
        echo "Directory Listing of $appcfg_dir:" > "$log_file"
        ls -alhR "$appcfg_dir" >> "$log_file"
        log "Directory listing for $appcfg_dir has been saved to $log_file"
    else
        log "Directory $appcfg_dir does not exist"
    fi
}

create_dir_logs() { # Currently creates a list of roms which can take a while depending on the number of roms.
    log "Generating directory listings"
    for dir in "$base_dir"/*; do
        if [ -d "$dir" ]; then
            dir_name=$(basename "$dir")
            log_file="$log_dir/$dir_name.log"
            echo "Directory Listing of $dir:" > "$log_file"
            ls -alhR "$dir" >> "$log_file"
            log "Directory listing for $dir has been saved to $log_file"
        fi
    done
}

##################
## CALL MAIN FUNCTION ##
##################

main "$@"
