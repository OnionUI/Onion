#!/bin/sh

# This script will run a suite of network checks from the device, the captured data is mostly response codes from services and ping replies
# It will however pull dmesg, which may contain the users WiFi SSID. Be cautious as to where you ask the user to upload this.

menulabel="Network log snapshot"
tooltip="Check common websites/services for access \n and export the logs to SD:log_export.7z"

##################
## SETUP ##
##################
SDDIR="/mnt/SDCARD"
SYSDIR="${SDDIR}/.tmp_update"
LOGDIR="${SYSDIR}/logs"
DIAGSDIR="${SYSDIR}/script/diagnostics"
THEME_TEST_ASSET="https://raw.githubusercontent.com/OnionUI/Themes/main/release/M%20by%20tenlevels.zip"
PORTS_TEST_ASSET="https://github.com/OnionUI/Ports-Collection/releases/latest/download/Sonic.Mania.7z"
PICO_BBS_ASSET="https://www.lexaloffle.com/bbs/cpost_lister3.php?cat=7"
RA_CFG="/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"
RA_URL="https://retroachievements.org"
LOG_EX_FILE="network_check"

##################
## LOGGING FUNCTION ##
##################

logfile=util_netcheck
. "${SYSDIR}/script/log.sh"
program=$(basename "$0" .sh)

log_message() {
    local message="$(date +"%Y-%m-%d %H:%M:%S") - $1"
    echo -e "$message" >> "${LOGDIR}/$LOG_EX_FILE.log"
    echo -e "$message" # for console
}

section_header() {
    local header=$1
    log_message "======== $header ========"
}

section_header_big() {
    local header_big=$1
    log_message "================================ $header_big ================================"
}

##################
## CHECKING      ##
##################

ping_success=0
ping_total=0
website_success=0
website_total=0
download_success=0
download_total=0

##################
## Util functions ##
##################

check_usb_devices() {
    local device_count=$(lsusb | wc -l)
    device_count=$((device_count))

    if [ "$device_count" -ge 3 ]; then
        log_message "Check successful: Found $device_count USB devices connected."
    else
        log_message "Check failed: Only $device_count USB devices found. Third USB device missing, possible WiFi chip failure - does WiFi work?"
    fi
}

perform_ping() {
    local host=$1
    local service=$2
    log_message "Starting ping test for $service: $host"
    ping_total=$((ping_total + 1))
    if ping -c 1 -W 2 $host > /dev/null 2>&1; then
        log_message "Ping to $host ($service) successful. \n"
        ping_success=$((ping_success + 1))
        return 0
    else
        log_message "Ping to $host ($service) failed. \n"
        return 1
    fi
}

check_website() {
    local url=$1
    website_total=$((website_total + 1))
    log_message "Checking website availability: $url"
    if wget -q --no-check-certificate --spider $url; then
        log_message "Website $url is up.\n"
        website_success=$((website_success + 1))
        return 0
    else
        log_message "Website $url is down.\n"
        return 1
    fi
}

test_download() {
    local url=$1
    download_total=$((download_total + 1))
    log_message "Testing download from: $url"
    # check the response trying to download assets from $url
    # 200 is good, anything else not so good.
    # doesn't actually download the asset, just attempts
    local status=$(curl -o /dev/null -s -w "%{http_code}" -L --head --insecure "$url")
    if [ "$status" -eq 200 ]; then
        log_message "Download test for $url was successful. Status code: $status \n"
        download_success=$((download_success + 1))
        return 0
    else
        log_message "Download test for $url failed with status code: $status \n"
        return 1
    fi
}

test_dns_resolution() {
    local dns_test_host="google.co.uk"
    if nslookup "$dns_test_host" > /dev/null 2>&1; then
        log_message "DNS resolution test for $dns_test_host successful. \n"
        return 0
    else
        log_message "DNS resolution test for $dns_test_host failed. \n"
        return 1
    fi
}

check_firmware_for_dns_issue() {
    local version
    version=$(/etc/fw_printenv miyoo_version)
    if [ "$version" = "202303021817" ]; then
        log_message "DNS Failure will occur due to firmware version, update your firmware. \n"
        return 0
    else
        log_message "Firmware supports DNS, testing...\n"
        test_dns_resolution
        return 1
    fi
}

pull_common_logs() {
    ifconfig -a >> "${LOGDIR}/network_info.log"
    dmesg | grep -i eth0 >> "${LOGDIR}/network_info.log"
    dmesg >> "${LOGDIR}/network_dmesg.log"
}

check_retroachievements_api() {
    # we're not actually checking if the API lets us login as we don't know the users API key but
    # we'll request some info from the API and see if it replies with a code we expect
    # it should respond as unauthorised/unauthed (401 or 419...) 
    # we can't use a POST on the login as they use CSRF
    local username="dummy"
    local api_key="dummy"
    local api_url="https://retroachievements.org/API/API_GetAchievementOfTheWeek.php?z=$username&y=$api_key"
    
    log_message "Checking RetroAchievements API availability"

    local status=$(curl -s -o /dev/null -w "%{http_code}" --insecure "$api_url")

    case "$status" in
        200)    # won't ever happen, we're not passing creds - leaving until i figure out a way to do this
            log_message "API is accessible and credentials are valid. Status code: $status \n"
            return 0
            ;;
        401)    # the one we should get, indicates the API is up and responding to a failed attempt
            log_message "API check was successful, returned 401 unauthorized. Status code: $status \n"
            return 0
            ;;
        419)    # i doubt we'll get this but catch it anyway (appears on a request to: http://retroachievements.org/API/dorequest.php?r=login)
            log_message "API check was successful, returned 419 unauthed. Status code: $status \n"
            return 0
            ;;
        503)    # bad
            log_message "API service is unavailable (503). Check back later. \n"
            return 1
            ;;
        500)    # bad
            log_message "Internal server error (500). \n"
            return 1
            ;;
        404)    # bad
            log_message "API endpoint not found (404). \n"
            return 1
            ;;
        408)    # bad
            log_message "Request timed out (408). \n"
            return 1
            ;;
        0)      # bad
            log_message "Failed to connect to API. \n"
            return 1
            ;;
        *)      # bad
            log_message "API check failed with status code: $status \n"
            return 1
            ;;
    esac
}

check_driver_inserted() {
    local module=$(lsmod | grep '8188fu')
    if echo "$module" | grep -q 'Live'; then
        log_message "8188fu driver is loaded and active."
        return 0
    else
        log_message "8188fu driver is missing or not active. Possible WiFi chip failure - does WiFi work?"
        return 1
    fi
}

check_valid_adaptor() {
    local active_interfaces=$(ifconfig | awk '/^[a-zA-Z0-9]/ {print $1}' | grep -v 'lo' | tr -d ':')

    if [ -z "$active_interfaces" ]; then
        log_message "No active network interfaces found."
        return 1
    else
        log_message "Active network interfaces found: $active_interfaces"
        return 0
    fi
}

check_network_complete() {
    local interface=$(ifconfig | awk '/^[a-zA-Z0-9]/ {print $1}' | grep -v 'lo' | tr -d ':' | head -n 1)

    if [ -z "$interface" ]; then
        log_message "No network interface found to check."
        return 1
    fi

    local ip_address=$(ifconfig $interface | grep 'inet addr:' | awk '{print $2}' | cut -d':' -f2)
    if [ -z "$ip_address" ]; then
        log_message "$interface does not have an IP address assigned."
        return 1
    fi

    local subnet_mask=$(ifconfig $interface | grep 'Mask:' | awk '{print $4}' | cut -d':' -f2)
    if [ -z "$subnet_mask" ]; then
        log_message "No subnet mask set for $interface."
        return 1
    fi

    local gateway=$(route -n | awk '/^0.0.0.0/ {print $2}' | head -n 1)
    if [ -z "$gateway" ]; then
        log_message "No default gateway set for $interface."
        return 1
    fi

    local dns_resolvers=$(grep 'nameserver' /etc/resolv.conf | awk '{print $2}')
    if [ -z "$dns_resolvers" ]; then
        log_message "No DNS resolvers configured."
        return 1
    else
        log_message "DNS Resolvers found: $dns_resolvers"
        echo $dns_resolvers | tr ' ' '\n' | while read dns_resolver; do
            if ping -c 1 -W 2 $dns_resolver > /dev/null 2>&1; then
                log_message "Ping to DNS resolver $dns_resolver successful."
            else
                log_message "Ping to DNS resolver $dns_resolver failed."
                return 1
            fi
        done
    fi

    log_message "Network check complete for $interface: IP $ip_address, Subnet Mask $subnet_mask, Gateway $gateway, DNS Resolvers: $dns_resolvers \n"
    return 0
}


display_summary() {
    log_message "Ping Test Results: $ping_success successful out of $ping_total attempts."
    log_message "Website Availability: $website_success successful out of $website_total checks."
    log_message "Download Test Results: $download_success successful out of $download_total attempts."
    
    # Determine the overall network health based on the ratio of success to total checks
    if [ "$ping_success" -eq "$ping_total" ]; then
        log_message "Ping replies from all DNS providers - OK"
    elif [ "$ping_success" -gt 0 ]; then
        log_message "Ping replies from DNS providers - Degraded"
    else
        log_message "Ping responses from DNS Providers - Failed"
    fi
    
    if [ "$website_success" -eq "$website_total" ]; then
        log_message "Website checks - All sites are reachable"
    elif [ "$website_success" -gt 0 ]; then
        log_message "Website checks - Some sites are unreachable"
    else
        log_message "Website checks - All sites are unreachable"
    fi
    
    if [ "$download_success" -eq "$download_total" ]; then
        log_message "Download tests - All downloads successful"
    elif [ "$download_success" -gt 0 ]; then
        log_message "Download tests - Some downloads failed"
    else
        log_message "Download tests - All downloads failed"
    fi
}

if [ "$#" -gt 0 ]; then
    if check_valid_adaptor; then
        log_message "Proceeding, valid adaptor found" 
    else
        log_message "Exiting, no valid adaptor found" 
    fi
        
    case "$1" in
        perform_ping)
            perform_ping "$2" "$3"
            ;;
        check_website)
            check_website "$2"
            ;;
        test_download)
            test_download "$2"
            ;;
        check_firmware_for_dns_issue)
            check_firmware_for_dns_issue
            ;;
        check_network_complete)
            check_network_complete
            ;;
        check_retroachievements_api)
            check_retroachievements_api
            ;;
        *)
            echo "Unsupported function or incorrect usage."
            echo "Usage: $0 <function> [<args>]"
            exit 1
            ;;
    esac
    exit
fi

main() {
    
    # we've entered through tweaks, remove old files
    if [ -f "$LOGDIR/$LOG_EX_FILE.log" ]; then
        rm "$LOGDIR/$LOG_EX_FILE.log" "$LOGDIR/network_info.log" "$LOGDIR/network_dmesg.log"
    else
        echo "$LOGDIR/$LOG_EX_FILE.log Doesn't exist"
    fi

    # initial setup, check wifi, check chip, check driver is inserted
    # check we're connected to a network
    section_header_big "Hardware and config checks"
    
    check_usb_devices
    check_driver_inserted

    # check wifi, obviously
    wifi_setting=$(/customer/app/jsonval wifi)
    if [ "$wifi_setting" -eq 0 ]; then
        log_message "WiFi is disabled, exiting script to avoid running network-dependent checks."
        exit 0 # drop out, no logfile will be created
    else
        log_message "WiFi is enabled, proceeding with network checks."
    fi
    
    # check our adaptor is valid and if we're on a completely negotiated network
    if check_valid_adaptor; then
        check_network_complete # we're connected but do we have the requirements
    fi
    
    # generic pings, lets see who we can hit
    section_header_big "Network Ping Tests"
    perform_ping "8.8.8.8" "Google DNS"
    perform_ping "1.1.1.1" "Cloudflare DNS"
    perform_ping "208.67.222.222" "OpenDNS"

    # website spiders, pings don't give us everything
    section_header_big "Service and Game Site Checks"
    check_website "https://github.com"
    check_website "https://www.baidu.com" # chinese
    check_website "https://www.naver.com"
    check_website "https://www.uol.com.br" # brazil
    check_website "https://www.timesofindia.indiatimes.com"
    check_website "http://lobby.libretro.com/"
    check_website "https://www.lexaloffle.com/bbs/cpost_lister3.php?cat=7"
    
    # download tests from some core services
    section_header_big "Download Tests"
    test_download $THEME_TEST_ASSET
    test_download $PORTS_TEST_ASSET
    test_download $PICO_BBS_ASSET
    
    # check cheevosapi status
    section_header_big "RetroAchievements API Check"
    check_retroachievements_api
    
    # check if the user is running old firmware that does not support DNS
    section_header_big "Firmware DNS Check"
    check_firmware_for_dns_issue

    # pull some interface config and dmesg
    section_header_big "System Network Configuration"
    log_message "Collecting network configuration information"
    pull_common_logs
    
    # summary gen
    section_header_big "Summary of Network Checks"
    display_summary

    sync

    if [ -x "${DIAGSDIR}/util_exporter.sh" ]; then
        log_message "Executing exporter utility."
        "${DIAGSDIR}/util_exporter.sh"
    else
        log_message "Exporter utility not found or not executable."
    fi

    log_message "Network checks complete."
}

main
