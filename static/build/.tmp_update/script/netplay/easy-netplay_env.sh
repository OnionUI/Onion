# Shared netplay environment setup

sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo

LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export LD_LIBRARY_PATH

hostip="${hostip:-192.168.100.100}" # default unless overridden by caller
peer_ip="${peer_ip:-$hostip}"
export hostip peer_ip

# turn off power save on the wifi - this is turned back on in the netplay_cleanup function
iw wlan0 set power_save off
iw wlan1 set power_save off