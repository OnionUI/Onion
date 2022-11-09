#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
radir=/mnt/SDCARD/RetroArch/.retroarch
extpath=$radir/cores/core_extensions.csv

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

    get_core_info

    if [ ! -f "$corepath" ] || [ ! -f "$coreinfopath" ]; then
        exit 1
    fi

    coreinfo=`cat "$coreinfopath"`
    corename=`get_info_value "$coreinfo" corename`

    LD_PRELOAD="/mnt/SDCARD/miyoo/lib/libpadsp.so" ./bin/prompt -t "GAME MENU" \
        "Reset game" \
        "Default core: $corename" \
        "Rename" \
        "Filter list" \
        "Refresh roms"
    retcode=$?

    echo "retcode: $retcode"

    if [ $retcode -eq 0 ]; then
        reset_game
    elif [ $retcode -eq 1 ]; then
        change_core
    elif [ $retcode -eq 2 ]; then
        rename_rom
    elif [ $retcode -eq 3 ]; then
        filter_roms
    elif [ $retcode -eq 4 ]; then
        refresh_roms
    fi

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

    if [ "$retroarch_core" == "" ]; then
        retroarch_core=`echo "$launch_script" | awk '{st = index($0,".retroarch/cores/"); s = substr($0,st+17); st2 = index(s,".so"); print substr(s,0,st2-1)}' | xargs`
    fi

    echo "ra_core: $retroarch_core"

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

    if [ "$ext" == "zip" ]; then
        tmp_files="`unzip -l "$rompath" | sed '1,3d;$d' | sed '$d' | awk '{$1=""; $2=""; $3=""; print $0}'`"
        inner_name=`basename "$(echo "$tmp_files" | grep "[!]")"`
        if [ "$inner_name" == "" ]; then
            inner_name=`basename "$(echo "$tmp_files" | head -n 1)"`
        fi
        ext=`echo "$inner_name" | awk -F. '{print tolower($NF)}'`
        echo "$ext"
        echo "-------------------------------------"
    fi

    get_core_extensions
    count=0
    available_corenames=""
    available_cores=""

    while read entry; do
        tmp_core=`echo "$entry" | awk '{split($0,a,","); print a[1]}'`
        tmp_corename=`echo "$entry" | awk '{split($0,a,","); print a[2]}'`
        tmp_extensions=`echo "$entry" | awk '{split($0,a,","); print a[3]}'`

        if ! echo "$tmp_extensions" | tr '|' '\n' | grep -q "$ext"; then
            continue
        fi

        count=$(($count + 1))

        if [ $count -ge 100 ]; then
            break
        fi

        available_cores="$available_cores $tmp_core"
        available_corenames="$available_corenames \"$tmp_corename\""
    done < $extpath

    echo "cores: $available_cores"
    echo "corenames: $available_corenames"

    runcmd="LD_PRELOAD=\"/mnt/SDCARD/miyoo/lib/libpadsp.so\" ./bin/prompt -t \"DEFAULT CORE\" $available_corenames"
    eval $runcmd
    retcode=$?

    if [ $retcode -lt 0 ] || [ $retcode -ge $count ]; then
        exit 1
    fi

    new_core=`echo $available_cores | awk -v N=$((retcode+1)) '{print $N}'`

    echo "new default core: $new_core"

    if [ -f "$romcfgpath" ]; then
        awk '!/core /' "$romcfgpath" > temp && mv temp "$romcfgpath"
    fi

    echo "core = \"$new_core\"" >> "$romcfgpath"
}

get_core_extensions() {
    if [ ! -f "$extpath" ]; then
        for entry in "$radir/cores"/*.info ; do
            tmp_info=`cat "$entry"`
            tmp_core=`basename "$entry" .info`

            if [ ! -f "$radir/cores/$tmp_core.so" ]; then
                continue
            fi

            tmp_corename=`get_info_value "$tmp_info" "corename"`
            tmp_extensions=`get_info_value "$tmp_info" "supported_extensions"`

            echo "$tmp_core,$tmp_corename,$tmp_extensions" >> "$extpath"
        done
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
    return
}

refresh_roms() {
    return
}

main
