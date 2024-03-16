#!/bin/sh
LD_LIBRARY_PATH="./imagemagick/libs:$LD_LIBRARY_PATH"


echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

if [ -z "$1" ] ; then
    Force=0
else
	Force=$1
fi


# Removing obsolete thumbnails and mac metadata files
for file in ./thumbnails/.[!.]*.png ./thumbnails/*.png
do
    filename=$(basename "$file" .png)
    if [ ! -d "./logos/$filename" ] ; then
        echo removing "$file"
        rm "$file"
    fi
done

# Creating missing thumbnails
for d in ./logos/* ; do

	if [ -f "$d/image1.jpg" ]; then

		echo ============================= ${d##*/} =============================

		if [ ! -f "./thumbnails/${d##*/}.png" ] || [ "$Force" -eq 1 ] ; then
		        #might be the cause of the problem...
			# we create a png : rotated, half resolution, 140% of luminosity, color depth 8 bits
			# delete original just in case
			rm "./thumbnails/${d##*/}.png"
			echo "./imagemagick/magick \"$d/image1.jpg\" -rotate 180 -resize 50% -modulate 110 -depth 8 -define png:color-type=6 \"./thumbnails/${d##*/}.png\""
		       ./imagemagick/magick "$d/image1.jpg" -rotate 180 -resize 50% -modulate 110 -depth 8 -define png:color-type=6 "./thumbnails/${d##*/}.png"
		else
  		        echo "Image ${d##*/} already existed but running imagemagick anyway"
                        rm "./thumbnails/${d##*/}.png"                                                                                
  		       ./imagemagick/magick "$d/image1.jpg" -rotate 180 -resize 50% -modulate 110 -depth 8 -define png:color-type=6 "./thumbnails/${d##*/}.png"
		fi
	fi

done

