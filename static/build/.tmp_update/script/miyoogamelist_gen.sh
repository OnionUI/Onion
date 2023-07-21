#!/bin/sh

rootdir="/mnt/SDCARD/Emu"
out='miyoogamelist.xml'

generate_miyoogamelist() {
    rompath=$1
    imgpath=$2
    extlist=$3

    cd $rompath

    # create backup of previous miyoogamelist.xml
    if [ -f "$out" ]; then
        mv "$out" "$out.bak"
    fi

    echo '<?xml version="1.0"?>' >$out
    echo '<gameList>' >>$out

    for rom in *; do
        # ignore subfolders because miyoogamelist don't work with them
        # also ignores Imgs folder, nice
        if [ -d "$rom" ]; then
            continue
        fi

        # ignore non-game files
        if ! echo "$rom" | grep -qE "\.($extlist)$"; then
            continue
        fi

        filename="${rom%.*}"
        digest=$(echo "$rom" | grep -oE "^([^(\[]*)" | awk '{$1=$1};1')

        cat <<EOF >>$out
    <game>
        <path>./$rom</path>
        <name>$digest</name>
        <image>$imgpath/$filename.png</image>
    </game>
EOF
    done

    echo '</gameList>' >>$out
}

for system in "$rootdir"/*; do
    if [ -d "$system" ]; then
        # ignore Arcades & Ports since they have their own gamelist already
        if echo "$system" | grep -qE "(PORTS|FBA2012|FBNEO|MAME2000|ARCADE|NEOGEO|CPS1|CPS1|CPS3)"; then
            continue
        fi

        cd "$system"

        # read info from config.json
        rompath=$(grep -E '"rompath":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')
        extlist=$(grep -E '"extlist":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')
        imgpath=$(grep -E '"imgpath":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')
        imgpath=".${imgpath#$rompath}"

        generate_miyoogamelist "$rompath" "$imgpath" "$extlist"
    fi
done
