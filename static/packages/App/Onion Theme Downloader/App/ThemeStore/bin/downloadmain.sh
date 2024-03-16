#!/bin/sh
cd $(dirname "$0")
pwd

sleep 5
#Has problem with upside down images
echo "Launching conv"
mkdir logos
#cd /mnt/SDCARD/App/MiyooThemeDownloaderApp/ && rm -f *.png
#cd /mnt/SDCARD/App/MiyooThemeDownloaderApp/ && rm -f *.zip
#!/bin/bash
#cd -
content=$(wget -qO- https://raw.githubusercontent.com/OnionUI/Themes/main/README.md)

echo "$content" | grep -o 'http[s]*://[^"]*preview\.png' | sort | uniq > preview_urls.txt
echo "$content" | grep -o 'http[s]*://[^"]*\.zip' | sort | uniq > zip_urls.txt

echo "Downloading preview images..."
#cat preview_urls.txt | xargs -n 1 wget -q

amt=$(grep -c '.' preview_urls.txt)
echo $amt
LD_LIBRARY_PATH="./imagemagick/libs:$LD_LIBRARY_PATH"
counter=1
while [ $counter -le $amt ]; do
    echo "Iteration $counter"
    a=$(head -n 1 preview_urls.txt)
    mkdir "./logos/$counter"
    wget "$a" -O "./logos/$counter/image1.jpg"
    ./imagemagick/magick ./logos/$counter/image1.jpg -rotate 180 ./logos/$counter/image1.jpg

    #sleep 1
    sed -i '1d' preview_urls.txt
    counter=$((counter + 1))
done
sh launch.sh
#exit


