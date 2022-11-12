#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
radir=/mnt/SDCARD/RetroArch/.retroarch
ext_cache_path=$radir/cores/cache/core_extensions.cache

mkdir -p $radir/cores/cache

cd $sysdir
if [ ! -f $sysdir/cmd_to_run.sh ]; then
    echo "cmd_to_run.sh not found"
    exit 1
fi

cmd=`cat $sysdir/cmd_to_run.sh`
rompath=""
emupath=""
romext=""
romcfgpath=""
default_core=""
retroarch_core=""
corepath=""
coreinfopath=""
coreinfo=""
corename=""

main() {
    rm -f $sysdir/cmd_to_run.sh

    echo "cmd: $cmd"
    # example: LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so "/mnt/SDCARD/Emu/GBATEST/../../.tmp_update/proxy.sh" "/mnt/SDCARD/Emu/GBATEST/../../Roms/GBATEST/mGBA/Final Fantasy IV Advance (U).zip"

    rompath=$(echo "$cmd" | awk '{st = index($0,"\" \""); print substr($0,st+3,length($0)-st-3)}')
    echo "rompath: $rompath"
    # example: "/mnt/SDCARD/Emu/GBATEST/../../Roms/GBATEST/mGBA/Final Fantasy IV Advance (U).zip"

    if [ ! -f "$rompath" ]; then
        echo "rom file not found"
        exit 1
    fi

    emupath=`echo "$rompath" | awk '{st = index($0,"/../../"); print substr($0,0,st-1)}'`
    romext=`echo "$(basename "$rompath")" | awk -F. '{print tolower($NF)}'`

    echo "emupath: $emupath"
    echo "extension: $romext"

    menu_options=""
    menu_option_labels=""

    get_core_info

    if [ -f "$corepath" ] && [ -f "$coreinfopath" ]; then
        coreinfo=`cat "$coreinfopath"`
        corename=`get_info_value "$coreinfo" corename`

        game_core_label="Game core: $corename"

        if [ "$retroarch_core" == "$default_core" ]; then
            game_core_label="$game_core_label (Default)"
        fi

        menu_options="$menu_options reset_game change_core"
        menu_option_labels="$menu_option_labels \"Reset game\" \"$game_core_label\""
    fi

    menu_options="$menu_options filter_roms refresh_roms"
    menu_option_labels="$menu_option_labels \"Filter list\" \"Refresh roms\""

    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./bin/prompt -t \"GAME MENU\" $menu_option_labels"
    eval $runcmd
    retcode=$?

    echo "retcode: $retcode"

    menu_action=`echo $menu_options | awk -v N=$((retcode+1)) '{print $N}'`
    eval $menu_action

    exit 1
}

get_core_info() {
    launch_script=`cat "$emupath/launch.sh"`
    retroarch_core=""

    romcfgpath="$(dirname "$rompath")/$(basename "$rompath" ".$romext").db_cfg"

    if [ -f "$romcfgpath" ]; then
        romcfg=`cat "$romcfgpath"`
        retroarch_core=`get_info_value "$romcfg" core`
    fi

    default_core=`echo "$launch_script" | awk '{st = index($0,".retroarch/cores/"); s = substr($0,st+17); st2 = index(s,".so"); print substr(s,0,st2-1)}' | xargs`

    if [ "$retroarch_core" == "" ]; then
        retroarch_core="$default_core"
    fi

    echo "default_core: $default_core"
    echo "retroarch_core: $retroarch_core"

    corepath="$radir/cores/$retroarch_core.so"
    coreinfopath="$radir/cores/$retroarch_core.info"
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,"="); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n'
}

reset_game() {
    cat $radir/retroarch.cfg | sed 's/savestate_auto_load = "true"/savestate_auto_load = "false"/' > $sysdir/reset.cfg
    echo "LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./retroarch -v -c \"$sysdir/reset.cfg\" -L \"$corepath\" \"$rompath\"" > $sysdir/cmd_to_run.sh
    exit 0
}

change_core() {
    ext="$romext"

    if [ "$ext" == "zip" ] || [ "$ext" == "7z" ]; then
        if [ "$ext" == "zip" ]; then
            zip_files=`unzip -l "$rompath" | sed '1,3d;$d' | sed '$d' | sort -n -r`
        else
            zip_files=`./bin/7zz l -ba "$rompath" | awk '{$1="";$2="";$3="";print $0;}' | sort -n -r`
        fi

        echo "zip/7z output:"
        echo "$zip_files"

        inner_name=`basename "$(echo "$zip_files" | grep "[!]")"`
        if [ "$inner_name" == "" ]; then
            inner_name=`basename "$(echo "$zip_files" | head -n 1)"`
        fi
        ext=`echo "$inner_name" | awk -F. '{print tolower($NF)}'`
        echo "inner extension: $ext"
        echo "-------------------------------------"
    fi

    get_core_extensions
    available_corenames=""
    available_cores=""

    count=0
    is_valid=0
    selected_index=0

    single_ext_cache_path="$radir/cores/cache/ext_cores_$ext.cache"

    if [ ! -f "$single_ext_cache_path" ]; then
        while read entry; do
            tmp_extensions=`echo "$entry" | awk '{split($0,a,";"); print a[3]}'`

            if ! echo "$tmp_extensions" | tr '|' '\n' | grep -q "$ext"; then
                continue
            fi

            echo "$entry" >> $single_ext_cache_path
        done < $ext_cache_path
    fi

    while read entry; do
        tmp_corename=`echo "$entry" | awk '{split($0,a,";"); print a[1]}'`
        tmp_core=`echo "$entry" | awk '{split($0,a,";"); print a[2]}'`

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

    echo "cores: $available_cores"
    echo "corenames: $available_corenames"

    if [ $is_valid -eq 0 ]; then
        ./bin/infoPanel --title "GAME CORE" --message "Not available for this rom" --auto
        exit 1
    fi

    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./bin/prompt -t \"GAME CORE\" -s $selected_index $available_corenames"
    eval $runcmd
    retcode=$?

    if [ $retcode -lt 0 ] || [ $retcode -ge $count ]; then
        exit 1
    fi

    new_core=`echo $available_cores | awk -v N=$((retcode+1)) '{print $N}'`

    echo "new default core: $new_core"

    if [ "$new_core" == "$default_core" ]; then
        if [ -f "$romcfgpath" ]; then
            rm -f "$romcfgpath"
        fi
    else
        if [ -f "$romcfgpath" ]; then
            awk '!/core /' "$romcfgpath" > temp && mv temp "$romcfgpath"
        fi

        echo "core = \"$new_core\"" >> "$romcfgpath"
    fi
}

get_core_extensions() {
    if [ ! -f "$ext_cache_path" ]; then
        ./bin/infoPanel --title "CACHING CORES" --message "Caching core info\n \nThis may take a minute..." --persistent &

        for entry in "$radir/cores"/*.info ; do
            tmp_info=`cat "$entry"`
            tmp_core=`basename "$entry" .info`

            if [ ! -f "$radir/cores/$tmp_core.so" ]; then
                continue
            fi

            tmp_corename=`get_info_value "$tmp_info" "corename"`
            tmp_extensions=`get_info_value "$tmp_info" "supported_extensions"`

            echo "$tmp_corename;$tmp_core;$tmp_extensions" >> "$ext_cache_path"

            sort -f -o temp "$ext_cache_path"
            rm -f "$ext_cache_path"
            mv temp "$ext_cache_path"
        done

        touch /tmp/dismiss_info_panel
    fi
}

rename_rom() {
    prev_name="$(basename "$rompath" ".$romext")"
    runcmd="LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so ./bin/kbinput -i \"$prev_name\" -t \"RENAME ROM\""

    eval $runcmd > temp
    retcode=$?

    kboutput=`cat temp | tail -1`
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

filter_roms() {
    echo ":: filter roms"
    echo "./bin/filter filter \"$emupath\""
    ./bin/filter filter "$emupath"
}

refresh_roms() {
    echo ":: refresh roms"
    echo "./bin/filter refresh \"$emupath\""
    ./bin/filter refresh "$emupath"
}

main
