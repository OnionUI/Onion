#!/bin/sh
datadir="$1"
package="$2"
layer=$(basename "$datadir")

get_json_value() {
    grep "\"$1\"\s*:" | awk '{split($0,a,/\s*:\s*/); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n'
}

set_json_value() {
    key="$1"
    value="$2"
    file="$3"

    sed -i "s,\"$key\"\s*:\s*\".*\",\"$key\": \"$value\"," "$file"
}

cd "$datadir"

config=$(find "./$package" -name config.json -print | head -n 1)
config="/mnt/SDCARD/$(echo "$config" | sed "s:./$package::g")"
preserved_label=""
preserved_imgpath=""

if [ -f "$config" ]; then
    preserved_label=$(cat "$config" | get_json_value label)
    preserved_imgpath=$(cat "$config" | get_json_value imgpath)
fi

for FILE in ./"$package"/*; do
    cp -rf "$FILE" "/mnt/SDCARD/"
done

if [ -f "$config" ]; then
    if [ -n "$preserved_label" ]; then
        set_json_value label "$preserved_label" "$config"
    fi
    if [ -n "$preserved_imgpath" ]; then
        set_json_value imgpath "$preserved_imgpath" "$config"
    fi
fi

sync
