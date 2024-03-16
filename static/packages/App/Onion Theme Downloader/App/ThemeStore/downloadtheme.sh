#!/bin/sh
#amt=$(grep -c '.' zip_urls.txt)
#echo $amt
#echo Num of preview?
#read num
cd $(dirname "$0")
num=$1
num=$((num + 0))
url=$(sed -n "${num}p" < zip_urls.txt)

#echo $num
#echo $irl
#Hardcoded download to theme
wget "$url" -O "/mnt/SDCARD/Themes/theme.$num.zip"
#unzip "/mnt/SDCARD/Icons/icon.$num.zip" -d /mnt/SDCARD/Icons/
./bin/themeSwitcher
