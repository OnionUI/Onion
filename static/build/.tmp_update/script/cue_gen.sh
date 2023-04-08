#!/bin/sh
rootdir="/mnt/SDCARD/Roms"
targets=PS SEGACD THIRTYTWOX

cd "$rootdir"

find $targets -name *.bin -type f | (
    count=0

    while read target ; do
        dir_path=`dirname "$target"`
        target_name=`basename "$target"`
        target_base="${target_name%.*}"
        cue_path="$dir_path/$target_base.cue"

        if echo "$target_base" | grep -q ' (Track [0-9][0-9]*)$'; then
            continue
        fi

        if [ -f "$cue_path" ]; then
            continue
        fi

        echo "FILE \"$target_name\" BINARY
  TRACK 01 MODE1/2352
    INDEX 01 00:00:00" > "$cue_path"

        let count++
    done

    echo "$count cue $([ $count -eq 1 ] && (echo "file") || (echo "files")) created"
)

find $targets -type f -name "*_cache2.db" -exec rm -f {} \;
