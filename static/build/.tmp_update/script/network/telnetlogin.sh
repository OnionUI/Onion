# This file will launch telnet with the following env vars following a successful login & set the time and date for the session (which fixes telnet times on file operations)
# hwclock may still need to be synced with hwclock -w
# It will only run with these vars when telnet has been manually started through tweaks (telnet at boot will not have these vars)
# It should also pull the TZ from the process that starts it (in the usual case, update_networking.sh)

export sysdir="/mnt/SDCARD/.tmp_update"
export miyoodir="/mnt/SDCARD/miyoo"
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte:/sbin:/usr/sbin:/bin:/usr/bin"
export PATH="$sysdir/bin:$PATH"
export USER=root
export TERM=vt102
export SHELL=/bin/sh
export PWD=/mnt/SDCARD
export HOME=/mnt/SDCARD/
cd /mnt/SDCARD

tr -d '\r' < /mnt/SDCARD/.tmp_update/config/.auth.txt > /tmp/.auth_tmp.txt

read -r username password _ < /tmp/.auth_tmp.txt

rm /tmp/.auth_tmp.txt

echo -n "Login: " && read entered_username
echo -n "Password: " && read -s entered_password

if [ "$entered_username" = "$username" ] && [ "$entered_password" = "$password" ]; then
	echo -e "Successful login - Welcome to Onion.\n"
    /bin/sh
else
    echo "Login failed"
	sleep 1
    exit 1
fi