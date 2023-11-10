#!/bin/sh
romdir=/mnt/SDCARD/Roms/SCUMMVM
imgdir=/mnt/SDCARD/Roms/SCUMMVM/Imgs
scandir=/mnt/SDCARD/Roms/SCUMMVM/Shortcuts_Standalone
scummvm_config=/mnt/SDCARD/Saves/CurrentProfile/config/scummvm_standalone/.scummvmrc

ui_title="ScummVM Standalone Import"
naming_mode=0
skip_scan=0

if [ "$1" == "skip_scan" ]; then
    skip_scan=1
fi

main() {
    prompt -t "$ui_title" -m "Choose naming scheme" \
        "Use database name" \
        "Use database name (no parens)" \
        "Use directory name" \
        "Use directory name (no parens)"
    naming_mode=$?

    if [ $naming_mode -gt 3 ]; then
        echo "ScummVM import script canceled"
        exit
    fi

    echo -e "\n=================================== $ui_title ==================================="
    echo "Naming scheme: $naming_mode"

    if [ $skip_scan -eq 0 ]; then
        infoPanel -t "$ui_title" -m "Scanning..." --persistent 2> /dev/null &
        echo -e "\nScanning...\n"
        start=`date +%s`

        scummvm -c "$scummvm_config" -p "$romdir" --add --recursive 2> /dev/null

        end=`date +%s`
        echo -e "\n:: Scan finished in $((end-start))s"
    fi

    if [ ! -f "$scummvm_config" ]; then
        infoPanel -t "$ui_title" -m "No scan found\n \nPlease run import first" --auto 2> /dev/null
        exit
    fi

    scummvm_result="$(cat "$scummvm_config")"

    # removing all the old shortcuts
    rm -f $scandir/*.scummvm 2> /dev/null
	sync

    # here we get all the targets names
    echo "$scummvm_result" | sed -n 's/^[ \t]*\[\(.*\)\].*/\1/p' | (
        start=`date +%s`
        count=0

        while read target ; do
            # We skip the first Scummvm section which is not a game
            if [ "$target" = "scummvm" ]; then continue; fi

            # get the full name of the game:
            description=`echo "$scummvm_result" | sed -n "/^[ \t]*\["$target"]/,/\[/s/^[ \t]*description[ \t]*=[ \t]*//p"`

            # get the path of the game:
            game_path=`echo "$scummvm_result" | sed -n "/^[ \t]*\["$target"]/,/\[/s/^[ \t]*path[ \t]*=[ \t]*//p" `

            save_shortcut "$target" "$description" "$game_path"
            let count++
        done

        end=`date +%s`
        echo -e "\n:: Parsed in $((end-start))s"

        touch /tmp/dismiss_info_panel
		sync

        if [ $count -eq 0 ]; then
            result_message="No games found"
        elif [ $count -eq 1 ]; then
            result_message="Found 1 game"
        else
            result_message="Found $count games"
        fi

        echo -e "\n================================= DONE! $result_message =================================\n\n"
        infoPanel -t "$ui_title" -m "$result_message" --auto 2> /dev/null
    )

    rm -f /tmp/dismiss_info_panel 2> /dev/null

	# Reset list and cache
	/mnt/SDCARD/.tmp_update/script/reset_list.sh "$scandir"
}

save_shortcut() {
    target="$1"
    description="$2"
    game_path="$3"

    full_name="$(escape_filename "$description")"
    old_name="$(escape_filename_old "$description")"
    full_name_n="$(remove_parens "$full_name")"
    d_name="$(basename "$game_path")"
    d_name_n="$(remove_parens "$d_name")"

    echo -e "\n[$target]\n   name: $full_name\n   d_name: $d_name"

    image_path="$imgdir/$full_name.png"
    if [ ! -f "$image_path" ]; then
        image_path="$imgdir/$old_name.png"
        if [ ! -f "$image_path" ]; then
            image_path="$imgdir/$full_name_n.png"
            if [ ! -f "$image_path" ]; then
                image_path="$imgdir/$d_name.png"
                if [ ! -f "$image_path" ]; then
                    image_path="$imgdir/$d_name_n.png"
                fi
            fi
        fi
    fi

    if [ $naming_mode -eq 0 ]; then
        name="$full_name"
    elif [ $naming_mode -eq 1 ]; then
        name="$full_name_n"
    elif [ $naming_mode -eq 2 ]; then
        name="$d_name"
    elif [ $naming_mode -eq 3 ]; then
        name="$d_name_n"
    fi

    image_save_path="$imgdir/$name.png"
    target_save_path="$scandir/$name.scummvm"

    echo -ne "\n"

    if [ "$image_path" != "$image_save_path" ] && [ -f "$image_path" ]; then
        echo "   -> moving box art: '$(basename "$image_path")' -> '$(basename "$image_save_path")'"
        mv "$image_path" "$image_save_path"
    fi

    if [ -f "$target_save_path" ]; then
        target_save_path="$scandir/$name ($(echo "$target" | sed -e 's/:/, /g')).scummvm"
    fi

    echo "   -> \"$target\" > '$(basename "$target_save_path")'"
    echo "$target" > "$target_save_path"
}

escape_filename() {
    echo "$1" | sed -e 's/\s*:\s*/ - /g' | sed -e 's/\//, /g' | tr -cd "A-Z a-z0-9().,_'-"
}

escape_filename_old() {
    echo "$1" | sed -e 's/: / - /g' | tr -cd "A-Z a-z0-9()._'-"
}

remove_parens() {
    echo "$1" | sed -e 's/\s*([^)]*)//g' -e 's/\s*\[[^]]*\]//g'
}

main
