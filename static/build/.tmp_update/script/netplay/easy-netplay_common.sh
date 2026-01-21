# Shared netplay helpers

# logging (needed by helpers in this file)
. $sysdir/script/log.sh

# Tunables
# seconds unless stated otherwise; bytes unless stated otherwise
NETPLAY_FTP_CHECK_RETRIES=3 # ftp head retry count for preflight checks
NETPLAY_FTP_CHECK_DELAY=1 # delay between ftp head retries
NETPLAY_FTP_READY_DELAY=3 # wait before first ftp head to allow peer ftp start
NETPLAY_FTP_DOWNLOAD_RETRIES=5 # download retry count per file
NETPLAY_FTP_DOWNLOAD_DELAY=1 # delay between download retries
NETPLAY_FTP_HEAD_TIMEOUT=2 # curl connect timeout for ftp head
NETPLAY_UDHCPC_RESTART_DELAY=1 # delay before restarting udhcpc
NETPLAY_FTP_START_DELAY=0.5 # wait after starting ftp before checks
NETPLAY_WIFI_POWER_ON_DELAY=2 # delay after wifi power-on
NETPLAY_WIFI_UP_DELAY=1 # delay after bringing wlan0 up
NETPLAY_WIFI_POST_START_DELAY=2 # delay after wifi start sequence
NETPLAY_WIFI_SOFT_FAIL_DELAY=1 # delay after wifi soft-fail
NETPLAY_SYNC_MAX_FILE_CHKSUM_SIZE=26214400 # max file size to allow checksum
NETPLAY_SYNC_MAX_FILE_DL_SIZE=104857600 # max file size to allow download
NETPLAY_COOKIE_MAX_FILE_SIZE=26214400 # max file size for cookie checksum entries
NETPLAY_SYNC_FAIL_DELAY=2 # delay after sync failures before cleanup
NETPLAY_INFOPANEL_SLEEP=0.5 # infoPanel delay
NETPLAY_FTP_HEAD_READY=0 # flag to avoid repeated FTP warmup delays
COOKIE_CLIENT_PATH="/mnt/SDCARD/RetroArch/retroarch.cookie.client"
COOKIE_FILE="/mnt/SDCARD/RetroArch/retroarch.cookie"

# checksize_func <file_path> <remote_size>
# - sets: same_size (0 different, 1 identical, 2 unknown)
checksize_func() {
    local func_file_path="${1:-$file_path}"
    local filesize_tocheck="${2:-$remote_file_size}"

    if [ -e "$func_file_path" ]; then
        local_file_size=$(stat -c%s "$func_file_path")
        log "checksize_func: local_file_size=$local_file_size for $func_file_path"
        log "checksize_func: remote_file_size='$filesize_tocheck'"
        if echo "$filesize_tocheck" | grep -q "^[0-9][0-9]*$"; then
            if [ "$filesize_tocheck" -eq "$local_file_size" ]; then
                same_size=1
            else
                same_size=0
            fi
            log "checksize_func: same_size=$same_size (numeric compare)"
        else
            log "Non-numeric remote file size for checksize_func: '$filesize_tocheck' (skipping size check)"
            same_size=1
        fi
    else
        same_size=0
    fi
}

# checksum_func <file_path> <crc>
# - uses: NETPLAY_SYNC_MAX_FILE_CHKSUM_SIZE
# - sets: same_chksum
checksum_func() {
    local_file_size=$(stat -c%s "$file_path")
    local func_file_path="$1"
    local CRC="$2"

    ########################## File checksum check : same_chksum = 0 different, 1 identical , 2 unknown

    if [ "$CRC" != "0" ]; then # file_checksum=0 means skip the difference check = always replace
        local_file_checksum=$(xcrc "$func_file_path")

        if [ "$local_file_size" -gt "$NETPLAY_SYNC_MAX_FILE_CHKSUM_SIZE" ]; then
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
# - uses: NETPLAY_INFOPANEL_SLEEP
# - shows persistent infoPanel and logs message
build_infoPanel_and_log() {
    local title="$1"
    local message="$2"
    local delay="$NETPLAY_INFOPANEL_SLEEP"

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
    sleep "$NETPLAY_UDHCPC_RESTART_DELAY"
    udhcpc -i wlan0 -s /etc/init.d/udhcpc.script >/dev/null 2>&1 &
}

# url_encode <path>
# - encodes each path segment but keeps '/'
url_encode() {
    local path="$1"
    local encoded=""
    local IFS='/'
    local part

    for part in $path; do
        if [ -n "$encoded" ]; then
            encoded="${encoded}/"
        fi
        encoded="${encoded}$(echo "$part" | awk '
    BEGIN {
	split ("1 2 3 4 5 6 7 8 9 A B C D E F", hextab, " ")
	hextab [0] = 0
	for ( i=1; i<=255; ++i ) ord [ sprintf ("%c", i) "" ] = i + 0
    }
    {
	encoded = ""
	for ( i=1; i<=length ($0); ++i ) {
	    c = substr ($0, i, 1)
	    if ( c ~ /[a-zA-Z0-9._-]/ ) {
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
    }')"
    done

    printf '%s\n' "$encoded"
}

# strip_game_name <name>
# - strips region/version tags and returns cleaned name
strip_game_name() {
    echo "$1" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g'
}

# format_game_name <name> [label] [prefix]
# - label adds "<label>: \n" before the name
# - prefix allows leading newline/spacing (e.g. "\n ")
format_game_name() {
    local name="$1"
    local label="$2"
    local prefix="$3"
    local trimmed

    trimmed=$(strip_game_name "$name")
    if [ -n "$label" ]; then
        printf '%s%s: \n%s' "$prefix" "$label" "$trimmed"
    else
        printf '%s%s' "$prefix" "$trimmed"
    fi
}

# remove_files
# - removes files if they exist
remove_files() {
    local f
    for f in "$@"; do
        if [ -n "$f" ] && [ -e "$f" ]; then
            echo "Removing: $f"
            rm -f "$f"
        fi
    done
}

# netplay_cleanup <message> <hotspot_cleanup> <restore_ftp> <kill_infopanel> <disable_hotspot_flag> [files...]
# - runs common cleanup steps and exits
netplay_cleanup() {
    local message="$1"
    local do_hotspot_cleanup="$2"
    local do_restore_ftp="$3"
    local do_kill_infopanel="$4"
    local do_disable_hotspot="$5"
    shift 5

    build_infoPanel_and_log "Cleanup" "$message"

    pkill -9 pressMenu2Kill

    if [ "$do_kill_infopanel" -eq 1 ] && is_running infoPanel; then
        killall -9 infoPanel
    fi

    if [ "$do_hotspot_cleanup" -eq 1 ]; then
        . "$sysdir/script/network/hotspot_cleanup.sh"
    fi

    if [ "$do_restore_ftp" -eq 1 ]; then
        restore_ftp
    fi

    remove_files "$@"

    if [ "$do_disable_hotspot" -eq 1 ]; then
        disable_flag hotspotState
    fi

    # reset power save on the wifi
    iw wlan0 set power_save on

    sync
    log "Cleanup done"
    exit
}

# wifi_disabled
# - returns 0 if wifi is disabled
wifi_disabled() {
    [ $(/customer/app/jsonval wifi) -eq 0 ]
}

# read_cookie [verbose]
# - parses "$COOKIE_CLIENT_PATH" into core/rom/checksum vars
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
    done <"$COOKIE_CLIENT_PATH"

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

# sync_file <type> <path> <check_size> <checksum> <sync_type> <mandatory>
# - uses: peer_ip or hostip, ensure_ftp_head, checksize_func, checksum_func
# - sync_type: -o overwrite if different, -f force, -b backup, -c check only
# - mandatory: -m to cleanup on failure
sync_file() {

	file_type="$1"            # Used in displayed message and some custom actions
	file_path="$2"            # Local file path
	file_check_size="$3"      # 0 or 1 to indicate if we have to check the file size
	remote_file_checksum="$4" # 0 to skip , real checksum value to check
	sync_type="$5"            # -o overwrite if different, -f forced, -b backup, -c check only
	file_mandatory="$6"       # -m , exit the script on failed sync_success

	# some useful vars
	dir_path=$(dirname "$file_path")
	remote_ip="${peer_ip:-$hostip}"
	file_url="ftp://${remote_ip}/$(url_encode "${file_path#*/}")"

	log "\n############################ SYNC_FILE DEBUGGING ############################"
	log file_type $file_type
	log file_path $file_path
	log file_check_size $file_check_size
	log remote_file_checksum $remote_file_checksum
	log sync_type $sync_type
	log file_mandatory $file_mandatory
	log file_url $file_url
	log dir_path $dir_path
	log "#############################################################################"

	# state vars
	same_size=
	same_chksum=
	sync_success=
	run_sync= # tell if the sync task must be done or not

	if ! ensure_ftp_head "$file_url" "$remote_ip" "$file_mandatory"; then
		same_size=0
		run_sync=0
		sync_success=0
	fi
	RequestResult="$ftp_head_result"

	if [[ $RequestResult == *"The file does not exist"* ]]; then
		log "The remote file does not exist."
		msg="The remote file does not exist."
		build_infoPanel_and_log "Syncing" "The remote file does not exist."
		run_sync=0
		sync_success=0
		same_size=0
	else
		remote_file_size=$(echo "$RequestResult" | grep -i "Content-Length" | awk '{print $2}')
		if ! echo "$remote_file_size" | grep -q "^[0-9][0-9]*$"; then # check if the remote file size is a numeric value
			log "Non-numeric remote file size: '$remote_file_size'"
			log "HEAD response: $RequestResult"
			same_size=0
		else
			log "remote_file_size: $remote_file_size"
		fi

	fi

	if [ -e "$file_path" ]; then

		########################## File checksum check : same_chksum = 0 different, 1 identical , 2 unknown
		checksum_func "$file_path" "$remote_file_checksum"

		########################## File size check   same_size = 0 different, 1 identical , 2 unknown
		if [ "$file_check_size" -eq 1 ]; then # file_checksum=0 means skip the difference check = always replace
			checksize_func "$file_path" "$remote_file_size"
		else
			log "Skipping file size check."
			same_size=1 # fake same size for skipping
		fi

	else
		log "The local file does not exist."
		same_size=0
		same_chksum=0
	fi

	########################## exception : max file size check on the remote
	if echo "$remote_file_size" | grep -q "^[0-9][0-9]*$"; then
		if [ "$remote_file_size" -le "$NETPLAY_SYNC_MAX_FILE_DL_SIZE" ]; then
			log "Remote file size ok: $remote_file_size bytes  (<= $NETPLAY_SYNC_MAX_FILE_DL_SIZE bytes)"
		else
			log "Remote file size too big: $remote_file_size bytes (> $NETPLAY_SYNC_MAX_FILE_DL_SIZE bytes)"
			run_sync=0
		fi
	else
		log "Skipping max file size check due to non-numeric remote size: '$remote_file_size'"
	fi

	##########################  We have all the required information, depending the chosen option we run the copy or not

	if [ "$sync_type" == "-o" ]; then # we overwrite the file if different
		log "option -o selected : we overwrite the file if different."
		if [ "$same_size" -ne 1 ] || [ "$same_chksum" -ne 1 ]; then
			[ -z "$run_sync" ] && run_sync=1
		fi
	fi

	if [ "$sync_type" == "-b" ]; then # backup
		log "option -b selected : we backup before overwrite."
		if { [ "$remote_file_checksum" != "0" ] && [ "$same_chksum" -ne 1 ]; } || [ "$remote_file_checksum" == "0" ]; then
			[ -z "$run_sync" ] && run_sync=1
		fi
	fi

	if [ "$sync_type" == "-f" ]; then # we overwrite the file if different
		log "option -f selected : forced file syncing."
		if { [ "$remote_file_checksum" != "0" ] && [ "$same_chksum" -ne 1 ]; } || [ "$remote_file_checksum" == "0" ]; then
			[ -z "$run_sync" ] && run_sync=1
		fi

	fi

	if [ "$sync_type" == "-c" ]; then # we overwrite the file if different
		log "option -c selected : no file copy, only check."
		run_sync=0

		if [ "$same_size" -ne 1 ]; then
			msg="Files doesn't have the same size."
			sync_success=0
			[ "$file_mandatory" = "-m" ] && cleanup

		fi
		if [ "$same_chksum" -ne 1 ]; then
			msg="$msg\nFiles doesn't have the same checksum."
			sync_success=0
			[ "$file_mandatory" = "-m" ] && cleanup

		fi
		if [ "$same_size" -eq 1 ] && [ "$same_chksum" -eq 1 ]; then
			msg="Remote and local files are identical."
			sync_success=1
		fi
		build_infoPanel_and_log "File check" "$msg"

	fi

	########################## COPY Operation ##########################

	if [ "$run_sync" -eq 1 ]; then

		if [ $file_type == "Cookie" ]; then # exception for cookies : we don't download with the same target name
			file_path="${file_path}.client"
		fi
		# let's make a backup first whatever the case
		if [ -e "$file_path" ]; then
			if [ $file_type == "Rom" ]; then
				Netplay_Rom_Folder="$(dirname "$file_path")/.netplay"
				mkdir -p "$Netplay_Rom_Folder"
				file_path="$Netplay_Rom_Folder/$(basename "$file_path")"
				rom=$file_path

			elif [ $file_type == "Core" ]; then
				Netplay_Core_Folder="$(dirname "$file_path")/.netplay"
				mkdir -p "$Netplay_Core_Folder"
				file_path="$Netplay_Core_Folder/$(basename "$file_path")"
				core=$file_path
			else
				mv "$file_path" "${file_path}_old"
				log "Existing $file_type file moved to ${file_path}_old"
			fi
		fi

		if [ ! -d "$dir_path" ]; then
			mkdir -p "$dir_path"
		fi

		log "Starting to download $file_type from $file_url"
		download_attempt=1
		while [ $download_attempt -le $NETPLAY_FTP_DOWNLOAD_RETRIES ]; do
			curl_output=$(curl -S -o "$file_path" "$file_url" 2>&1)
			curl_exit=$?

			if [ $curl_exit -eq 0 ]; then
				log "$file_type download completed"
				break
			fi

			log "$file_type download failed (curl exit=$curl_exit, attempt $download_attempt/$NETPLAY_FTP_DOWNLOAD_RETRIES)"
			log "curl error: $curl_output"
			if [ $curl_exit -eq 9 ] || [ $curl_exit -eq 78 ]; then
				log "FTP path denied for $file_url"
				break
			fi

			download_attempt=$((download_attempt + 1))
			sleep "$NETPLAY_FTP_DOWNLOAD_DELAY"
		done

	fi

	###################### FINAL CHECK RESULT #########################

	if [ ! -e "$file_path" ]; then
		if [ "$file_type" = "Img" ]; then
			log "Image not found on peer; continuing without image."
		else
			build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."
		fi

		# copy has failed : restoring the original file
		if [ $file_type != "Rom" ] && [ $file_type != "Core" ]; then
			mv "${file_path}_old" "$file_path"
			log "backup restored"
		fi
		sleep "$NETPLAY_SYNC_FAIL_DELAY"
		if [ "$file_mandatory" = "-m" ]; then
			if [ "$file_type" != "Img" ]; then
				if type notify_peer >/dev/null 2>&1; then
					notify_peer "$remote_ip" "stop_now"
				fi
				cleanup
			fi
		fi

	else

		checksum_func "$file_path" "$remote_file_checksum"

		if [ "$file_check_size" -eq 1 ]; then # file_checksum=0 means skip the difference check = always replace
			checksize_func "$file_path" "$remote_file_size"
		else
			log "Skipping file size check."
			same_size=1 # fake same size for skipping
		fi

		if [ "$same_size" -eq 1 ] && [ "$same_chksum" -eq 1 ]; then
			build_infoPanel_and_log "Syncing" "$file_type synced."
			if [ $file_type == "Rom" ]; then
				log "Refreshing roms list ${rom%/*}/${romdirname}_cache6.db"
				rm "${rom%/*}/${romdirname}_cache6.db"
			fi

		else
			build_infoPanel_and_log "Sync Failed" "Failed to download the $file_type file."

			# copy has failed : restoring the original file
			if [ $file_type != "Rom" ] && [ $file_type != "Core" ]; then
				mv "${file_path}_old" "$file_path"
				log "backup restored"
			fi
			sleep "$NETPLAY_SYNC_FAIL_DELAY"
			if [ "$file_mandatory" = "-m" ]; then
				if type notify_peer >/dev/null 2>&1; then
					notify_peer "$remote_ip" "stop_now"
				fi
				cleanup
			fi
		fi
		#####"

	fi

	################### END ##########################
}

# create_cookie_info
# - writes core/rom/checksum/cpuspeed into retroarch.cookie
create_cookie_info() {
    echo "[core]: $netplaycore" >"$COOKIE_FILE"
    echo "[rom]: $cookie_rom_path" >>"$COOKIE_FILE"

    if [ -s "$netplaycore" ]; then
        log "Writing core size"
        core_size=$(stat -c%s "$netplaycore")
        if [ "$core_size" -gt "$NETPLAY_COOKIE_MAX_FILE_SIZE" ]; then
            echo "[corechksum]: 0" >>"$COOKIE_FILE"
        else
            echo "[corechksum]: $(xcrc "$netplaycore")" >>"$COOKIE_FILE"
        fi
    fi

    if [ -s "$cookie_rom_path" ]; then
        rom_size=$(stat -c%s "$cookie_rom_path")
        log "Cookie local rom size : $rom_size"
        if [ "$rom_size" -gt "$NETPLAY_COOKIE_MAX_FILE_SIZE" ]; then
            echo "[romchksum]: 0" >>"$COOKIE_FILE"
        else
            echo "[romchksum]: $(xcrc "$cookie_rom_path")" >>"$COOKIE_FILE"
        fi
    fi

    if [ -s "$cpuspeed" ]; then
        echo "[cpuspeed]: $cpuspeed" >>"$COOKIE_FILE"
        log "Writing custom cpuspeed: $cpuspeed"
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
    check_ftp_local
    sleep "$NETPLAY_FTP_START_DELAY"
}

# check_ftp_local [port]
# - logs whether tcpsvd is running and its args/port
check_ftp_local() {
    local port="${1:-21}"
    if pgrep tcpsvd >/dev/null; then
        local tcpsvd_ps
        tcpsvd_ps=$(ps | grep tcpsvd | grep -v grep)
        log "FTP tcpsvd running (expected port $port): $tcpsvd_ps"
    else
        log "FTP tcpsvd not running (expected port $port)"
        build_infoPanel_and_log "FTP Error" "FTP server failed to start."
    fi
}

# ensure_ftp_head <url> <remote_ip> <mandatory>
# - give peer time to start FTP before first transfer, then do a quick HEAD
# - sets: ftp_head_result, ftp_head_exit
ensure_ftp_head() {
    # wait for peer tcpsvd only on the first HEAD; subsequent calls skip delay once ready
    if [ "${NETPLAY_FTP_HEAD_READY:-0}" -eq 0 ]; then
        sleep "$NETPLAY_FTP_READY_DELAY"
    fi
    ftp_head_result=$(curl -sS -I --connect-timeout "$NETPLAY_FTP_HEAD_TIMEOUT" "$1" 2>&1)
    ftp_head_exit=$?
    if [ $ftp_head_exit -eq 0 ]; then
        NETPLAY_FTP_HEAD_READY=1
    fi
    return 0
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
        sleep "$NETPLAY_WIFI_POWER_ON_DELAY"
        ifconfig wlan0 up
        sleep "$NETPLAY_WIFI_UP_DELAY"
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
                sleep "$NETPLAY_WIFI_SOFT_FAIL_DELAY"
            fi
        fi

        sleep "$NETPLAY_WIFI_POST_START_DELAY"
    fi
}

# capture quick state of a file for debugging save/rom transfer issues.
log_file_state() {
    local label="$1"
    local path="$2"
    if [ -f "$path" ]; then
        local size
        size=$(stat -c%s "$path" 2>/dev/null || wc -c <"$path")
        local md5
        md5=$(md5sum "$path" 2>/dev/null | awk '{print $1}')
        log "$label: exists size=${size} md5=${md5:-N/A} path=$path"
    else
        log "$label: missing path=$path"
    fi
}
