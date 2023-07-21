#!/bin/sh

rootdir="/mnt/SDCARD/Roms"
out='miyoogamelist.xml'

generate_miyoogamelist() {
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

        # .xml, .bak & .db files are no games
        # .bin files are no games, .cue files are!
        if echo "$rom" | grep -qE "\.(xml|bak|db|bin)$"; then
            continue
        fi

        filename="${rom%.*}"
        digest=$(echo "$rom" | grep -oE "^([^(]*)" | awk '{$1=$1};1')

        cat <<EOF >>$out
    <game>
        <path>./$rom</path>
        <name>$digest</name>
        <image>./Imgs/$filename.png</image>
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
        generate_miyoogamelist
    fi
done
