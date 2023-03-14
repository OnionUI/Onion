#!/bin/sh
preview_dir="$1"
theme_dir="/mnt/SDCARD/Themes"

if [ ! -f "$preview_dir/source" ]; then
    exit 2
fi

theme_name="$(basename "$preview_dir")"
theme_path="$(cat "$preview_dir/source")"
ext=`echo "$(basename "$theme_path")" | awk -F. '{print tolower($NF)}'`

if [ ! -f "$theme_path" ]; then
    exit 2
fi

echo "theme: $theme_name"
echo "path: $theme_path"

# extract theme from compressed file
7z x -aoa -bd -o"$theme_dir" "$theme_path" "$theme_name/*" > /dev/null

if [ -d "$theme_dir/$theme_name" ]; then
    rm -rf "$theme_dir/.previews/$theme_name"
    
    output=`7z l -slt "$theme_path" "*/config.json" | grep -oP "(?<=Path = ).+" | tail -n +2 | awk -v prefix="$theme_dir/" '$0=prefix $0'`

    do_remove=1

    while IFS= read -r line
    do
        output_dir="$(dirname "$line")"
        if [ ! -d "$output_dir" ]; then
            do_remove=0
            break
        fi
    done < <(printf '%s\n' "$output")

    if [ $do_remove -eq 1 ]; then
        rm -f "$theme_path"
    fi
fi
