# Shared netplay helpers

checksize_func() {
    if [ "$file_check_size" -eq 1 ]; then
        if [ -e "$file_path" ]; then
            local_file_size=$(stat -c%s "$file_path")
            if [ "$remote_file_checksum" == "0" ]; then
                same_size=1
            elif [ "$local_file_size" -eq "$remote_file_checksum" ]; then
                same_size=1
            else
                same_size=0
            fi
        else
            same_size=0
        fi
    else
        same_size=1
    fi
}

checksum_func() {
    local_file_size=$(stat -c%s "$file_path")
    local func_file_path="$1"
    local CRC="$2"

    ########################## File checksum check : same_chksum = 0 different, 1 identical , 2 unknown

    if [ "$CRC" != "0" ]; then # file_checksum=0 means skip the difference check = always replace
        local_file_checksum=$(xcrc "$func_file_path")

        if [ "$local_file_size" -gt "$MAX_FILE_CHKSUM_SIZE" ]; then
            log "File size too big for checksum: it would be too long"
            same_chksum=2
        else
            if [ "$CRC" == "$local_file_checksum" ]; then
                same_chksum=1
            else
                same_chksum=0
            fi
        fi
    else
        same_chksum=0
    fi
}

enable_flag() {
    flag="$1"
    touch "$sysdir/config/.$flag"
}

flag_enabled() {
    flag="$1"
    [ -f "$sysdir/config/.$flag" ]
}

is_running() {
    process_name="$1"
    pgrep "$process_name" >/dev/null
}

notify_stop() {
    notify_peer "stop_now"
    sleep 2
    cleanup
}

ready_up() {
    check_stop
    ping -c 5 $client_ip >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        notify_peer "host_ready"
    else
        build_infoPanel_and_log "Error" "No connectivity to $client_ip, \n is the client still connected?"
        notify_stop
    fi
}

restore_ftp() {
    log "Restoring original FTP server"
    killall -9 tcpsvd
    if flag_enabled ftpState; then
        if flag_enabled authftpState; then
            bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpdauth.conf &
        else
            bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf &
        fi
    fi
}

udhcpc_control() {
    if pgrep udhcpc >/dev/null; then
        killall -9 udhcpc
    fi
    sleep 1
    udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
}

url_encode() {
    local string="$1"
    local length="${#string}"
    local encoded=""
    local pos c o

    for ((pos = 0; pos < length; pos++)); do
        c=${string:$pos:1}
        case "$c" in
        [-_.~a-zA-Z0-9]) o="$c" ;;
        *) printf -v o '%%%02x' "'${c}" ;;
        esac
        encoded+="$o"
    done
    echo "$encoded"
}

wait_for_client() {
    check_stop
    build_infoPanel_and_log "Hotspot" "Waiting for a client to connect..."

    client_ip=""
    client_mac=""
    counter=0

    killall -9 wpa_supplicant
    killall -9 udhcpc

    sleep 1

    while true; do
        sta_list=$($sysdir/bin/hostapd_cli all_sta 2>/dev/null)
        $sysdir/bin/hostapd_cli all_sta flush

        if [ $? -ne 0 ]; then
            build_infoPanel_and_log "Hotspot" "Hostapd hook failing, retrying."
            counter=$((counter + 1))
        fi

        if [ ! -z "$sta_list" ]; then
            client_mac=$(echo "$sta_list" | awk 'NR==2{print $1; exit}')
            client_ip=$(arp -an | awk '/"'"$client_mac"'"/ {gsub(/[\(\)]/,""); print $2}')

            if [ ! -z "$client_ip" ]; then
                case "$client_ip" in
                192.168.100.*)
                    log "$sta_list"
                    log "A client has connected. IP: $client_ip"
                    build_infoPanel_and_log "Hotspot" "A client has connected! \n IP: $client_ip"
                    break
                    ;;
                esac
            fi
        fi

        sleep 1
        counter=$((counter + 1))

        if [ $counter -ge 30 ]; then
            log "No client has connected"
            build_infoPanel_and_log "Hotspot error" "No client has connected. Exiting..."
            cleanup
        fi
    done

    sleep 1
    log "$client_ip has joined the hotspot"
}

wait_for_host() {
    local counter=0

    build_infoPanel_and_log "Ready" "Waiting for host to ready up"
    while true; do
        sync
        check_stop
        for file in /tmp/host_ready; do
            if [ -f "$file" ]; then
                build_infoPanel_and_log "Message from host" "Setup complete"
                rm /tmp/host_ready # be ready for the second use of host_ready flag
                break 2
            fi
        done

        sleep 1
        counter=$((counter + 1))

        if [ $counter -ge 25 ]; then
            build_infoPanel_and_log "Error" "The host didn't ready up, cannot continue..."
            notify_stop
        fi
    done
}
