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
7z x -aoa -bd -o"$theme_dir" "$theme_path" "$theme_name/*"
sync

if [ -d "$theme_dir/$theme_name" ]; then
    rm -rf "$theme_dir/.previews/$theme_name"
    
    output=`7z l -slt "$theme_path" "*/config.json" | grep '^Path = ' | sed 's/^Path = //g' | tail -n +2 | awk -v prefix="$theme_dir/" '$0=prefix $0'`

    touch /tmp/remove_theme_archive

    echo "$output" | while read line; do
        output_dir="$(dirname "$line")"
        if [ ! -d "$output_dir" ]; then
            rm -f /tmp/remove_theme_archive
            break
        fi
    done

    if [ -f /tmp/remove_theme_archive ]; then
        echo "deleting: $theme_path"
        rm -f "$theme_path"
        rm -f /tmp/remove_theme_archive
    fi
fi
