#!/bin/sh
progdir=`dirname "$0"`
targetdir="$1"
dir_name=`basename "$targetdir"`

if [ ! -d "$progdir/$dir_name" ] || [ ! -d "$targetdir" ]; then
    exit
fi

echo "-- Copying common scripts to: $targetdir"

find "$targetdir" -name config.json -type f -exec dirname {} \; | (
    while read target ; do
        cp $progdir/$dir_name/* "$target"
    done
)