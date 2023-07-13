#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
prev_version=$(cat $sysdir/onionVersion/previous_version.txt)

if [ "$prev_version" == "4.2.0-beta-dev-65c7b31" ] ||
    [ "$prev_version" == "4.2.0-beta-dev-0763d40" ] ||
    [ "$prev_version" == "4.2.0-beta-dev-4c7e2db" ]; then
    echo -e "cheevos_enable = \"false\"\ncheevos_hardcore_mode_enable = \"false\"" > /tmp/temp_fix_patch.cfg

    ./script/patch_ra_cfg.sh /tmp/temp_fix_patch.cfg

    rm /tmp/temp_fix_patch.cfg
fi
