#!/bin/sh
romdir=/mnt/SDCARD/Roms/SCUMMVM
imgdir=/mnt/SDCARD/Roms/SCUMMVM/Imgs
scandir=/mnt/SDCARD/Emu/SCUMMVM/../../Roms/SCUMMVM/Shortcuts
scummvm_config=/mnt/SDCARD/BIOS/scummvm.ini

ui_title="ScummVM Import"

naming_mode=0

count=0
target=""
full_name=""
old_name=""
game_path=""

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

    infoPanel -t "$ui_title" -m "Scanning..." --persistent 2> /dev/null &

    echo -e "\n=================================== $ui_title ==================================="
    echo "Naming scheme: $naming_mode"
    echo "Scanning..."
    start=`date +%s`

    scummvm -c "$scummvm_config" -p "$romdir" --add --recursive 2> /dev/null

    end=`date +%s`
    echo -e "\n:: Scan finished in $((end-start))s"

    # removing all the old shortcuts
    rm -f $scandir/*.target 2> /dev/null
	sync

    start=`date +%s`
    ignore=1
	save_count=0
        
    while read line ; do
        if echo "$line" | grep -q '^\s*\[[^]]*]'; then
            target="$(echo "$line" | sed -n 's/^\s*\[\([^]]*\)\].*/\1/p')"

            if [ "$target" == "scummvm" ]; then
                ignore=1
            else
                ignore=0
            	let count++
            fi

            continue
        fi

        if [ $ignore -eq 1 ]; then
            continue
        fi

        if echo "$line" | grep -q '^description='; then
            value="$(echo "$line" | sed 's/^description=//g')"
            full_name="$(escape_filename "$value")"
            old_name="$(escape_filename_old "$value")"
        elif echo "$line" | grep -q '^path='; then
            game_path="$(echo "$line" | sed 's/^path=//g')"
        elif [ "$line" == "" ]; then
            save_shortcut
            ignore=1
			let save_count++
        fi
    done < $scummvm_config

    if [ $count -gt $save_count ]; then
        save_shortcut
    fi

    end=`date +%s`
    echo -e "\n:: Parsed in $((end-start))s"

    touch /tmp/dismiss_info_panel

    if [ $count -eq 0 ]; then
        result_message="No games found"
    elif [ $count -eq 1 ]; then
        result_message="Found 1 game"
    else
        result_message="Found $count games"
    fi

    echo -e "\n================================= DONE! $result_message =================================\n\n"
    infoPanel -t "$ui_title" -m "$result_message" --auto 2> /dev/null

	# Reset list and cache
	/mnt/SDCARD/.tmp_update/script/reset_list.sh "$scandir"
}

save_shortcut() {
    full_name_n="$(remove_parens "$full_name")"
    d_name="$(basename "$game_path")"
    d_name_n="$(remove_parens "$d_name")"

    echo -e "\n$count: [$target]\n   name: $full_name\n   d_name: $d_name"

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
    target_save_path="$scandir/$name.target"

    echo -ne "\n"

    if [ "$image_path" != "$image_save_path" ] && [ -f "$image_path" ]; then
        echo "   -> moving box art: '$(basename "$image_path")' -> '$(basename "$image_save_path")'"
        mv "$image_path" "$image_save_path"
    fi

    if [ -f "$target_save_path" ]; then
        target_save_path="$scandir/$name ($(echo "$target" | sed -e 's/:/, /g')).target"
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
