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
    echo -e "\n\n:: MIGRATION: OVERLAY AND FILTER CHANGES"

    usages=$(find_usages)
    echo -e ":: usages\n---\n$usages\n---"

    process_changes

    # Remove empty directories
    find "$ra_dir/overlay" -type d -exec rmdir {} \; > /dev/null
    find "$ra_dir/filters/video" -type d -exec rmdir {} \; > /dev/null

    echo "---"
    echo "Files deleted: $deletions ($skipped_deletions skipped / in use)"
    echo "Files moved: $moves ($fixed_configs configs amended)"
}

find_usages() {
    echo "$(find "$config_dir" -name '*.cfg' -exec echo {} \;)" > /tmp/config_files.txt

    while read line; do
        overlay=$(get_info_value "$(cat "$line")" "input_overlay" | sed -e 's/.*\.retroarch\///')
        filter=$(get_info_value "$(cat "$line")" "video_filter" | sed -e 's/.*\.retroarch\///')

        shortpath=$(echo "$line" | sed -e 's/.*\/config\///')

        if [ -f "$ra_dir/$overlay" ]; then
            echo "overlay	$overlay	$shortpath"
            echo "overlay	$(dirname "$overlay")/$(get_info_value "$(cat "$ra_dir/$overlay")" "overlay0_overlay")	"
        fi
        if [ -f "$ra_dir/$filter" ]; then
            echo "filter	$filter	$shortpath"
            echo "filter	$(dirname "$filter")/$(get_info_value "$(cat "$ra_dir/$filter")" "filter").so	"
        fi
    done < /tmp/config_files.txt

    rm /tmp/config_files.txt
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
            delete_file "$src"
        fi
        if [ "$mode" == "M" ]; then
            move_file "$src" "$dst"
        fi
    done < "$change_file"
}

delete_file() {
    local src=$1
    local path_src="$ra_dir/$src"

    if ! echo "$usages" | grep -q "	$src	"; then
        if [ -f "$path_src" ]; then
            echo "Delete: $src"
            rm "$path_src"
            deletions=$((deletions + 1))
        fi
    else
        skipped_deletions=$((skipped_deletions + 1))
    fi
}

move_file() {
    local src=$1
    local dst=$2
    local path_src="$ra_dir/$src"
    local path_dst="$ra_dir/$dst"

    usage=$(echo "$usages" | grep "	$src	")
    if [ "$usage" != "" ]; then
        fix_config "$usage" "$dst"
        fixed_configs=$((fixed_configs + 1))
    fi

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

    moves=$((moves + 1))
}

fix_config() {
    local usage=$1
    local dst=$2
    local type=$(echo "$usage" | cut -d'	' -f1)
    local src=$(echo "$usage" | cut -d'	' -f2)
    local config_file=$(echo "$usage" | cut -d'	' -f3)
    local config_path="$config_dir/$config_file"

    if [ -f "$config_path" ]; then
        echo -e "Fix config: $config_file ($dst -> $src)"

        if [ "$type" == "overlay" ]; then
            value="input_overlay = \":/.retroarch/$dst\""
        else
            value="video_filter = \":/.retroarch/$dst\""
        fi

        echo "$value" > /tmp/temp_patch.cfg
        ./script/patch_ra_cfg.sh "/tmp/temp_patch.cfg" "$config_path"
        rm /tmp/temp_patch.cfg
    fi
}

main
