#!/bin/sh
migration_dir=$(dirname "$0")
change_file="$migration_dir/data/overlay_filter_changes.tsv"
ra_dir=/mnt/SDCARD/RetroArch/.retroarch
config_dir=/mnt/SDCARD/Saves/CurrentProfile/config

deletions=0
skipped_deletions=0
moves=0
fixed_configs=0

usages=""

main() {
    usages=$(find_usages)
    echo -e ":: usages\n---\n$usages\n---"

    process_changes

    # Remove empty directories
    find "$ra_dir/overlay" -empty -type d -delete
    find "$ra_dir/filters/video" -empty -type d -delete

    echo "---"
    echo "Files deleted: $deletions ($skipped_deletions skipped / in use)"
    echo "Files moved: $moves ($fixed_configs configs amended)"
}

find_usages() {
    find "$config_dir" -name '*.cfg' -print0 |
        while IFS= read -r -d '' line; do
            overlay=$(get_info_value "$(cat "$line")" "input_overlay" | sed -E 's/(.*?)\.retroarch\///')
            filter=$(get_info_value "$(cat "$line")" "video_filter" | sed -E 's/(.*?)\.retroarch\///')

            shortpath=$(echo "$line" | sed -E 's/(.*?)\/config\///')

            if [ "$overlay" != "" ]; then
                echo "overlay	$overlay	$shortpath"
            fi
            if [ "$filter" != "" ]; then
                echo "filter	$filter	$shortpath"
            fi
        done
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,/\s*=\s*/); print a[2]}' | tr -d '"' | tr -d '\n'
}

process_changes() {
    echo ":: Processing changes: $change_file"
    while read entry; do
        local mode=$(echo "$entry" | cut -d'	' -f1)
        local src=$(echo "$entry" | cut -d'	' -f2)
        local dst=$(echo "$entry" | cut -d'	' -f3)

        if [ "$mode" == "D" ]; then
            if [ -f "$ra_dir/$src" ]; then
                delete_file "$src"
            fi
        fi
        if [ "$mode" == "M" ]; then
            if [ -f "$ra_dir/$src" ]; then
                move_file "$src" "$dst"
            fi
        fi
    done < "$change_file"
}

delete_file() {
    local src=$1
    local path_src="$ra_dir/$src"

    if [ ! -f "$path_src" ]; then
        return
    fi

    if ! echo "$usages" | grep -q "	$src	"; then
        echo "Delete: $src"
        rm "$path_src"
        deletions=$((deletions + 1))
    else
        skipped_deletions=$((skipped_deletions + 1))
    fi
}

move_file() {
    local src=$1
    local path_src="$ra_dir/$src"
    local path_dst="$ra_dir/$dst"

    if [ ! -f "$path_src" ]; then
        return
    fi

    if [ -f "$path_dst" ]; then
        echo "Cleanup: $src (moved to $dst)"
        rm "$path_src"
    else
        echo "Move: $src -> $dst"
        mv "$path_src" "$path_dst"
    fi

    usage=$(echo "$usages" | grep "	$src	")
    if [ "$usage" != "" ]; then
        fix_config "$usage" "$dst"
        fixed_configs=$((fixed_configs + 1))
    fi

    moves=$((moves + 1))
}

fix_config() {
    local usage=$1
    local dst=$2
    local src=$(echo "$usage" | cut -d'	' -f2)
    local config_file=$(echo "$usage" | cut -d'	' -f3)
    local config_path="$config_dir/$config_file"

    if [ -f "$config_path" ]; then
        echo -e "Fix config: $config_file ($dst -> $src)"
        sed -i "s,$src,$dst," "$config_path"
    fi
}

main
