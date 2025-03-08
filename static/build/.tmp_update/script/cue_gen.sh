#!/bin/sh
rootdir="/mnt/SDCARD/Roms"

if [ $# -gt 0 ]; then
    targets="$1"
else
    targets="PS SEGACD NEOCD PCE PCFX AMIGA"
fi

cd "$rootdir"

find $targets -maxdepth 3 -name "*.bin" -type f 2>/dev/null | sort | (
    count=0

    while read target_platform; do
        dir_path=$(dirname "$target_platform")
        target_name=$(basename "$target_platform")
        target_base="${target_name%.*}"
        cue_path="$dir_path/$target_base.cue"

        # strip filename of () and trim
        game_name=$(echo "$target_name" | sed -E 's/\(.*\)//g' | sed 's/\..*$//' | sed 's/[[:space:]]*$//')

        # Extract track number if present
        track_number=$(echo "$target_base" | sed -E 's/.*(Track|Disc|Disk) ([0-9]+).*/\2/;t;d')

        # If no track number found, use "01"
        if [ -z "$track_number" ]; then
            track_number="01"
        fi

        # Add leading zero to track number if it is a single digit
        if echo "$track_number" | grep -q '^[0-9]$'; then
            track_number="0$track_number"
        fi

        # skip for non first tracks
        if echo "$track_number" | grep -q '01'; then
            # rewrite cue
            cue="FILE \"$game_name\" BINARY
    TRACK $track_number MODE1/2352
        INDEX 01 00:00:00"
            echo "$cue"
            echo "$cue" >"$cue_path"

            count=$((count + 1))
        fi
    done

    echo "$count cue $([ $count -eq 1 ] && echo "file" || echo "files") created for $target_platform"
)

find $targets -maxdepth 1 -type f -name "*_cache6.db" -exec rm -f {} \;

echo "Success"
