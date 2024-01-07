#!/bin/sh

old_dir="/mnt/SDCARD/App/PackageManager/data/App/Battery monitor tool"

if [ -d "$old_dir" ]; then
    rm -rf "$old_dir"
fi
