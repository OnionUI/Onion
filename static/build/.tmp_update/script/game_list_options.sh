#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
radir=/mnt/SDCARD/RetroArch/.retroarch
ext_cache_path=$radir/cores/cache/core_extensions.cache
globalscriptdir=/mnt/SDCARD/App/romscripts

recentlist=/mnt/SDCARD/Roms/recentlist.json
recentlist_hidden=/mnt/SDCARD/Roms/recentlist-hidden.json
recentlist_temp=/tmp/recentlist-temp.json

save_dir=/mnt/SDCARD/Saves/CurrentProfile/saves
gpsp_migration_log="$save_dir/gpSP/migration_status.log"

mkdir -p $radir/cores/cache

# Logging setup
logfile=$(basename "$0" .sh)
. $sysdir/script/log.sh

cd $sysdir

device_model=$(cat /tmp/deviceModel)
has_networking=$([ $device_model -eq 354 ] && echo 1 || echo 0)

ROM_TYPE_UNKNOWN=0
ROM_TYPE_GAME=1
ROM_TYPE_APP=2

TAB_GAMES=1
TAB_FAVORITES=2
TAB_APPS=3
TAB_RECENTS=10
TAB_EXPERT=16
current_tab=$(cat /tmp/state.json | grep "\"type\":" | sed -e 's/^.*:\s*//g' | sed -e 's/\s*,$//g' | xargs | awk '{ print $2 }')

cmd=$(cat ./cmd_to_run.sh 2> /dev/null)
rompath=""
emupath=""
emulabel=""
launch_path=""
romtype=$ROM_TYPE_UNKNOWN
romext=""
romroot=""
romcfgpath=""
default_core=""
retroarch_core=""
corepath=""
coreinfopath=""
coreinfo=""
corename=""
manpath=""

menu_options=""
menu_option_labels=""
menu_option_args=""
menu_option_info=""

main() {
    
    if [ ! -f ./cmd_to_run.sh ]; then
        log "cmd_to_run.sh not found"
        exit 1
    fi

    # Recent list entry removal
    if [ ! -f $sysdir/config/.showRecents ]; then
        currentrecentlist=$recentlist_hidden
    else
        currentrecentlist=$recentlist
    fi

    firstrecententry=$(head -n 1 "$currentrecentlist")
    sed '1d' $currentrecentlist > $recentlist_temp
    mv $recentlist_temp $currentrecentlist

    if [ $current_tab -eq $TAB_GAMES ]; then
        log "tab: games"
    elif [ $current_tab -eq $TAB_FAVORITES ]; then
        log "tab: favorites"
    elif [ $current_tab -eq $TAB_APPS ]; then
        log "tab: apps"
    elif [ $current_tab -eq $TAB_RECENTS ]; then
        log "tab: recents"
    elif [ $current_tab -eq $TAB_EXPERT ]; then
        log "tab: expert"
    fi

    if [ $current_tab -eq $TAB_APPS ]; then
        exit
    fi

    emupath=$(dirname $(echo "$cmd" | awk '{ gsub(/"/, "", $2); st = index($2,".."); if (st) { print substr($2,0,st) } else { print $2 } }'))

    if [ "$emupath" == "/mnt/SDCARD/App" ]; then
        romtype=$ROM_TYPE_APP
    else
        rompath=$(echo "$cmd" | awk '{ st = index($0,"\" \""); print substr($0,st+3,length($0)-st-3) }')

        if echo "$rompath" | grep -q ":"; then
            rompath=$(echo "$rompath" | awk '{st = index($0,":"); print substr($0,st+1)}')
        fi

        export cookie_rom_path="$rompath"

        if check_is_game "$rompath"; then
            romtype=$ROM_TYPE_GAME
        fi

        romext=$(echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}')

        emu_config_json=$(cat "$emupath/config.json")
        romroot="$emupath/$(get_json_value "$emu_config_json" "rompath")"
        emulabel="$(get_json_value "$emu_config_json" "label" | sed -e 's/^\s*//g' -e 's/\s*$//g')"
        launch_path="$emupath/launch.sh"
    fi

    if [ $current_tab -eq $TAB_FAVORITES ]; then
        emulabel="Favorites"
    elif [ $current_tab -eq $TAB_RECENTS ]; then
        emulabel="Recents"
    fi

    log "cmd: $cmd"
    # example: LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so "/mnt/SDCARD/Emu/GBATEST/../../.tmp_update/proxy.sh" "/mnt/SDCARD/Emu/GBATEST/../../Roms/GBATEST/mGBA/Final Fantasy IV Advance (U).zip"
    log "romtype: $romtype"
    log "rompath: $rompath"
    # example: "/mnt/SDCARD/Emu/GBATEST/../../Roms/GBATEST/mGBA/Final Fantasy IV Advance (U).zip"
    log "romext: $romext"

    log "emupath: $emupath"
    log "emulabel: $emulabel"

    log "romroot: $romroot"
    log "launch: $launch_path"

    log "has_networking: $has_networking"

    skip_game_options=0

    if [ ! -f "$rompath" ] || [ "$romext" == "miyoocmd" ] || [ $romtype -ne $ROM_TYPE_GAME ]; then
        skip_game_options=1
    fi

    if [ $skip_game_options -eq 0 ]; then
        get_core_info

        game_core_label="Game core"

        if [ ! -f "$radir/cores/$default_core.so" ]; then
            default_core=""
        fi

        add_reset_core=0

        if [ -f "$corepath" ] && [ -f "$coreinfopath" ]; then
            coreinfo=$(cat "$coreinfopath")
            corename=$(get_info_value "$coreinfo" corename)

            game_core_label="Game core: $corename"

            if [ "$retroarch_core" == "$default_core" ]; then
                game_core_label="$game_core_label (Default)"
            else
                add_reset_core=1
            fi

            add_menu_option reset_game "Reset game" "Starts the game without\nloading the auto save state"
        fi

        romdirname=$(echo "$rompath" | sed "s/^.*Roms\///g" | cut -d "/" -f1)
        romname=$(basename "$rompath" ".$romext")
        manpath="/mnt/SDCARD/Roms/$romdirname/Manuals/$romname.pdf"

        log "romdirname: $romdirname"
        log "manpath: $manpath"

        if [ -f "$manpath" ]; then
            add_menu_option open_manual "Game manual" "Opens the manual for this game"
        fi

        add_menu_option change_core "$game_core_label" "Change the RetroArch core\nused to run this game"

        if [ $add_reset_core -eq 1 ]; then
            add_menu_option reset_core "Restore default core" "Restores the default core\nfor this rom"
        fi

        # Add "Migrate mGBA Save" option dynamically
        if [ "$retroarch_core" == "gpsp_libretro" ] && [ -f "$save_dir/mGBA/$romname.srm" ]; then
            add_menu_option transfer_mgba_save "Transfer mGBA save" \
                "Transfer mGBA save to gpSP.\nConfirmation will be requested if a\nnon-empty save already exists." \
                "$romname"
        fi
    fi # skip_game_options

    if [ $current_tab -eq $TAB_GAMES ]; then
        if [ -f "$emupath/active_filter" ]; then
            filter_kw=$(cat "$emupath/active_filter")
            add_menu_option clear_filter "Clear filter" "Clear the current rom list filter"
            add_menu_option filter_roms "Current filter: $filter_kw" "Change the rom list filter"
        else
            add_menu_option filter_roms "Filter list" "Filter the rom list by keyword"
        fi
    fi

    if [ $current_tab -eq $TAB_GAMES ] || [ $current_tab -eq $TAB_EXPERT ]; then
        add_menu_option refresh_roms "Refresh list" "Refresh the rom list\n(re-scan for new games)"
    fi

    add_script_files "$globalscriptdir"

    if [ $current_tab -eq $TAB_GAMES ]; then
        add_script_files "$globalscriptdir/emu"
    elif [ $current_tab -eq $TAB_EXPERT ]; then
        add_script_files "$globalscriptdir/rapp"
    elif [ $current_tab -eq $TAB_FAVORITES ]; then
        add_script_files "$globalscriptdir/favorites"
    elif [ $current_tab -eq $TAB_RECENTS ]; then
        add_script_files "$globalscriptdir/recents"
    fi

    if [ $current_tab -eq $TAB_GAMES ] || [ $current_tab -eq $TAB_EXPERT ]; then
        add_script_files "$emupath/romscripts"
    fi

    log "\n\n=================================================================================================="
    
    # Display menu and get user selection
    show_prompt
    retcode=$?

    log "\n\n=================================================================================================="
    log "retcode: $retcode"

    # Execute chosen action
    menu_action=$(get_item "$menu_options" $retcode)
    menu_arg=$(get_item "$menu_option_args" $retcode)
    runcmd="$menu_action $menu_arg"
    log "> $runcmd"
    log "\n\n==================================================================================================\n\n"
    eval $runcmd
    log "\n\n=================================================================================================="

    rm -f ./cmd_to_run.sh 2> /dev/null

    exit 1
}

show_prompt() {
    scriptfile=$(mktemp)

    # Start command (use printf because info can contain newlines)
    echo -n "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t \"GLO\" -i" > "$scriptfile"

    echo "$menu_option_labels" | while IFS= read -r label; do
        printf ' "%s"' "$label"
    done >> "$scriptfile"

    echo "$menu_option_info" | while IFS= read -r info; do
        info_with_real_newlines=$(printf "%s" "$info" | sed 's/\\n/\n/g')
        printf ' "%s"' "$info_with_real_newlines"
    done >> "$scriptfile"

    echo >> "$scriptfile"  # ensure script ends with newline

    log "Executing prompt script: $(cat "$scriptfile")"

    chmod +x "$scriptfile"

    sh "$scriptfile"
    retcode=$?

    rm -f "$scriptfile"

    return $retcode
}

check_is_game() {
    echo "$1" | grep -q "retroarch" || echo "$1" | grep -q "/../../Roms/"
}

add_game_to_recent_list() {
    echo "$firstrecententry" | cat - "$currentrecentlist" > "$recentlist_temp"
    mv "$recentlist_temp" "$currentrecentlist"
}

add_script_files() {
    scriptdir="$1"
    if [ -d "$scriptdir" ]; then
        for entry in "$scriptdir"/*.sh; do
            if [ ! -f "$entry" ]; then
                continue
            fi

            require_networking=$([ $(get_info_value "$(cat "$entry")" require_networking) -eq 1 ] && echo 1 || echo 0)
            log "$entry"
            log "require_networking: $require_networking"

            if [ $has_networking -eq 0 ] && [ $require_networking -eq 1 ]; then
                continue
            fi

            scriptlabel=$(get_info_value "$(cat "$entry")" scriptlabel)

            if [ "$scriptlabel" == "" ]; then
                scriptlabel=$(basename "$entry" .sh)
            elif [ "$scriptlabel" == "DynamicLabel" ]; then
                # We run the script with "DynamicLabel" in third parameter to generate the name
                "$entry" "$rompath" "$emupath" "DynamicLabel" "$emulabel" "$retroarch_core" "$romdirname" "$romext"
                scriptlabel=$(cat /tmp/DynamicLabel.tmp)
                rm /tmp/DynamicLabel.tmp
            fi

            scriptlabel=$(echo "$scriptlabel" | sed "s/%LIST%/$emulabel/g")

            scriptinfo=$(get_info_value "$(cat "$entry")" scriptinfo)
            if [ "$scriptinfo" == "" ]; then
                scriptinfo="Runs the script:\n$scriptlabel"
            fi

            if [ "$scriptlabel" != "none" ]; then
                log "adding romscript: $scriptlabel"
                add_menu_option run_script "$scriptlabel" "$scriptinfo" "$entry"
            fi
        done
    fi
}

add_menu_option() {
    action="$1"
    label="$2"
    info="$3"
    args="$4"

    # Escape newlines in the info message for proper passing to `prompt`
    escaped_info=$(echo -e "$info" | sed ':a;N;$!ba;s/\n/\\n/g')

    if [ "$menu_options" == "" ]; then
        menu_options="$action"
        menu_option_labels="$label"
        menu_option_args="\"$args\""
        menu_option_info="$escaped_info"
    else
        menu_options="$menu_options
$action"
        menu_option_labels="$menu_option_labels
$label"
        menu_option_args="$menu_option_args
\"$args\""
        menu_option_info="$menu_option_info
$escaped_info"
    fi
}

get_item() {
    content="$1"
    index="$2"
    # Convert escaped newlines (\\n) back into real lines and extract the correct line
    echo -e "$content" | sed "$((index + 1))q;d"
}

get_core_info() {
    retroarch_core=""

    romcfgpath="$(dirname "$rompath")/.game_config/$(basename "$rompath" ".$romext").cfg"

    if [ -f "$romcfgpath" ]; then
        romcfg=$(cat "$romcfgpath")
        retroarch_core=$(get_info_value "$romcfg" core)
    fi

    if grep -q "default_core=" "$launch_path"; then
        default_core="$(cat "$launch_path" | grep "default_core=" | head -1 | awk '{split($0,a,"="); print a[2]}' | xargs)_libretro"
    else
        default_core=$(cat "$launch_path" | grep ".retroarch/cores/" | awk '{st = index($0,".retroarch/cores/"); s = substr($0,st+17); st2 = index(s,".so"); print substr(s,0,st2-1)}' | xargs)
    fi

    if [ "$retroarch_core" == "" ]; then
        retroarch_core="$default_core"
    fi

    log "default_core: $default_core"
    log "retroarch_core: $retroarch_core"

    corepath="$radir/cores/$retroarch_core.so"
    coreinfopath="$radir/cores/$retroarch_core.info"

    export cookie_core_path="$corepath"
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,/\s*=\s*/); print a[2]}' | tr -d '"' | tr -d '\n' | tr -d '\r'
}

get_json_value() {
    echo "$1" | grep "\"$2\"\s*:" | awk '{split($0,a,/\s*:\s*/); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n' | tr -d '\r'
}

reset_game() {
    log ":: reset_game $*"
    touch /tmp/reset_game
    add_game_to_recent_list
    exit 0
}

open_manual() {
    log ":: open_manual $*"
    if [ -f "$manpath" ]; then
        LD_LIBRARY_PATH="$sysdir/lib/parasyte:$LD_LIBRARY_PATH" $sysdir/bin/green/green "$manpath"
    fi
}

change_core() {
    log ":: change_core $*"
    ext="$romext"
    is_archive=""

    if [ "$ext" == "zip" ] || [ "$ext" == "7z" ]; then
        is_archive="$ext"

        if ! cat "$emupath/config.json" | grep -q "\"shortname\"\s*:\s*1"; then
            if [ "$ext" == "zip" ]; then
                zip_files=$(unzip -l "$rompath" | sed '1,3d;$d' | sed '$d' | sort -n -r)
            else
                zip_files=$(./bin/7z l -ba "$rompath" | awk '{$1="";$2="";$3="";print $0;}' | sort -n -r)
            fi

            log "zip/7z output:"
            log "$zip_files"

            inner_name=$(basename "$(echo "$zip_files" | grep "[!]")")
            if [ "$inner_name" == "" ]; then
                inner_name=$(basename "$(echo "$zip_files" | head -n 1)")
            fi
            ext=$(echo "$inner_name" | awk -F. '{print tolower($NF)}')
            log "inner extension: $ext"
            log "-------------------------------------"

        fi
    fi

    get_core_extensions
    available_corenames=""
    available_cores=""

    count=0
    is_valid=0
    selected_index=0

    while [ $is_valid -eq 0 ]; do
        single_ext_cache_path="$radir/cores/cache/ext_cores_$ext.cache"

        if [ ! -f "$single_ext_cache_path" ]; then
            while read entry; do
                tmp_extensions=$(echo "$entry" | awk '{split($0,a,";"); print a[3]}')

                if ! echo "$tmp_extensions" | tr '|' '\n' | grep -q "$ext"; then
                    continue
                fi

                echo "$entry" >> $single_ext_cache_path
            done < $ext_cache_path
        fi

        if [ "$default_core" == "" ]; then
            is_valid=1
        fi

        while read entry; do
            tmp_corename=$(echo "$entry" | awk '{split($0,a,";"); print a[1]}')
            tmp_core=$(echo "$entry" | awk '{split($0,a,";"); print a[2]}')

            if [ "$tmp_core" == "$default_core" ]; then
                is_valid=1
                tmp_corename="$tmp_corename (Default)"
            fi

            if [ "$tmp_core" == "$retroarch_core" ]; then
                selected_index=$count
            fi

            count=$(($count + 1))

            if [ $count -ge 100 ]; then
                break
            fi

            available_cores="$available_cores $tmp_core"
            available_corenames="$available_corenames \"$tmp_corename\""
        done < $single_ext_cache_path

        if [ "$is_archive" == "" ]; then
            break
        fi

        if [ $is_valid -eq 0 ]; then
            ext="$is_archive"
            is_archive=""
        fi
    done

    log "cores: $available_cores"
    log "corenames: $available_corenames"

    if [ $is_valid -eq 0 ]; then
        infoPanel --title "GAME CORE (.$ext)" --message "Not available for this rom\n(.$ext files)" --auto
        exit 1
    fi

    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./bin/prompt -t \"GAME CORE (.$ext)\" -s $selected_index $available_corenames"
    eval $runcmd
    retcode=$?

    if [ $retcode -lt 0 ] || [ $retcode -ge $count ]; then
        exit 1
    fi

    new_core=$(echo $available_cores | awk -v N=$((retcode + 1)) '{print $N}')

    log "new default core: $new_core"

    if [ "$new_core" == "$default_core" ]; then
        reset_core
    else
        if [ -f "$romcfgpath" ]; then
            awk '!/core /' "$romcfgpath" > temp && mv temp "$romcfgpath"
        else
            mkdir -p "$(dirname "$romcfgpath")"
        fi

        echo "core = \"$new_core\"" >> "$romcfgpath"
    fi
}

reset_core() {
    if [ -f "$romcfgpath" ]; then
        rm -f "$romcfgpath"
        rm -d "$(dirname "$romcfgpath")"
    fi
}

get_core_extensions() {
    if [ ! -f "$ext_cache_path" ]; then
        ./bin/infoPanel --title "CACHING CORES" --message "Caching core info\n \nThis may take a minute..." --persistent &
        ./script/build_ext_cache.sh "$radir"
        touch /tmp/dismiss_info_panel
        sync
    fi
}

rename_rom() {
    log ":: rename_rom $*"
    prev_name="$(basename "$rompath" ".$romext")"
    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./bin/kbinput -i \"$prev_name\" -t \"RENAME ROM\""

    eval $runcmd > temp
    retcode=$?

    kboutput=$(cat temp | tail -1)
    rm -f temp

    log "kb retcode: $retcode"

    if [ $retcode -ne 0 ]; then
        return
    fi

    new_name="$kboutput"

    log "rename: '$prev_name' -> '$new_name'"

    cd $sysdir
    ./bin/renameRom "$rompath" "$new_name"
}

clear_filter() {
    log ":: clear_filter $*"
    log "./bin/filter clear_filter \"$emupath\""
    filter clear_filter "$emupath"
}

filter_roms() {
    log ":: filter_roms $*"
    log "./bin/filter filter \"$emupath\""
    filter filter "$emupath"
}

refresh_roms() {
    log ":: refresh_roms $*"
    log "./bin/filter refresh \"$emupath\""
    filter refresh "$emupath"
    ./script/reset_list.sh "$romroot"
}

run_script() {
    log ":: run_script $*"
    scriptpath="$1"
    if [ -f "$scriptpath" ]; then
        cd "$emupath"
        chmod a+x "$scriptpath"
        LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so "$scriptpath" "$rompath" "$emupath"
        exit $?
    else
        infoPanel --title "SCRIPT NOT FOUND" --message "$(basename "$scriptpath")" --auto
    fi
}

transfer_mgba_save() {
    log ":: transfer_mgba_save $*"
    romname="$1"

    # Call the external transfer script, prompting user if needed
    $sysdir/script/transfer_mgba_save.sh "$romname"

    if [ $? -eq 0 ]; then
        log "Transfer successful, marking migration as done for ROM: $romname"

        # Remove rom from migration needed list
        if [ -f "$gpsp_migration_log" ]; then
            sed -i "/^$romname$/d" "$gpsp_migration_log"
        fi

        # Show info panel
        infoPanel --title "MIGRATION SUCCESSFUL" --message "mGBA save transferred to gpSP." --auto
    fi
}

main
