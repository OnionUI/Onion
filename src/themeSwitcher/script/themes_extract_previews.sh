#!/bin/sh
theme_dir=/mnt/SDCARD/Themes

if [ ! -d "$theme_dir" ]; then
    exit 2
fi

extract_preview() {
    theme_path="$1"
    ext="$2"
    theme_name="$(basename "$theme_path" ".$ext")"

    if [ -d "$theme_dir/.previews/$theme_name" ]; then
        return
    fi

    echo "extracting preview: $theme_path"

    output=`7z x -aos -bd -bb1 -o"$theme_dir/.previews" "$theme_path" "*/config.json" "*/preview.png" "*/icons/gba.png" | grep '^- .*/config.json$' | sed 's/^- //g' | awk -v prefix="$theme_dir/.previews/" '$0=prefix $0'`
    sync

    echo "$output" | while read line; do
        output_dir="$(dirname "$line")"

        if [ "$output_dir" == "." ]; then
            continue
        fi

        if [ -d "$theme_dir/$(basename "$output_dir")" ]; then
            rm -rf "$output_dir"
        else
            echo "  -> $output_dir"
            echo "$theme_path" > "$output_dir/source"
        fi
    done

    echo "---"
}

for entry in "$theme_dir"/*.zip; do
    if [ ! -f "$entry" ]; then
        continue
    fi
    extract_preview "$entry" "zip"
done

for entry in "$theme_dir"/*.7z; do
    if [ ! -f "$entry" ]; then
        continue
    fi
    extract_preview "$entry" "7z"
done

for entry in "$theme_dir"/*.rar; do
    if [ ! -f "$entry" ]; then
        continue
    fi
    extract_preview "$entry" "rar"
done
