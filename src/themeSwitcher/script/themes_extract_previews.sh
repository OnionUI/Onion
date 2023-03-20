#!/bin/sh
theme_dir=/mnt/SDCARD/Themes

if [ ! -d "$theme_dir" ]; then
    exit 2
fi

main () {
    scan_for_archives "zip"
    scan_for_archives "7z"
    scan_for_archives "rar"
}

scan_for_archives() {
    ext="$1"
    for entry in "$theme_dir"/*.$ext; do
        if [ ! -f "$entry" ]; then
            continue
        fi
        check_archive "$entry" "$ext"
    done
}

check_archive() {
    archive_path="$1"
    ext="$2"
    archive_name="$(basename "$archive_path" ".$ext")"
    archive_hash=$(md5hash "$archive_path")

    if compare_hash "$theme_dir/$archive_name" "$archive_hash" ; then
        echo "[IGNORE] theme already installed: $archive_name"

        rm -f "$archive_path"

        if [ -d "$theme_dir/.previews/$archive_name" ]; then
            rm -rf "$theme_dir/.previews/$archive_name"
        fi

        return
    fi

    if compare_hash "$theme_dir/.previews/$archive_name" "$archive_hash" ; then
        echo "[IGNORE] found preview for: $archive_name"
        return
    fi

    echo "[CHECK] checking archive: $archive_path"

    file_list=`7z l -slt "$archive_path" "*/config.json" | grep '^Path = ' | sed 's/^Path = //g' | tail -n +2 | awk -v prefix="$theme_dir/.previews/" '$0=prefix $0'`

    echo "$file_list" | while read line; do
        output_dir="$(dirname "$line")"

        if [ "$output_dir" == "." ]; then
            continue
        fi

        extract_preview "$output_dir" "$archive_path" "$archive_hash"
    done
}

extract_preview() {
    preview_dir="$1"
    archive_path="$2"
    archive_hash="$3"
    theme_name="$(basename "$preview_dir")"

    if compare_hash "$theme_dir/$theme_name" "$archive_hash" ; then
        echo "  [IGNORE] theme already installed: $theme_name"

        if [ -d "$preview_dir" ]; then
            rm -rf "$preview_dir"
        fi

        return
    fi

    if compare_hash "$preview_dir" "$archive_hash" ; then
        echo "  [IGNORE] found preview for: $theme_name"
        return
    fi

    output=`7z x -aoa -bd -bb1 -o"$theme_dir/.previews" "$archive_path" "$theme_name/config.json" "$theme_name/preview.png" "$theme_name/icons/gba.png" | grep '^- .*/config.json$' | sed 's/^- //g' | awk -v prefix="$theme_dir/.previews/" '$0=prefix $0'`
    sync

    echo "$output" | while read line; do
        output_dir="$(dirname "$line")"

        if [ "$output_dir" == "." ]; then
            continue
        fi

        echo "  [EXTRACT] -> $output_dir"
        echo "$archive_hash" > "$output_dir/md5hash"
        echo "$archive_path" > "$output_dir/source"
    done
}

compare_hash() {
    comp_dir="$1"
    archive_hash="$2"

    if [ -f "$comp_dir/md5hash" ]; then
        comp_hash=$(cat "$comp_dir/md5hash")

        if [ "$archive_hash" == "$comp_hash" ]; then
            return 0
        fi
    fi

    return 1
}

md5hash() {
    echo `md5sum "$1" | awk '{ print $1; }'`
}

main
