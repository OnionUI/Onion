sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
icondir=$sysdir/res
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export WPACLI=/customer/app/wpa_cli
export IMGPOP=$sysdir/bin/imgpop
# Syntax: ./imgpop duration delay image_path x_position y_position.

main() {
    if ifconfig wlan0 &> /dev/null; then
        if is_running wpa_supplicant && is_running udhcpc; then
            wifiup
        fi
        sleep 1
        log "WPS: Wi-Fi is up"
        killall -9 imgpop
        break
    else
        wifiquery
        log "WPS: Wi-Fi disabled, trying to enable before connecting.."
        /customer/app/axp_test wifion
        sleep 2
        ifconfig wlan0 up
        sleep 1
        wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
        sleep 2
        killall -9 imgpop
        sed -i 's/"wifi":\s*0/"wifi": 1/' /mnt/SDCARD/system.json # tell mainui that wifi needs to be kept up once started

        if is_running wpa_supplicant; then
            wifiup
            sleep 1
        fi

        touch /tmp/dont_restart_wifi
        sync
    fi

    start_udhcpc
    $WPACLI disable_network all > /dev/null 2>&1 & # disconnect any existing networks
    log "WPS: Disconnecting from current network"
    $WPACLI wps_pbc # start wps
    log "WPS: Trying to connect to WPS host"

    start_time=$(date +%s)

    while true; do
        IP=$(ip route get 1 2> /dev/null | awk '{print $NF;exit}')

        if [ -z "$IP" ]; then
            wpsflicker
            sleep 5

            current_time=$(date +%s)
            elapsed_time=$((current_time - start_time))

            if [ $elapsed_time -gt 30 ]; then
                if [ -z "$IP" ]; then
                    wpsfail
                    log "WPS: Failed to connect.."
                    sleep 5
                    exit
                else
                    wpsconnected
                fi
            fi
        else
            break
        fi
    done

    killall -9 imgpop
    log "WPS: Connected!"
    wpsconnected
    sleep 2
    killall -9 imgpop
    exit
}

start_udhcpc() {
    udhcpc -i wlan0 -s /etc/init.d/udhcpc.script > /dev/null 2>&1 &
}

kill_udhcpc() {
    if pgrep udhcpc > /dev/null; then
        killall -9 udhcpc
    fi
}

conn_cleanup() {
    kill_udhcpc
    start_udhcpc
}

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

wifiquery() {
    $IMGPOP 5 0 "$icondir/wifiquery.png" 84 428 > /dev/null 2>&1 &
}

wifiup() {
    $IMGPOP 5 0 "$icondir/wifiup.png" 84 428 > /dev/null 2>&1 &
}

wpsfail() {
    $IMGPOP 5 0 "$icondir/wpsfail.png" 84 428 > /dev/null 2>&1 &
}

wpstrying() {
    $IMGPOP 1 0 "$icondir/wpstrying.png" 84 428 > /dev/null 2>&1 &
}

wpsconnected() {
    $IMGPOP 10 0 "$icondir/wpsconnected.png" 84 428 > /dev/null 2>&1 &
}

wpsflicker() {
    count=0
    while [ $count -lt 4 ]; do
        if [ $((count % 2)) -eq 0 ]; then
            wpstrying
        else
            wpsfail
        fi
        sleep 1
        count=$((count + 1))
        killall -9 imgpop
    done &
}

log() {
    echo "$(date)" $* >> $sysdir/logs/network.log
}

main &
