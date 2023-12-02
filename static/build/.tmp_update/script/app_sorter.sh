create_temp_dir() {
    mkdir -p /mnt/SDCARD/App.temp
    for dir in /mnt/SDCARD/App/*/; do
        if [ -f "$dir/config.json" ]; then
            mv "$dir" /mnt/SDCARD/App.temp/
        fi
    done
}

read_and_store_labels() {
    temp_file="/mnt/SDCARD/temp_labels.txt"
    : > "$temp_file"

    for dir in /mnt/SDCARD/App.temp/*/; do
        if [ -f "$dir/config.json" ]; then
            label=$(awk -F'"' '/"label":/ {print $4}' "$dir/config.json")
            echo "$label:$dir" >> "$temp_file"
        fi
    done
}

sort_and_copy_back() {
    sort_order="$1"
    if [ "$sort_order" = "desc" ]; then
        sort_flag="-rf"
    else
        sort_flag="-f"
    fi

    sort $sort_flag "$temp_file" | while IFS=: read -r label dir; do
        mv "$dir" /mnt/SDCARD/App/
    done
}

cleanup() {
    #restart mainui to refresh app list (if called from a button/script while main is running)
    if pgrep "MainUI" > /dev/null; then
        pkill -2 "MainUI"
    fi

    rm -rf /mnt/SDCARD/App.temp
    rm "$temp_file"
}

main() {
    sort_order="${1:-asc}"
    create_temp_dir
    read_and_store_labels
    sort_and_copy_back "$sort_order"
    cleanup
}

main "$@"