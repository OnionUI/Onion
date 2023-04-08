#!/bin/sh
patch="$1"
config="$2"

if [ "$config" == "" ]; then
    config=/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg
fi

if [ ! -f "$patch" ]; then
    echo "Patch file doesn't exist"
    return 1
fi

if [ ! -f "$config" ]; then
    echo "Config file doesn't exist"
    return 1
fi

cat "$patch" | (
    count=0
    content=`cat "$config"`

    while read line; do
        key=`echo "$line" | sed 's/^\s*//g' | sed 's/\s*=.*$//g'`
        value=`echo "$line" | sed 's/^[^\"]*"//g' | sed 's/"[^\"]*$//g'`

        if echo "$content" | grep -q "^\s*$key\s*="; then
            content=`echo "$content" | sed "/$key\s*=/c\${key} = \"$value\""`
        else
            content="$content
$key = \"$value\""
        fi

        let count++
    done

    echo "$content" > "$config"

    echo "$count lines patched"
)
