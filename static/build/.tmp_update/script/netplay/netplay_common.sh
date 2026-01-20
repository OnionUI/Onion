# Shared netplay helpers

# checksize_func <file_path> <remote_size>
# - sets: same_size (0 different, 1 identical, 2 unknown)
checksize_func() {
    local func_file_path="${1:-$file_path}"
    local filesize_tocheck="${2:-$remote_file_size}"

    if [ -e "$func_file_path" ]; then
        local_file_size=$(stat -c%s "$func_file_path")
        if echo "$filesize_tocheck" | grep -q "^[0-9][0-9]*$"; then
            if [ "$filesize_tocheck" -eq "$local_file_size" ]; then
                same_size=1
            else
                same_size=0
            fi
        else
            log "Non-numeric remote file size for checksize_func: '$filesize_tocheck'"
            same_size=2
        fi
    else
        same_size=0
    fi
}

# checksum_func <file_path> <crc>
# - uses: MAX_FILE_CHKSUM_SIZE
# - sets: same_chksum
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
        log "Skipping checksum check."
        same_chksum=1
    fi
}

# enable_flag <flag_name>
# - creates $sysdir/config/.<flag_name>
enable_flag() {
    flag="$1"
    touch "$sysdir/config/.$flag"
}

# disable_flag <flag_name>
# - removes $sysdir/config/.<flag_name>
disable_flag() {
    flag="$1"
    mv "$sysdir/config/.$flag" "$sysdir/config/.$flag_"
}

# flag_enabled <flag_name>
# - returns 0 if flag file exists
flag_enabled() {
    flag="$1"
    [ -f "$sysdir/config/.$flag" ]
}

# is_running <process_name>
# - returns 0 if process is running
is_running() {
    process_name="$1"
    pgrep "$process_name" >/dev/null
}

# build_infoPanel_and_log <title> <message>
# - uses: INFOPANEL_SLEEP (default 0.3s)
# - shows persistent infoPanel and logs message
build_infoPanel_and_log() {
    local title="$1"
    local message="$2"
    local delay="${INFOPANEL_SLEEP:-0.3}"

    log "Info Panel: \n\tStage: $title\n\tMessage: $message"
    if is_running infoPanel; then
        killall -9 infoPanel
    fi
    infoPanel --title "$title" --message "$message" --persistent &
    sync
    touch /tmp/dismiss_info_panel
    sync
    sleep "$delay"
    sync
}

# restore_ftp
# - restores original FTP state based on flags
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

# udhcpc_control
# - restarts udhcpc on wlan0
udhcpc_control() {
    if pgrep udhcpc >/dev/null; then
        killall -9 udhcpc
    fi
    sleep 1
    udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
}

# url_encode <string>
# - percent-encodes string for URLs
url_encode() {
    encoded_str=$(echo "$*" | awk '
    BEGIN {
	split ("1 2 3 4 5 6 7 8 9 A B C D E F", hextab, " ")
	hextab [0] = 0
	for ( i=1; i<=255; ++i ) ord [ sprintf ("%c", i) "" ] = i + 0
    }
    {
	encoded = ""
	for ( i=1; i<=length ($0); ++i ) {
	    c = substr ($0, i, 1)
	    if ( c ~ /[a-zA-Z0-9.-]/ ) {
		encoded = encoded c		# safe character
	    } else if ( c == " " ) {
		encoded = encoded "%20"	# special handling
	    } else {
		# unsafe character, encode it as a two-digit hex-number
		lo = ord [c] % 16
		hi = int (ord [c] / 16);
		encoded = encoded "%" hextab [hi] hextab [lo]
	    }
	}
	    print encoded
    }
')
    echo "$encoded_str"
}

# url_encode_path <path>
# - encodes each path segment but keeps '/'
url_encode_path() {
    local path="$1"
    local encoded=""
    local IFS='/'
    local part

    for part in $path; do
        if [ -n "$encoded" ]; then
            encoded="${encoded}/"
        fi
        encoded="${encoded}$(url_encode "$part")"
    done

    printf '%s\n' "$encoded"
}

# read_cookie [verbose]
# - parses /mnt/SDCARD/RetroArch/retroarch.cookie.client into core/rom/checksum vars
# - sets: core_url, rom_url, romdirname, romName, romNameNoExtension, Img_path
read_cookie() {
    local verbose="$1"
    sync
    while IFS= read -r line; do
        case $line in
        "[core]: "*)
            core="${line##"[core]: "}"
            ;;
        "[rom]: "*)
            rom="${line##"[rom]: "}"
            ;;
        "[coresize]: "*)
            corechecksum="${line##"[coresize]: "}"
            ;;
        "[corechksum]: "*)
            corechecksum="${line##"[corechksum]: "}"
            ;;
        "[romsize]: "*)
            romchecksum="${line##"[romsize]: "}"
            ;;
        "[romchksum]: "*)
            romchecksum="${line##"[romchksum]: "}"
            ;;
        "[cpuspeed]: "*)
            cpuspeed="${line##"[cpuspeed]: "}"
            ;;
        esac
        log "$core $rom $coresize $corechksum $romsize $romchksum"
    done <"/mnt/SDCARD/RetroArch/retroarch.cookie.client"

    # url encode or curl complains
    export core_url=$(url_encode "$core")
    export rom_url=$(url_encode "$rom")

    romdirname=$(echo "$rom" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
    romName=$(basename "$rom")
    romNameNoExtension=${romName%.*}
    Img_path="/mnt/SDCARD/Roms/$romdirname/Imgs/$romNameNoExtension.png"

    if [ "$verbose" = "1" ]; then
        log "Cookie file read :"
        log "romdirname $romdirname"
        log "romName $romName"
        log "romNameNoExtension $romNameNoExtension"
        log "Img_path $Img_path"
    else
        log "Cookie file read"
    fi
}

# start_ftp [check_stop_message]
# - starts built-in FTP server for signaling
# - if a message is provided, check_stop is invoked with it first
start_ftp() {
    if [ -n "$1" ]; then
        check_stop "$1"
    fi
    if is_running bftpd; then
        log "FTP already running, killing to rebind"
        bftpd_p=$(ps | grep bftpd | grep -v grep | awk '{for(i=4;i<=NF;++i) printf $i\" \"}')
        killall -9 bftpd
        killall -9 tcpsvd
        tcpsvd -E 0.0.0.0 21 ftpd -w / &
    else
        tcpsvd -E 0.0.0.0 21 ftpd -w / &
        log "Starting FTP server"
    fi
}
# check_wifi <use_udhcpc> <hard_fail> <down_wlan1>
# - use_udhcpc: 1 to call udhcpc_control after wpa_supplicant
# - hard_fail: 1 to notify_stop on failure, 0 to just pause
# - down_wlan1: 1 to bring wlan1 down before checks
check_wifi() {
    local use_udhcpc="$1"
    local hard_fail="$2"
    local down_wlan1="$3"

    if [ "$down_wlan1" -eq 1 ]; then
        ifconfig wlan1 down
    fi

    if ifconfig wlan0 &>/dev/null; then
        log "Wifi up"
    else
        build_infoPanel_and_log "WIFI" "Wifi disabled, starting..."

        /customer/app/axp_test wifion
        sleep 2
        ifconfig wlan0 up
        sleep 1
        $miyoodir/app/wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf

        if [ "$use_udhcpc" -eq 1 ]; then
            udhcpc_control
        fi

        if is_running wpa_supplicant && ifconfig wlan0 >/dev/null 2>&1; then
            build_infoPanel_and_log "WIFI" "Wifi started."
        else
            build_infoPanel_and_log "WIFI" "Unable to start WiFi\n unable to continue."
            if [ "$hard_fail" -eq 1 ]; then
                notify_stop
            else
                sleep 1
            fi
        fi

        sleep 2
    fi
}
