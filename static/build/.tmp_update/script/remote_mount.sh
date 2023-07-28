#!/bin/sh
# Remote_mount_helper
sysdir=/mnt/SDCARD/.tmp_update
SHELLECT=$sysdir/script/shellect.sh
APPDIR=/mnt/SDCARD/App/RemoteMount
CONFIG_FILE="$sysdir/config/saved_mounts.cfg"
FUSEKO="$sysdir/modules/fuse.ko"
KEY_SOURCE="$sysdir/config/OnionKey"
KEY_DEST="/var/OnionKey"

# Colours from OTA
RED='\033[1;31m' # ERROR
GREEN='\033[1;32m' # Success 
YELLOW='\033[1;33m'
BLUE='\033[1;34m' # Info
NC='\033[0m' # No Color

TEMP=$(mktemp)
SERVERADDR=""
USERNAME=""
LOCALDIR=""
REMOTEDIR=""
breakout="0"


# Entry point
main() {
    if [ -e "$APPDIR/.firstrun" ]; then
        clear
        create_banner "INFO" 
        echo -e "\n${YELLOW} Before going any further.. \n\n Have you setup the host/client keypair?\n" 
        echo -e "${YELLOW} This relies on SSH keys being on the client\n and the host \n ${NC}"
        echo -e "${YELLOW} SSH Connectivity is REQUIRED \n ${NC}" 
        create_banner "INFO" 
        echo -e "${NC}\nPress start\n"
        rm "$APPDIR/.firstrun"
        read
        clear
        main_menu
    else
        enable_wifi
        clear
        main_menu
    fi
}

# Build main menu with Shellect
main_menu() {
    local choice=$(echo -e "Add a new mount\nConnect to stored mount\nUnmount a folder\nDelete and unmount\nShow mount status\nShow brief mount status\nClear mount file\nGenerate SSH Key\nHelp\nExit" | $SHELLECT -t "Select option:" -b "Press A to select")
    
    case "$choice" in
    "Add a new mount")
        clear
        add_new_mount
        ;;
    "Connect to stored mount")
        clear
        mount_stored
        ;;
    "Unmount a folder")
        clear
        unmount_folder
        ;;
    "Delete and unmount")
        clear
        delete_unmount
        ;;
    "Show mount status")
        clear
        show_status
        ;;
    "Show brief mount status")
        clear
        show_brief_status
        ;;
    "Clear mount file")
        clear
        clear_history
        ;;
    "Generate SSH Key")
        clear
        key_gen
        ;;
    "Help")
        clear
        info_screen
        ;;
    "Exit")
        exit
        ;;
    *)
        echo "Invalid option"
        ;;
    esac
}

# Make sure wifi is up.
enable_wifi() {
	IP=$(ip route get 1 | awk '{print $NF;exit}')
	if [ "$IP" = "" ]; then
		echo "Wifi is disabled - trying to enable it..."
		insmod /mnt/SDCARD/8188fu.ko > /dev/null 2>&1
		ifconfig lo up > /dev/null 2>&1
		/customer/app/axp_test wifion > /dev/null 2>&1
		sleep 2
		ifconfig wlan0 up > /dev/null 2>&1
		wpa_supplicant -B -D nl80211 -iwlan0 -c /appconfigs/wpa_supplicant.conf > /dev/null 2>&1
		udhcpc -i wlan0 -s /etc/init.d/udhcpc.script > /dev/null 2>&1
		sleep 3
		clear
	fi
}

# Menu item to add a new mount
add_new_mount() {
    clear
    create_banner "Add new mount"
    echo -e "\n${NC}How to add a new server: \n\nFirst, input the server address below \n\nThis could be an IP address (e.g 192.168.1.84) \n or a hostname (e.g xk-shares.com)\n" 
    echo -e "${YELLOW}Type \"q\" at any point in this process to exit to main menu\n"
    echo -e "${YELLOW}Press Y to bring up the keyboard\n"
    prompt_user "${BLUE}What's the server address?${NC}" SERVERADDR
    if [ "$breakout" -eq 1 ]; then main_menu; return; fi

    if ! ping -c 1 -W 1 "$SERVERADDR" > /dev/null; then
        echo -e "${RED}Error, remote device not responding${NC}"
        sleep 2
        main_menu
    fi
    
    prompt_user "${BLUE}What's the username to connect with?${NC}" USERNAME
    if [ "$breakout" -eq 1 ]; then main_menu; return; fi
    
    clear
    
    echo -e "${NC}Now, what's the local directory? \n \nExamples would include: \n /mnt/SDCARD/Roms/GB or \n /mnt/SDCARD/Roms/GBA \n"
    
    prompt_user "${BLUE}Enter the directory to mount in${NC}" LOCALDIR
    if [ "$breakout" -eq 1 ]; then main_menu; return; fi
    
    clear
    
    echo -e "${NC}Now, what's the remote directory? \n \nExamples would include: \n C:/Users/XK/Roms/GB or \n /home/XK/Roms/GBA \n"
    
    prompt_user "${BLUE}Enter the directory to mount${NC}" REMOTEDIR
    if [ "$breakout" -eq 1 ]; then main_menu; return; fi
    
    clear
    
    echo -e "${GREEN}Saving options to file..."
    
    if [ ! -f $CONFIG_FILE ] || [ ! -s $CONFIG_FILE ]; then
        echo '[]' > $CONFIG_FILE
    fi

    mount_json=$(jq -n \
                    --arg sa "$SERVERADDR" \
                    --arg un "$USERNAME" \
                    --arg ld "$LOCALDIR" \
                    --arg rd "$REMOTEDIR" \
                    '{server_address: $sa, username: $un, local_dir: $ld, remote_dir: $rd}')

    jq ". += [$mount_json]" $CONFIG_FILE > tmp.$$.json && mv tmp.$$.json $CONFIG_FILE

    echo -e "${GREEN}Trying to mount, standby..."
    generate_sshfs_cmdline $USERNAME $SERVERADDR $REMOTEDIR $LOCALDIR
}

# Menu item to mount a stored ID
mount_stored() {
    clear
    count=1
    list_mounts

    read -p "Please select an option (or press A to cancel): " REPLY

    if [ -z "$REPLY" ]; then
        echo -e "No selection made\n"
        main_menu
    fi

    selected_json=$(jq -r ".[$((REPLY-1))]" "$CONFIG_FILE")

    username=$(echo -e "$selected_json" | jq -r '.username')
    server_address=$(echo -e "$selected_json" | jq -r '.server_address')
    remote_dir=$(echo -e "$selected_json" | jq -r '.remote_dir')
    local_dir=$(echo -e "$selected_json" | jq -r '.local_dir')

    generate_sshfs_cmdline "$username" "$server_address" "$remote_dir" "$local_dir"
}

# Menu item to unmount a folder that we already have mounted.
unmount_folder() {
    clear
    jq -c '.[]' "$CONFIG_FILE" > "$TEMP"
    count=1
    list_mounts
    read_and_store

    if mount | grep "on ${local_dir} type" > /dev/null; then
        echo -e "\n"
        read -p "${local_dir} is mounted. Unmount? (y/n): " unmount_reply
        if [[ "$unmount_reply" == "y" ]]; then
            fusermount -u "$local_dir"
            echo -e "\n${GREEN}${local_dir} has been unmounted.${NC}"
            sleep 3
        fi
    fi

    rm -f "$TEMP"
    main_menu
}

# Menu item to delete a mounted item and offer to unmount
delete_unmount() {
    clear
    jq -c '.[]' "$CONFIG_FILE" > "$TEMP"
    count=1
    list_mounts

    read_and_store

    if mount | grep "on ${local_dir} type" > /dev/null; then
        read -p "${local_dir} is mounted. Do you want to unmount it? (y/n): " unmount_reply
        if [[ "$unmount_reply" == "y" ]]; then
            fusermount -u "$local_dir"
            echo "${local_dir} has been unmounted."
        fi
    fi

    sed "${REPLY}d" "$TEMP" > tmp.$$.json && mv tmp.$$.json "$TEMP"

    output=$(jq -c -R -s -n '[inputs | fromjson?]' < "$TEMP")

    if [ $? -eq 0 ]; then
        echo "$output" > "$CONFIG_FILE"
        echo -e "${GREEN}Item deleted successfully${NC}"
        sleep 2
        rm -f "$TEMP"
        main_menu
    else
        echo -e "${RED}Failed to delete item${NC}"
        sleep 2
        rm -f "$TEMP"
        main_menu
    fi
}

# Grab the json from the config file
show_status() {
    clear
    count=1
    list_mounts
    echo -e "${BLUE}\nPress start to go back to the main menu...${NC}\n"
    read 
    main_menu
}


# Grab the json from the config file but condense it to fit more on screen
show_brief_status() {
    clear
    count=1
    list_mounts_brief
    echo -e "${BLUE}\nPress start to go back to the main menu...${NC}\n"
    read 
    main_menu
}

# Clear the confg file, will wipe everything but will not offer to unmount.
clear_history() {
    clear
    echo '[]' > $CONFIG_FILE
    echo -e "${GREEN}History cleared, saved file reset."
    sleep 2 
    main_menu
}

# Menu item to generate a keyfile tp $KEY_SOURCE
key_gen() {
    sync
    if [ -f $KEY_SOURCE ]; then
        echo -e "${RED}Key already exists at: \n ${YELLOW}/mnt/SDCARD/.tmp_update/config/OnionKey.pub \nSkipping key generation.\n"
    else
        echo -e "\n ${GREEN}Your key has been generated!: \n/mnt/SDCARD/.tmp_update/config/OnionKey.pub\n"
        ssh-keygen -t ed25519 -N "" -f $KEY_SOURCE
    fi

    create_banner "Key start"
    cat $KEY_SOURCE.pub
    create_banner "Key end"
    
    echo -e "\n${NC}You now need to copy this to your host machine.\n"
    echo -e "${YELLOW}Examples of location on Windows would be: \n ${BLUE}If admin account: \n ${NC}C:\ProgramData\ssh\administrator_authorized_keys \n ${BLUE}If standard account: \n ${NC}C:\Users\XXX\.ssh\authorized_keys\n "
    echo -e "${YELLOW}Example of location on Linux would be: \n ${BLUE}/home/username/.ssh/authorized_keys${NC}"
    echo -e "\n Press start to return to the menu..."
    read
    clear
    main_menu
}

# General help screen
info_screen(){
    clear
    echo -e "${BLUE}Info\n"
    echo -e "${NC}This app is designed to mount remote/external \n directories to local directories\n"
    echo -e "${NC}Example use cases include: \n   ${BLUE}- Mounting ROM folders stored on a PC/Server\n   - Mounting save folders\n   - Mounting external developer folders    \n "
    echo -e "${NC}In simple terms, this will allow you to play ROMS \n that are not actually located on this device. \n"
    echo -e "${NC}Support is available for this app however,\n you will need to know how to manage SSH Keys.\n"
    echo -e "${NC}For more information, select the \n\"Generate SSH Keys\" menu option or check the wiki"
    echo -e "\n Press start to exit..."
    read 
    clear
    main_menu
}

## Common functions
generate_sshfs_cmdline() {
    check_fuse
    local USERNAME=$1
    local SERVERADDR=$2
    local REMOTEDIR=$3
    local LOCALDIR=$4

    if [ -f "$KEY_SOURCE" ]; then
        cp "$KEY_SOURCE" "$KEY_DEST"
        chmod 600 "$KEY_DEST"
    else
        echo -e "${RED}Error: Keyfile not found. Please generate a key and copy the pubkey to the client.${NC}"
        sleep 5 
        main_menu
    fi
    
    local cmd="setsid sshfs -d -o ssh_command=\"ssh -F /mnt/SDCARD/.tmp_update/config/.sshcfg\" $USERNAME@$SERVERADDR:$REMOTEDIR $LOCALDIR >> /dev/null 2>&1 &"
    eval $cmd
    
    sleep 2

    if mount | grep -q "$LOCALDIR"; then
        echo -e "${GREEN}Mount successful${NC}"
        sleep 3
    else
        echo -e "${RED}Mount unsuccessful. Terminating sshfs and ssh processes and unmounting directory.${NC}"
        killall -9 sshfs
        killall -9 ssh
        fusermount -u "$LOCALDIR"
    fi
    main_menu
}

read_and_store(){
    read -p "Please select an option (or press start to cancel): " REPLY

    if [ -z "$REPLY" ]; then
        echo "No selection made"
        rm -f "$TEMP"
        main_menu
    fi

    local_dir=$(jq -r ".[$((REPLY-1))].local_dir" "$CONFIG_FILE")
    
}

check_fuse() {
    clear
    if lsmod | grep -q '^fuse '; then
        echo -e "${GREEN}Fuse active"
    else
        echo -e "${RED}Fuse not active, inserting..."
        if [ -f "$FUSEKO" ]; then
            echo -e "${GREEN}File exists, attempting to insert..."
            insmod $FUSEKO

            if lsmod | grep -q '^fuse '; then
                echo -e "${GREEN}Fuse active"
                return
            else
                echo -e "${RED}Failed to insert fuse, can't continue"
                sleep 2
                exit
            fi
        else
            echo -e "${RED}File does not exist, can't continue"
            sleep 2
            exit
        fi
    fi
}

prompt_user() { 
    while true; do
        echo -e "$1:"
        read response
        
        if [ "$response" == "q" ]; then
            echo "Returning to main menu..."
            breakout=1
            main_menu
            break
        fi

        case "$2" in
            "SERVERADDR")
                if echo "$response" | grep -qE '^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'; then
                    eval "$2=\"$response\""
                    break
                else
                    echo "Please enter a valid IP address."
                fi
                ;;
            "USERNAME")
                if [ ! -z "$response" ]; then
                    eval "$2=\"$response\""
                    break
                else
                    echo "Username cannot be empty. Please enter a valid username."
                fi
                ;;
            "LOCALDIR")
                if echo "$response" | grep -qE '^(/mnt/SDCARD)'; then
                    if [ -d "$response" ]; then
                        eval "$2=\"$response\""
                        break
                    else
                        echo "Directory does not exist. Creating it now..."
                        mkdir -p "$response"
                        eval "$2=\"$response\""
                        break
                    fi
                else
                    echo "Invalid directory. Only directories within /mnt/SDCARD can be mounted"
                fi
                ;;
            "REMOTEDIR")
                if [ ! -z "$response" ]; then
                    eval "$2=\"$response\""
                    break
                else
                    echo "Remote directory path cannot be empty. Please enter a valid path."
                fi
                ;;
            *)
                echo "Invalid input type"
                ;;
        esac
    done
}


list_mounts() {
    jq -c '.[]' "$CONFIG_FILE" | while IFS= read -r line; do
        local_dir=$(echo "$line" | jq -r '.local_dir')
        mounted=$(mount | grep -q "$local_dir"; echo $?)
        if [ "$mounted" -eq 0 ]; then
            mount_status="${GREEN}Yes${NC}"
        else
            mount_status="${RED}No${NC}"
        fi
        server_addr=$(echo $line | jq -r '.server_address')
        remote_dir=$(echo $line | jq -r '.remote_dir')
        echo -e "${BLUE}ID: ${NC}$count || ${BLUE}Mounted?: ${NC}${mount_status} || ${BLUE}Host: ${NC}$server_addr"
        echo -e "${BLUE}LD: ${NC}$local_dir"
        echo -e "${BLUE}RD: ${NC}$remote_dir\n\n"
        count=$((count+1))
    done
}

list_mounts_brief() {
    jq -c '.[]' "$CONFIG_FILE" | while IFS= read -r line; do
        local_dir=$(echo "$line" | jq -r '.local_dir')
        mounted=$(mount | grep -q "$local_dir"; echo $?)
        if [ "$mounted" -eq 0 ]; then
            mount_status="${GREEN}Yes${NC}"
        else
            mount_status="${RED}No${NC}"
        fi
        server_addr=$(echo $line | jq -r '.server_address')
        echo -e "${BLUE}ID: ${NC}$count || ${BLUE}Host: ${NC}$server_addr || ${BLUE}LD: ${NC}$local_dir"
        count=$((count+1))
    done
}


first_run(){
    create_banner "INFO" 
    echo -e "\n${YELLOW} Before going any further.. \n\n Have you setup the host/client keypair?\n" 
    echo -e "${YELLOW} !!This will only work if the server is accepting SSH connections!! \n ${NC}" 
    create_banner "INFO" 
    echo -e "${NC}\nPress start\n"
}


create_banner() {
    echo -e "${BLUE}################# $1 #################${NC}"
}

main
