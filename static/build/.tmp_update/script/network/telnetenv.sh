# This file will launch telnet with the following env vars following without the need to login.
# hwclock may still need to be synced with hwclock -w
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

echo -e "Welcome to Onion\n "
/bin/sh