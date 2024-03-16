#!/bin/sh

#Will not flash anything despite filename
echo "=====================  $0   $1" 
cd ..

filename=$(basename -- "$1")
foldername="${filename%.*}"
echo "===================== displaying : ./logos/$foldername/image1.jpg"
./bin/jpgr "./logos/$foldername/image1.jpg"   # == Displays a rotated preview of the jpeg file


# Check firmware version
MIYOO_VERSION=`/etc/fw_printenv miyoo_version`
MIYOO_VERSION=${MIYOO_VERSION#miyoo_version=}
echo "Current firmware version : $MIYOO_VERSION"

if [ -f "/customer/app/axp_test" ]; then  # differenciate MM and MMP supported firmware
	MODEL="MMP"
	SUPPORTED_VERSION="202306282128"
else
	MODEL="MM"
	SUPPORTED_VERSION="202306111426"
fi



if [ $MIYOO_VERSION -gt $SUPPORTED_VERSION ]; then
	./bin/blank
	./bin/say "Firmware not supported."$'\n Versions further 20230326\nare not supported for now.\n\nPress a key to return to app menu.'
	./bin/confirm any
	exit 0
fi


# =========================================== Functions ===========================================
HexEdit() {
	filename=$1
	offset=$2
	value="$3"
	binary_value=$(printf "%b" "\\x$value")
	printf "$binary_value" | dd of="$filename" bs=1 seek="$offset" conv=notrunc
}

checkjpg() {
	JpgFilePath=$1
	Filename=`basename "$JpgFilePath"`
	echo
	./bin/checkjpg "$JpgFilePath"
	CHECK_JPG=$?
	if [ $CHECK_JPG -eq 0 ]; then
		echo "$Filename is a valid VGA JPG file"
	elif [ $CHECK_JPG -eq 1 ]; then
		./bin/blank
		./bin/say "$Filename is not a valid jpg file !"$'\n\n(Try to open it with your favorite image\neditor and \"save as\" -> jpg again)\n\nExiting without flash !'
		./bin/confirm any
		exit 0
	elif [ $CHECK_JPG -eq 2 ]; then
		./bin/blank
		./bin/say "$Filename "$'doesn\'t have \nthe right resolution !\n\nIt should be 640x480 (VGA)\n\nExiting without flash !'
		./bin/confirm any
		exit 0
	else
	  echo "Unknown Checkjpg error occurred"
	  exit 0
	fi
}
# =================================================================================================



# if we press "A" for flashing and the current image exists
if [ -f "./logos/$foldername/image1.jpg" ]; then
	DisplayInstructions=1
	./bin/say "Choose Theme or Icon?"$'\n'\("$foldername"\)$'\n\nA = Theme    B = Cancel  X = Icon\nSelect = Fullscreen'
	
	while :
	do
    	KeyPressed=$(./bin/getkey)
    	sleep 0.15  # Little debounce
    	echo "====== Key pressed : $KeyPressed"

    	if [ "$KeyPressed" = "A" ]; then
    		echo "=== Start ==="
    		#./bin/say OK
    		#./bin/blank
    		#exit
    		#./bin
                #$foldername is number
		./bin/say "Loading theme "$foldername"..."
		sleep 3
                sh ./downloadtheme.sh $foldername
                ./bin/blank
    		./bin/say "Done"
    		sleep 5
		cd /mnt/SDCARD/.tmp_update
		./bin/themeSwitcher
		exit 1
	elif [ "$KeyPressed" = "X" ]; then
		mkdir /mnt/SDCARD/Icons/
		./bin/say "Loading icon "$foldername"..."
		sleep 3
                sh ./downloadicon.sh $foldername
                ./bin/blank
		unzip "/mnt/SDCARD/Icons/icon.$foldername.zip" -d /mnt/SDCARD/Icons/
    		./bin/say 'Use Tweaks -> Appearance -> Icons packs'
    		sleep 5
		cd /mnt/SDCARD/Icons
		unzip icon.$num.zip
		cd /mnt/SDCARD/.tmp_update
		./bin/tweaks

		exit 1
    	elif [ "$KeyPressed" = "B" ] || [ "$KeyPressed" = "menu" ] ; then
    		./bin/blank
    		./bin/say "Canceling"
    		exit
		elif [ "$KeyPressed" = "select" ]; then    # == if select has been pressed we don't display the text instructions
			if [ "$DisplayInstructions" = "1" ]; then
				DisplayInstructions=0
				./bin/blank
	            ./bin/jpgr "./logos/$foldername/image1.jpg"   # == Displays a rotated preview of the jpeg file
			else
				DisplayInstructions=1
			#	./bin/blank
	            ./bin/jpgr "./logos/$foldername/image1.jpg"   # == Displays a rotated preview of the jpeg file
				./bin/say "Really want to flash ?"$'\n'\("$foldername"\)$'\n\nA = Confirm    B = Cancel\nSelect = Fullscreen'
			fi
   
    	   	   
    	fi
	done
else
	echo "./logos/$foldername/image1.jpg not found"
	./bin/blank
	./bin/say "$foldername/image1.jpg not found"$'\n\nExiting without flash !'
	./bin/confirm any
	exit 1
fi

