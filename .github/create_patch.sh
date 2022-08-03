#!/bin/bash

USER=OnionUI
REPO=Onion
TARGET=Onion

unzip_progress() {
    zipfile=$1
    dest=$2
    if [[ dest = "" ]]
    then
        dest=.
    fi
    total=`unzip -l "$zipfile" | tail -1 | grep -Eo "([0-9]+) files" | sed "s/[^0-9]*//g"`
    # unzip -o "$zipfile" | awk -v total="$total" 'function bname(file,a,n){n=split(file,a,"/");return a[n]!="" ? a[n] : a[n-1]}BEGIN{cnt=0;printf ""}{act=$1;$1=""; printf "%3.0f%% - %s %s\n",cnt*100/total,act,bname($0); cnt+=1;}'
    unzip -o -d "$dest" "$zipfile" | awk -v total="$total" 'BEGIN{cnt=0}{printf "%3.0f%%\r",cnt*100/total; cnt+=1}'
    echo -ne '100%\r\n'
}

LATEST_URL=`curl -s https://api.github.com/repos/$USER/$REPO/releases/latest \
| grep -w "browser_download_url.*https://github.com/$USER/$REPO/releases/download/.*/$TARGET.*\.zip" \
| cut -d : -f 2,3 \
| tr -d "[:space:]\""`

echo "'$LATEST_URL'"

mkdir -p temp
cd temp

zipfile=Onion-latest.zip

if [[ ! -f "$zipfile" ]]
then
    echo "-- downloading latest release"
    wget -O $zipfile "$LATEST_URL" -q --show-progress
fi

echo "-- extracting latest release"
mkdir -p extracted
unzip_progress $zipfile extracted

root=extracted

if [[ ! -d extracted/miyoo ]]
then
    for dir in extracted/*; do
        if [[ -d "$dir" ]]
        then
            root=$dir
            break
        fi
    done
fi

if [[ -d "$root/.tmp_update/.data" ]]
then
    # old build system
    mv "$root/.tmp_update/.data/Core" ./old_build
    mv ./old_build/Saves/.tmp_update ./old_build/.tmp_update
else
    # new build system
    echo "-- extracting latest build"
    unzip_progress "$root/miyoo/app/.installer/onion.pak" ./old_build
fi

rm -rf extracted

# TODO: Diff build and old_build to make patch...
