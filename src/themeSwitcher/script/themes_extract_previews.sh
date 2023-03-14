#!/bin/sh
theme_dir=/mnt/SDCARD/Themes
PATH="/mnt/SDCARD/.tmp_update/bin:$PATH"

if [ ! -d "$theme_dir" ]; then
    exit 2
fi

rm -rf "$theme_dir/.previews"

extract_preview() {
    theme_path="$1"

    echo "extracting preview: $theme_path"

    output=`7z x -aoa -bd -bb1 -o"$theme_dir/.previews" "$theme_path" "*/config.json" "*/preview.png" "*/icons/gba.png" | grep '^- .*/config.json$' | sed 's/^- //g' | awk -v prefix="$theme_dir/.previews/" '$0=prefix $0'`

    while IFS= read -r line
    do
        output_dir="$(dirname "$line")"

        if [ -d "$theme_dir/$(basename "$output_dir")" ]; then
            rm -rf "$output_dir"
        else
            echo "  -> $output_dir"
            echo "$theme_path" > "$output_dir/source"
        fi
    done < <(printf '%s\n' "$output")

    echo "---"
}

for entry in "$theme_dir"/*.{zip,7z} ; do
    if [ ! -f "$entry" ]; then
        continue
    fi
    extract_preview "$entry"
done
