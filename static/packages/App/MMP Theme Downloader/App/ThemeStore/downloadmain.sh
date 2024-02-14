#!/bin/sh
cd $(dirname "$0")
pwd
mkdir /mnt/SDCARD/Themes #just in case it isnt there
#sleep 5
#echo "Running thumbnail script in case there are already previews"
sleep 5
#./thumbnails_generator.sh
echo "Launching downloadmain.sh (preview download script)"
echo "Press X and then menu to skip download"
echo "Do not cancel on first launch"
sleep 5
mkdir logos
mkdir thumbnails
#cd /mnt/SDCARD/App/MiyooThemeDownloaderApp/ && rm -f *.png
#cd /mnt/SDCARD/App/MiyooThemeDownloaderApp/ && rm -f *.zip
#!/bin/bash
#cd -
content=$(wget -qO- https://raw.githubusercontent.com/OnionUI/Themes/main/README.md)





#echo "$content" | grep -o 'icons/[^"]*gba\.png' | sed 's|^|https://raw.githubusercontent.com/OnionUI/Themes/main/|' > preview_urls.txt

#exit
#echo "$content" | grep -o 'themes/[^"]*gba\.png' | sed 's|^|https://raw.githubusercontent.com/OnionUI/Themes/main/|' > preview_urls.txt

#exit


echo "$content" | grep -o 'http[s]*://[^"]*preview\.png' | sort | uniq > preview_urls.txt
echo "$content" | grep -o 'icons/[^"]*gba\.png' | sed 's|^|https://raw.githubusercontent.com/OnionUI/Themes/main/|' >> preview_urls.txt
#echo "$content" | grep -o 'themes/[^"]*gba\.png' | sed 's|^|https://raw.githubusercontent.com/OnionUI/Themes/main/|' >> preview_urls.txt
#noecho "$content" | grep -o 'http[s]*://[^"]*\.zip?raw=true' | sort | uniq > zip_urls.txt
echo "$content" | grep -o 'http[s]*://[^"]*\.zip' | sort | uniq > zip_urls.txt
echo "$content" | grep -o 'release/[^"]*.zip?raw=true' | sed 's|^|https://raw.githubusercontent.com/OnionUI/Themes/main/|' >> zip_urls.txt
#exit
echo "Downloading preview images..."
#nocat preview_urls.txt | xargs -n 1 wget -q

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
echo Making thumbnails for new previews
sleep 5
# running thumbnails_generator for AdvanceMenu
./thumbnails_generator.sh
#sh launch.sh
#exit


