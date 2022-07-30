#!/bin/bash

mkdir -p .temp
cd .temp

wget -O featured.txt https://raw.githubusercontent.com/OnionUI/Themes/main/.github/data/featured.txt > /dev/null 2>&1
featured=`cat ./featured.txt`

readarray -t themes <<< "$featured"

f() { themes=("${BASH_ARGV[@]}"); }

shopt -s extdebug
f "${themes[@]}"
shopt -u extdebug

for element in "${themes[@]}"
do
    echo ":: downloading theme: $element"
    wget -O "$element.zip" "https://github.com/OnionUI/Themes/blob/main/release/$element.zip?raw=true" > /dev/null 2>&1
    unzip -oq "$element.zip" -d ../build/Themes
done

cd ..
rm -rf .temp/
