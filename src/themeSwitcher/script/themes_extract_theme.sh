#!/bin/sh
preview_dir="$1"
theme_dir="/mnt/SDCARD/Themes"

if [ ! -f "$preview_dir/source" ]; then
    exit 2
fi

theme_name="$(basename "$preview_dir")"
archive_path="$(cat "$preview_dir/source")"
ext=`echo "$(basename "$archive_path")" | awk -F. '{print tolower($NF)}'`

if [ ! -f "$archive_path" ]; then
    exit 2
fi

main () {
    echo "theme: $theme_name"
    echo "path: $archive_path"

    # extract theme from compressed file
    7z x -aoa -bd -o"$theme_dir" "$archive_path" "$theme_name/*"
    sync

    if [ -d "$theme_dir/$theme_name" ]; then
        rm -rf "$theme_dir/.previews/$theme_name"
        
        file_list=`7z l -slt "$archive_path" "*/config.json" | grep '^Path = ' | sed 's/^Path = //g' | tail -n +2 | awk -v prefix="$theme_dir/" '$0=prefix $0'`

        md5hash "$archive_path" > "$theme_dir/$theme_name/md5hash"

        if check_should_remove "$file_list" ; then
            echo "deleting: $archive_path"
            rm -f "$archive_path"
        fi
    fi
}

check_should_remove() {
    file_list="$1"
    touch /tmp/remove_theme_archive

    echo "$file_list" | while read line; do
        output_dir="$(dirname "$line")"
        if [ ! -d "$output_dir" ]; then
            rm -f /tmp/remove_theme_archive
            break
        fi
    done

    if [ -f /tmp/remove_theme_archive ]; then
        rm -f /tmp/remove_theme_archive
        return 0
    fi

    return 1
}

md5hash() {
    echo `md5sum "$1" | awk '{ print $1; }'`
}

main
