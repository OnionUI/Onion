#!/bin/sh

rootdir="/mnt/SDCARD/Emu"

clean_name() {
    name="$1"
    extlist="$2"

    ### REMOVE NON-ESSENTIALS ###
    # remove file extensions
    while echo "$name" | grep -qE "\.($extlist)$"; do
        name="${name%.*}"
    done

    # remove everything in brackets
    name=$(echo "$name" | sed -e 's/([^)]*)//g')
    name=$(echo "$name" | sed -e 's/\[[^]]*\]//g')

    # remove rankings
    name=$(echo "$name" | sed -e 's/^[0-9]\+\.//')

    # trim
    name=$(echo "$name" | awk '{$1=$1};1')

    ### FORMAT ###
    # move article to the start of the name, if present
    article=$(echo "$name" | sed -ne 's/.*, \(A\|The\|An\).*/\1/p')
    if [ ! -z $article ]; then
        name="$article $(echo "$name" | sed -e 's/, \(A\|The\|An\)//')"
    fi

    # change " - " to ": " for subtitles
    name=$(echo "$name" | sed -e 's/ - /: /')

    echo "$name"
}

generate_miyoogamelist() {
    basepath=$1
    gamelistpath="$basepath/miyoogamelist.xml"
    rompath=$2
    imgpath=$3
    extlist=$4

    cd "$basepath/$rompath"

    # create backup of previous miyoogamelist.xml
    if [ -f "$gamelistpath" ]; then
        mv "$gamelistpath" "$gamelistpath.bak"
    fi

    echo '<?xml version="1.0"?>' >"$gamelistpath"
    echo '<gameList>' >>"$gamelistpath"

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
        digest=$(clean_name "$rom" "$extlist")

        cat <<EOF >>"$gamelistpath"
    <game>
        <path>$rompath/$rom</path>
        <name>$digest</name>
        <image>$imgpath/$filename.png</image>
    </game>
EOF
    done

    echo '</gameList>' >>"$gamelistpath"
}

for system in "$rootdir"/*; do
    if [ -d "$system" ]; then
        # ignore Arcades & Ports since they have their own gamelist already
        if echo "$system" | grep -qE "(PORTS|FBA2012|FBNEO|MAME2000|ARCADE|NEOGEO|CPS1|CPS2|CPS3)"; then
            continue
        fi

        cd "$system"

        # read info from config.json
        rompath=$(grep -E '"rompath":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')
        extlist=$(grep -E '"extlist":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')
        imgpath=$(grep -E '"imgpath":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')
        gamelist=$(grep -E '"gamelist":' config.json | sed -e 's/^.*:\s*"\(.*\)",*/\1/')

        basepath="${gamelist%/miyoogamelist.xml}"
        imgpath=".${imgpath#$basepath}"
        rompath=".${rompath#$basepath}"

        generate_miyoogamelist "$basepath" "$rompath" "$imgpath" "$extlist"
    fi
done
