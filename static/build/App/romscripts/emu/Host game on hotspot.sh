# GLO HOST
# Script to:
# 	Start hotspot, 
# 	Create a cookie file containing details for the client, 
# 	Start FTP to be able to host this file, 
# 	Start RA as a netplay host with -H, the core path and the rom path.
# 	Leave WPS commented out until it's further tested, it works but leads to an unstable connection. Possible issue with the new hostapd binary
# Used within GLO as an addon script

# Env setup
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"

##########
##Setup.##
##########

# We'll need wifi up for this. Lets try and start it..

check_wifi(){
if ifconfig wlan0 &>/dev/null; then
	sleep 1
	log "GLO::Retro_Quick_Host: Wi-Fi is up already"
	break
else
	log "GLO::Retro_Quick_Host: Wi-Fi disabled, trying to enable before connecting.."
	/customer/app/axp_test wifion
	sleep 2
	ifconfig wlan0 up
	conn_cleanup
	$miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf
	sleep 2
fi
}

# We'll need hotspot to host the local connection
start_hotspot() { 
	ifconfig wlan1 up 
	ifconfig wlan0 down # Put wlan0 down to suspend the current wifi connections
		
	hotspot0addr=$(grep -E '^dhcp-range=' "$sysdir/config/dnsmasq.conf" | cut -d',' -f1 | cut -d'=' -f2) 
	hotspot0addr=$(echo $hotspot0addr | awk -F'.' -v OFS='.' '{$NF-=1; print}') 
	gateway0addr=$(grep -E '^dhcp-option=3' $sysdir/config/dnsmasq.conf | awk -F, '{print $2}')
	subnetmask=$(grep -E '^dhcp-range=.*,[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+,' "$sysdir/config/dnsmasq.conf" | cut -d',' -f3) 
	
	ifconfig wlan1 $hotspot0addr netmask $subnetmask 
	
	$sysdir/bin/hostapd -P /var/run/hostapd.pid -B -i wlan1 $sysdir/config/hostapd.conf &
	$sysdir/bin/dnsmasq --conf-file=$sysdir/config/dnsmasq.conf -u root & 
	ip route add default via $gateway0addr
	
	if is_running hostapd; then
		log "GLO::Retro_Quick_Host: Started with IP of: $hotspot0addr, subnet of: $subnetmask"
	else
		log "GLO::Retro_Quick_Host: Failed to start, please try turning off/on. If this doesn't resolve the issue reboot your device."
		disable_flag HotspotState
	fi
}

# We'll need FTP to host the cookie to the client
start_ftp(){
if is_running bftpd; then
	log "GLO::Retro_Quick_Host: FTP already running, killing to rebind"
	killall -9 bftpd
	bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf
else
	bftpd -d -c /mnt/SDCARD/.tmp_update/config/bftpd.conf
	log "GLO::Retro_Quick_Host: Starting FTP server"
fi
}

# We'll need WPS to make a hands off connection (even if the user has changed the hotspot password from the default)
# start_wps(){
	# touch /tmp/hostapd.psk
	# sync
	# sleep 1
	# $sysdir/bin/hostapd_cli wps_pbc
	# log "GLO::Retro_Quick_Host: Starting WPS host"
# }

# We'll start Retroarch in host mode with -H with the core and rom paths loaded in.
# start_retroarch(){
# Currently not used, need to build the retroarch start process with -H to host netplay, the core dir and the rom path (aswell as -v -L)
# Rom paths with spaces uses ""
# }

# Create a cookie, the client will attempt to grab this file via FTP to allow us to setup the retro connection on there
create_cookie(){
cookiefile="/mnt/SDCARD/retroarch_cookie.txt"

pid=$(pgrep retroarch)

if [ -z "$pid" ]; then
  echo "GLO::Retro_Quick_Host: process not found."
  exit 1
fi

cmdline=$(cat "/proc/$pid/cmdline" | tr '\0' ' ')

rom=$(echo "$cmdline" | awk '{print $5$6$7$8$9$10}') # concat as many entries as possible as some roms have spaces in
core=$(echo "$cmdline" | awk '{print $4}')

echo "[core]: /mnt/SDCARD/Retroarch/$core" >> $cookiefile
echo "[rom]: $rom" >> $cookiefile

log "GLO::Retro_Quick_Host: Created cookie file for the client"
}

###########
#Utilities#
###########

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

is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

log() {
    echo "$(date)" $* >> $sysdir/logs/ra_quick_host.log
}

#########
##Main.##
#########

lets_go(){
check_wifi
start_hotspot
start_ftp
# start_wps
# start_retroarch ???
create_cookie
}

lets_go