## Add your menulabel and tooltip for tweaks below, don't put them at the top of the file.

menulabel="System log snapshot"
tooltip="Take a snapshot of the \n system logs and export them \n to SD:logdump.7z"

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

log() {
    echo "$1" > /tmp/log_status
}

##################
## MAIN ##
##################

main() {
     log "Generating system snapshot, please wait"
     snapshot
     $diagsdir/util_exporter.sh
}

##################
## GROUPS FOR FUNCS ##
##################

snapshot() {
    system_healthcheck
    wifi_healthcheck
    export_ra_cfg
    create_dir_logs
    log "Snapshot generated"
    sleep 0.5
}

##################
## WRITER/MOVER ##
##################

write_info() {
    write_header "$1" $3
    echo -e "$1\n" >> $3
    eval $2 >> $3
    echo -e "\n\n" >> $3
}

write_header() {
    echo -e "############################ $1 ############################\n\n" >> $2
}

##################
## SYSTEM LOGS ##
##################

actual_uptime() {
    uptime=$(cut -d. -f1 /proc/uptime)
    
    uptime_days=$((uptime / 60 / 60 / 24))
    uptime_hours=$((uptime / 60 / 60 % 24))
    uptime_minutes=$((uptime / 60 % 60))
    uptime_seconds=$((uptime % 60))
    
    echo "Uptime: $uptime_days days, $uptime_hours hours, $uptime_minutes minutes, $uptime_seconds seconds"
}

system_healthcheck () {
    write_info "Onion version:" "cat $sysdir/onionVersion/version.txt" $sysinfo_file
    write_info "Firmware version:" "/etc/fw_printenv miyoo_version" $sysinfo_file
    write_info "Device model:" "cat /tmp/deviceModel" $sysinfo_file
    write_info "Charging/battery state:" "/customer/app/axp_test" $sysinfo_file
    write_info "System uptime:" "actual_uptime" $sysinfo_file
    write_info "CPU Information:" "cat /proc/cpuinfo" $sysinfo_file
    write_info "CPU Temperature:" "cat /sys/devices/virtual/mstar/msys/TEMP_R" $sysinfo_file
    write_info "RAM Information:" "cat /proc/meminfo" $sysinfo_file
    write_info "Disk Space Information:" "df -h" $sysinfo_file
    write_info "System swapfile" "cat /proc/swaps" $sysinfo_file
    write_info "List of Running Processes:" "ps aux" $sysinfo_file
    write_info "Framebuffer info:" "cat /proc/mi_modules/fb/mi_fb0" $sysinfo_file
    write_info "More framebuffer info:" "fbset" $sysinfo_file
    write_info "System.json state:" "cat /appconfigs/system.json" $sysinfo_file
    write_info "Keymap state:" "cat $sysdir/config/keymap.json" $sysinfo_file    
    write_info "Config folder dump" "ls -alhR $sysdir/config/" $sysinfo_file 
}

##################
## NETWORKING ##
##################

get_netserv_status() { # Just checks if these exist - will need upkeep in this format
    services=".smbdState_ .authsmbdState_ .authftpState_ .authhttpState_ .authsshState_ .ftpState_ .hotspotState_ .httpState_ .ntpState_ .smbdState_ .sshState_ .telnetState_"

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

check_hostapd_conf() {
    file_path="$sysdir/config/hostapd.conf"
    file_size=$(stat -c%s "$file_path")

    if [[ "$file_size" -eq 245 ]]; then
        echo "Filesize matches; hostapd.conf has not been modified."
    else
        echo "Mismatch; hostapd.conf has been modified."
    fi
}

check_dnsmasq_conf() { # This file can either be 251 or 187 depending on the state of .logging. 
    file_path="$sysdir/config/dnsmasq.conf"
    file_size=$(stat -c%s "$file_path")

    if [[ "$file_size" -eq 251 || "$file_size" -eq 187 ]]; then
        echo "Filesize matches; dnsmasq.conf has not been modified."
    else
        echo "Mismatch; dnsmasq.conf has been modified."
    fi
}

wifi_healthcheck() {
    
    write_info "Network service:" "get_netserv_status" $networkinfo_file
    write_info "Checking size of hostapd.conf:" "check_hostapd_conf" $networkinfo_file
    write_info "Checking size of dnsmasq.conf" "check_dnsmasq_conf" $networkinfo_file
    write_info "Wpa_supplicant.conf health:" "wpa_supplicant_health_check" $networkinfo_file 
    write_info "Wpa_supplicant.conf ssid count:" "wpa_supplicant_contains_networks" $networkinfo_file
    write_info "WiFi Status:" "ifconfig" $networkinfo_file
    write_info "Internet Access State:" "ping -q -c 1 -W 1 google.com >/dev/null && echo The Internet is up. || echo The Internet seems down." $networkinfo_file
    write_info "WiFi Adaptors detailed:" "iw phy" $networkinfo_file
    write_info "WiFi connection health:" "cat /proc/net/wireless" $networkinfo_file

}

##################
## FILES AND DIRECTORIES ##
##################

export_ra_cfg() {
    cp $ra_cfg_file $workingdir/ra_cfg/retroarch_config.cfg
}

create_dir_logs() { # Currently creates a list of roms which can take a while depending on the number of roms.
    for dir in "$base_dir"/*; do
        if [ -d "$dir" ]; then
            dir_name=$(basename "$dir")
            log_file="$log_dir/$dir_name.log"
            echo "Directory Listing of $dir:" > "$log_file"
            ls -alhR "$dir" >> "$log_file"
        fi
    done
}
  
##################
## CALL MAIN FUNCTION ##
##################

main "$@"