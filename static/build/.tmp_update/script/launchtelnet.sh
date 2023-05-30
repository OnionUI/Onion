# This file will launch telnet with the following env vars to set the time and date for the session (which fixes telnet times on file operations)
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

telnetd -l sh &