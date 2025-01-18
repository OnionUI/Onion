#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
radir=/mnt/SDCARD/RetroArch/.retroarch
ext_cache_path=$radir/cores/cache/core_extensions.cache
globalscriptdir=/mnt/SDCARD/App/romscripts

recentlist=/mnt/SDCARD/Roms/recentlist.json
recentlist_hidden=/mnt/SDCARD/Roms/recentlist-hidden.json
recentlist_temp=/tmp/recentlist-temp.json

UI_TITLE="GLO"

mkdir -p $radir/cores/cache

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

main() {
    if [ ! -f ./cmd_to_run.sh ]; then
        echo "cmd_to_run.sh not found"
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
        echo "tab: games"
    elif [ $current_tab -eq $TAB_FAVORITES ]; then
        echo "tab: favorites"
    elif [ $current_tab -eq $TAB_APPS ]; then
        echo "tab: apps"
    elif [ $current_tab -eq $TAB_RECENTS ]; then
        echo "tab: recents"
    elif [ $current_tab -eq $TAB_EXPERT ]; then
        echo "tab: expert"
    fi

    if [ $current_tab -eq $TAB_APPS ]; then
        exit
    fi

    rm -f ./cmd_to_run.sh 2> /dev/null

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

    echo "cmd: $cmd"
    # example: LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so "/mnt/SDCARD/Emu/GBATEST/../../.tmp_update/proxy.sh" "/mnt/SDCARD/Emu/GBATEST/../../Roms/GBATEST/mGBA/Final Fantasy IV Advance (U).zip"
    echo "romtype: $romtype"
    echo "rompath: $rompath"
    # example: "/mnt/SDCARD/Emu/GBATEST/../../Roms/GBATEST/mGBA/Final Fantasy IV Advance (U).zip"
    echo "romext: $romext"

    echo "emupath: $emupath"
    echo "emulabel: $emulabel"

    echo "romroot: $romroot"
    echo "launch: $launch_path"

    echo "has_networking: $has_networking"

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

            add_menu_option reset_game "Reset game"
        fi

        romdirname=$(echo "$rompath" | sed "s/^.*Roms\///g" | cut -d "/" -f1)
        manpath="/mnt/SDCARD/Roms/$romdirname/Manuals/$(basename "$rompath" ".$romext").pdf"

        echo "romdirname: $romdirname"
        echo "manpath: $manpath"

        if [ -f "$manpath" ]; then
            add_menu_option open_manual "Game manual"
        fi

        add_menu_option change_core "$game_core_label"

        if [ $add_reset_core -eq 1 ]; then
            add_menu_option reset_core "Restore default core"
        fi
    fi # skip_game_options

    if [ $current_tab -eq $TAB_GAMES ]; then
        if [ -f "$emupath/active_filter" ]; then
            filter_kw=$(cat "$emupath/active_filter")
            add_menu_option clear_filter "Clear filter"
            add_menu_option filter_roms "Filter: $filter_kw"
        else
            add_menu_option filter_roms "Filter list"
        fi
    fi

    if [ $current_tab -eq $TAB_GAMES ] || [ $current_tab -eq $TAB_EXPERT ]; then
        add_menu_option refresh_roms "Refresh list"
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

    # Show GLO menu
    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so prompt -t \"$UI_TITLE\" $(list_args "$menu_option_labels")"
    echo -e "\n\n=================================================================================================="
    echo -e "> $runcmd\n\n"
    eval $runcmd
    retcode=$?

    echo -e "\n\n=================================================================================================="
    echo "retcode: $retcode"

    # Execute action
    menu_action=$(get_item "$menu_options" $retcode)
    menu_arg=$(get_item "$menu_option_args" $retcode)
    runcmd="$menu_action $menu_arg"
    echo -e "> $runcmd"
    echo -e "\n\n==================================================================================================\n\n"
    eval $runcmd
    echo -e "\n\n=================================================================================================="

    exit 1
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
            echo "$entry"
            echo "require_networking: $require_networking"

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

            if [ "$scriptlabel" != "none" ]; then
                echo "adding romscript: $scriptlabel"
                add_menu_option run_script "$scriptlabel" "$entry"
            fi
        done
    fi
}

add_menu_option() {
    action="$1"
    label="$2"
    args="$3"
    if [ "$menu_options" == "" ]; then
        menu_options="$action"
        menu_option_labels="$label"
        menu_option_args="\"$args\""
    else
        menu_options=$(echo -e "$menu_options\n$action")
        menu_option_labels=$(echo -e "$menu_option_labels\n$label")
        menu_option_args=$(echo -e "$menu_option_args\n\"$args\"")
    fi
}

list_args() {
    echo "$1" | awk '{print "\\\""$0"\\\""}' | xargs echo
}

get_item() {
    index=$2
    echo "$1" | sed "$((index + 1))q;d"
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

    echo "default_core: $default_core"
    echo "retroarch_core: $retroarch_core"

    corepath="$radir/cores/$retroarch_core.so"
    coreinfopath="$radir/cores/$retroarch_core.info"

    export cookie_core_path="$corepath"
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,/\s*=\s*/); print a[2]}' | tr -d '"' | tr -d '\n'
}

get_json_value() {
    echo "$1" | grep "\"$2\"\s*:" | awk '{split($0,a,/\s*:\s*/); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n'
}

reset_game() {
    echo ":: reset_game $*"
    echo -e "savestate_auto_load = \"false\"\nconfig_save_on_exit = \"false\"\n" > /tmp/reset.cfg
    echo "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v --appendconfig \"/tmp/reset.cfg\" -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
    add_game_to_recent_list
    exit 0
}

open_manual() {
    echo ":: open_manual $*"
    if [ -f "$manpath" ]; then
        LD_LIBRARY_PATH="$sysdir/lib/parasyte:$LD_LIBRARY_PATH" $sysdir/bin/green/green "$manpath"
    fi
}

change_core() {
    echo ":: change_core $*"
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

            echo "zip/7z output:"
            echo "$zip_files"

            inner_name=$(basename "$(echo "$zip_files" | grep "[!]")")
            if [ "$inner_name" == "" ]; then
                inner_name=$(basename "$(echo "$zip_files" | head -n 1)")
            fi
            ext=$(echo "$inner_name" | awk -F. '{print tolower($NF)}')
            echo "inner extension: $ext"
            echo "-------------------------------------"

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

    echo "cores: $available_cores"
    echo "corenames: $available_corenames"

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

    echo "new default core: $new_core"

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
    echo ":: rename_rom $*"
    prev_name="$(basename "$rompath" ".$romext")"
    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./bin/kbinput -i \"$prev_name\" -t \"RENAME ROM\""

    eval $runcmd > temp
    retcode=$?

    kboutput=$(cat temp | tail -1)
    rm -f temp

    echo "kb retcode: $retcode"

    if [ $retcode -ne 0 ]; then
        return
    fi

    new_name="$kboutput"

    echo "rename: '$prev_name' -> '$new_name'"

    cd $sysdir
    ./bin/renameRom "$rompath" "$new_name"
}

clear_filter() {
    echo ":: clear_filter $*"
    echo "./bin/filter clear_filter \"$emupath\""
    filter clear_filter "$emupath"
}

filter_roms() {
    echo ":: filter_roms $*"
    echo "./bin/filter filter \"$emupath\""
    filter filter "$emupath"
}

refresh_roms() {
    echo ":: refresh_roms $*"
    echo "./bin/filter refresh \"$emupath\""
    filter refresh "$emupath"
    ./script/reset_list.sh "$romroot"
}

run_script() {
    echo ":: run_script $*"
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

main
