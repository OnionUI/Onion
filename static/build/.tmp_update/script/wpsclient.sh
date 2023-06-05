sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
icondir=$sysdir/res
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export WPACLI=/customer/app/wpa_cli
export IMGPOP=$sysdir/bin/imgpop
# Syntax: ./imgpop duration delay image_path x_position y_position.

start_udhcpc(){
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

wpsfail() {
$IMGPOP 5 0 "$icondir/wpsfail.png" 84 428 > /dev/null 2>&1 &
}

wpstrying() {
count=0
    while [ $count -lt 4 ]; do
        if [ $((count % 2)) -eq 0 ]; then
            $IMGPOP 1 0 "$icondir/wpstrying.png" 215 433 > /dev/null 2>&1 &
        else
            $IMGPOP 1 0 "$icondir/wpsfail.png" 215 433 > /dev/null 2>&1 &
        fi
		sleep 1
        count=$((count + 1))
		killall -9 imgpop
    done &
}

wpsconnected() {
$IMGPOP 10 0 "$icondir/wpsconnected.png" 215 433 > /dev/null 2>&1 &
}

log() {
    echo "$(date)" $* >> $sysdir/logs/network.log
}

main() {
ifconfig wlan0 down
ifconfig wlan0 up
kill_udhcpc
$WPACLI disable_network all > /dev/null 2>&1 &	
log "WPS: Disconnecting from current network"
$WPACLI wps_pbc 
log "WPS: Trying to connect to WPS host"
start_udhcpc 

start_time=$(date +%s)

while true; do
    IP=$(ip route get 1 2>/dev/null | awk '{print $NF;exit}')

    if [ -z "$IP" ]; then
        wpstrying
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

main &