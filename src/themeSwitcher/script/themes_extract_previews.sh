#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The script scans /mnt/SDCARD/Themes for .zip, .7z, and .rar theme archives,
# then extracts only lightweight preview assets config.json, preview.png, and
# icons/gba.png into /mnt/SDCARD/Themes/.previews.
# It caches each archives size and modification time so unchanged archives
# can be skipped without recalculating MD5. MD5 hashes are used to detect
# already-installed themes or previews extracted from the same archive.
# When a matching theme is already installed, the archive and any redundant
# preview are removed.
# The upstream sync after extraction is deliberately omitted: previews are a
# regenerable cache and syncing here is costly on SD-backed storage.
# Usage: themes_extract_previews.sh

theme_dir=/mnt/SDCARD/Themes
previews_root="$theme_dir/.previews"
archive_cache_dir="$previews_root/.archives"

if [ ! -d "$theme_dir" ]; then
    exit 2
fi

mkdir -p "$archive_cache_dir"

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
    archive_key="$archive_name.$ext"
    archive_stat_path="$archive_cache_dir/$archive_key.stat"
    archive_themes_path="$archive_cache_dir/$archive_key.themes"
    current_stat=$(archive_stat "$archive_path")

    if [ -n "$current_stat" ] && \
       archive_cache_valid "$archive_path" "$archive_stat_path" \
           "$archive_themes_path" "$current_stat" ; then
        echo "[IGNORE] unchanged archive: $archive_name"
        return
    fi

    archive_hash=$(md5hash "$archive_path")

    if compare_hash "$theme_dir/$archive_name" "$archive_hash" ; then
        echo "[IGNORE] theme already installed: $archive_name"

        rm -f "$archive_path"

        if [ -d "$theme_dir/.previews/$archive_name" ]; then
            rm -rf "$theme_dir/.previews/$archive_name"
        fi

        rm -f "$archive_stat_path" "$archive_themes_path"
        return
    fi

    if compare_hash "$theme_dir/.previews/$archive_name" "$archive_hash" ; then
        echo "[IGNORE] found preview for: $archive_name"
        printf '%s\n' "$archive_name" > "$archive_themes_path"
        if [ -n "$current_stat" ]; then
            printf '%s\n' "$current_stat" > "$archive_stat_path"
        fi
        return
    fi

    echo "[CHECK] checking archive: $archive_path"

    file_list=`7z l -slt "$archive_path" "*/config.json" | grep '^Path = ' | sed 's/^Path = //g' | tail -n +2 | awk -v prefix="$theme_dir/.previews/" '$0=prefix $0'`
    archive_themes_tmp="$archive_themes_path.tmp.$$"
    : > "$archive_themes_tmp"

    echo "$file_list" | while IFS= read -r line; do
        output_dir="$(dirname "$line")"

        if [ "$output_dir" = "." ]; then
            continue
        fi

        theme_name="$(basename "$output_dir")"
        printf '%s\n' "$theme_name" >> "$archive_themes_tmp"
        extract_preview "$output_dir" "$archive_path" "$archive_hash"
    done

    # Only cache when at least one theme was found. An empty result can mean a
    # corrupt or unreadable archive rather than one genuinely containing no
    # themes; caching that would skip the archive on every later run until its
    # size or modification time changes.
    if [ -s "$archive_themes_tmp" ]; then
        mv -f "$archive_themes_tmp" "$archive_themes_path"
        if [ -n "$current_stat" ]; then
            printf '%s\n' "$current_stat" > "$archive_stat_path"
        fi
    else
        rm -f "$archive_themes_tmp"
    fi
}

extract_preview() {
    theme_preview_dir="$1"
    archive_path="$2"
    archive_hash="$3"
    theme_name="$(basename "$theme_preview_dir")"

    if compare_hash "$theme_dir/$theme_name" "$archive_hash" ; then
        echo "  [IGNORE] theme already installed: $theme_name"

        if [ -d "$theme_preview_dir" ]; then
            rm -rf "$theme_preview_dir"
        fi

        return
    fi

    if compare_hash "$theme_preview_dir" "$archive_hash" ; then
        echo "  [IGNORE] found preview for: $theme_name"
        return
    fi

    output=`7z x -aoa -bd -bb1 -o"$theme_dir/.previews" "$archive_path" "$theme_name/config.json" "$theme_name/preview.png" "$theme_name/icons/gba.png" | grep '^- .*/config.json$' | sed 's/^- //g' | awk -v prefix="$theme_dir/.previews/" '$0=prefix $0'`

    echo "$output" | while IFS= read -r line; do
        output_dir="$(dirname "$line")"

        if [ "$output_dir" = "." ]; then
            continue
        fi

        echo "  [EXTRACT] -> $output_dir"
        echo "$archive_hash" > "$output_dir/md5hash"
        echo "$archive_path" > "$output_dir/source"
    done
}

archive_cache_valid() {
    archive_path="$1"
    stat_path="$2"
    themes_path="$3"
    current_stat="$4"

    [ -f "$stat_path" ] || return 1
    [ -f "$themes_path" ] || return 1
    [ "$(cat "$stat_path")" = "$current_stat" ] || return 1

    while IFS= read -r theme_name; do
        [ -n "$theme_name" ] || continue

        if [ -f "$previews_root/$theme_name/config.json" ] &&
           [ -f "$previews_root/$theme_name/source" ] &&
           [ "$(cat "$previews_root/$theme_name/source")" = "$archive_path" ]; then
            continue
        fi

        [ -f "$theme_dir/$theme_name/config.json" ] || return 1
    done < "$themes_path"

    return 0
}

archive_stat() {
    # BusyBox stat supports this on Onion. If unavailable, the empty result
    # simply keeps the existing MD5-based behavior.
    stat -c '%s:%Y' "$1" 2>/dev/null
}

compare_hash() {
    comp_dir="$1"
    archive_hash="$2"

    if [ -f "$comp_dir/md5hash" ]; then
        comp_hash=$(cat "$comp_dir/md5hash")

        if [ "$archive_hash" = "$comp_hash" ]; then
            return 0
        fi
    fi

    return 1
}

md5hash() {
    md5sum "$1" | awk '{ print $1; }'
}

main
