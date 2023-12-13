#!/bin/bash

mkdir -p cache
cd cache

wget -O featured.txt https://raw.githubusercontent.com/OnionUI/Themes/main/.github/data/featured.txt > /dev/null 2>&1
featured=`cat ./featured.txt`
rm -f ./featured.txt

readarray -t themes <<< "$featured"

f() { themes=("${BASH_ARGV[@]}"); }

shopt -s extdebug
f "${themes[@]}"
shopt -u extdebug

mkdir -p ../dist/Themes

for element in "${themes[@]}"
do
    zipfile="$element.zip"

    if [[ ! -f "$zipfile" ]]
    then
        echo "-- downloading theme: $element"
        wget -O "$zipfile" "https://github.com/OnionUI/Themes/raw/main/release/$element.zip" -q --show-progress
    fi

    if [ "$element" == "Silky by DiMo" ]; then
        echo "-- extracting theme: $element"
        unzip -oq "$zipfile" -d ../dist/Themes
    else
        echo "-- copying theme: $element"
        cp "$zipfile" ../dist/Themes
    fi
done
