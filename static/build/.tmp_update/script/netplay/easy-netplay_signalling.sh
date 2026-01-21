# Netplay signalling helpers

# notify_peer [target_ip] <flag>
# - uses: peer_ip if target_ip is omitted
# - sends /tmp/<flag> via FTP to peer
notify_peer() {
    local target_ip=""
    local notify_flag=""

    if [ $# -eq 1 ]; then
        target_ip="$peer_ip"
        notify_flag="$1"
    else
        target_ip="$1"
        notify_flag="$2"
    fi

    if [ -z "$target_ip" ]; then
        log "No peer IP set for notify_peer ($notify_flag)."
        return 1
    fi

    local notify_file="/tmp/$notify_flag"
    touch "$notify_file"
    sync
    curl -T "$notify_file" "ftp://${target_ip}/${notify_file}" >/dev/null 2>&1 # the first / after the IP must be not encoded

    if [ $? -eq 0 ]; then
        log "Successfully transferred $notify_file to ftp://${target_ip}/${notify_file}"
    else
        log "Failed to transfer $notify_file to ftp://${target_ip}/${notify_file}"
    fi
}

# check_stop [message]
# - uses: /tmp/stop_now sentinel to abort
check_stop() {
    local message="$1"
    sync
    if [ -e "/tmp/stop_now" ]; then
        build_infoPanel_and_log "Message from client" "${message:-The client has had a problem joining the session.}"
        sleep 2
        cleanup
    fi
}

# notify_stop
# - best-effort notify peer and cleanup
notify_stop() {
    # notify_peer: best-effort stop signal to peer
    notify_peer "stop_now" >/dev/null 2>&1
    sleep 2
    cleanup
}

# ready_up
# - pings client_ip then signals host_ready
ready_up() {
    # check_stop: client reported a setup/join issue
    check_stop "The client has had a problem joining the session."
    ping -c 5 $client_ip >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        # notify_peer: signal host ready to client
        notify_peer "$client_ip" "host_ready"
    else
        build_infoPanel_and_log "Error" "No connectivity to $client_ip, \n is the client still connected?"
        notify_stop
    fi
}

# wait_for_client
# - waits for client connection on hotspot
# - sets: client_ip, peer_ip
wait_for_client() {
    # check_stop: client reported a setup/join issue
    check_stop "The client has had a problem joining the session."
    build_infoPanel_and_log "Hotspot" "Waiting for a client to connect..."
    log "wait_for_client: start (sysdir=$sysdir)"

    client_ip=""
    client_mac=""
    counter=0
    lease_file=$(grep -E '^dhcp-leasefile=' "$sysdir/config/dnsmasq.conf" 2>/dev/null | head -n 1 | cut -d'=' -f2)

    killall -9 wpa_supplicant
    killall -9 udhcpc

    sleep 1

    while true; do
        sta_list=$($sysdir/bin/hostapd_cli all_sta 2>/dev/null)
        $sysdir/bin/hostapd_cli all_sta flush

        log "wait_for_client: sta_list='${sta_list}'"

        if [ $? -ne 0 ]; then
            build_infoPanel_and_log "Hotspot" "Hostapd hook failing, retrying."
            counter=$((counter + 1))
        fi

        if [ ! -z "$sta_list" ]; then
            client_mac=$(printf '%s\n' "$sta_list" | awk 'NR==2{print $1; exit}')
            client_mac=$(printf '%s' "$client_mac" | tr -cd '0-9a-fA-F:' | tr 'A-F' 'a-f')
            log "wait_for_client: client_mac='${client_mac}'"
            if [ -n "$client_mac" ] && [ -n "$lease_file" ] && [ -f "$lease_file" ]; then
                sync
                lease_dump=$(cat "$lease_file" 2>/dev/null)
                client_ip=$(printf '%s\n' "$lease_dump" | awk -v mac="$client_mac" 'tolower($2)==tolower(mac){print $3; exit}')
            fi
            if [ -z "$client_ip" ]; then
                arp_dump=$(arp -an 2>/dev/null)
                if [ -n "$client_mac" ]; then
                    client_ip=$(printf '%s\n' "$arp_dump" | awk -v mac="$client_mac" 'tolower($0) ~ tolower(mac) {gsub(/[()]/,"",$2); print $2; exit}')
                fi
            fi
            log "wait_for_client: client_ip='${client_ip}'"

            if [ ! -z "$client_ip" ]; then
                case "$client_ip" in
                192.168.100.*)
                    log "$sta_list"
                    log "A client has connected. IP: $client_ip"
                    build_infoPanel_and_log "Hotspot" "A client has connected! \n IP: $client_ip"
                    peer_ip="$client_ip"
                    sync
                    break
                    ;;
                esac
            fi
        fi

        sleep 0.5
        counter=$((counter + 1))

        # wait for 15 seconds, then bail
        if [ $counter -ge 30 ]; then
            log "No client has connected"
            build_infoPanel_and_log "Hotspot error" "No client has connected. Exiting..."
            cleanup
        fi
    done

    sleep 1
    log "$client_ip has joined the hotspot"
}

# wait_for_host
# - waits for /tmp/host_ready signal
wait_for_host() {
    build_infoPanel_and_log "Ready" "Waiting for host to start game"
    while true; do
        sync
        # check_stop: host reported a setup issue
        check_stop "The host has had a problem setting up the session"
        for file in /tmp/host_ready; do
            if [ -f "$file" ]; then
                build_infoPanel_and_log "Message from host" "Setup complete"
                rm /tmp/host_ready # be ready for the second use of host_ready flag
                break 2
            fi
        done

        sleep 1
    done
}
