#!/bin/sh
. /mnt/SDCARD/App/EmuDeckCloudSync/functions.sh


#Uninstall
if [ -f /mnt/SDCARD/.tmp_update/startup/emudeck.sh ]; then
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
		-t "EmuDeck CloudSync" \
		-m "What do you want to do?" \
		"Force upload all my saves to the cloud" \
		"Force download all my saves from the cloud" \
		"Uninstall" \
		"Exit" \
	
	retcode=$?	
		
	if [ $retcode -eq 0 ]; then
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/uploading.json --persistent &
		emudeck_getSetDate
		timestamp=$(date +%s)
		echo $timestamp > $EmuDeckPath/.pending_upload
		while [ -f $EmuDeckPath/.pending_upload ]; do emudeck_cloud_upload; done
		touch /tmp/dismiss_info_panel
		exit
	fi
	
		
	if [ $retcode -eq 1 ]; then
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/downloading.json --persistent &
		emudeck_getSetDate
		timestamp=$(date +%s)
		echo $timestamp > $EmuDeckPath/.pending_download
		while [ -f $EmuDeckPath/.pending_download ]; do emudeck_cloud_download; done
		touch /tmp/dismiss_info_panel
		exit
	fi
	
	if [ $retcode -eq 2 ]; then
		rm -rf /mnt/SDCARD/rclone
		rm -rf /mnt/SDCARD/rclone.conf
		rm -rf /mnt/SDCARD/.tmp_update/startup/emudeck.sh
		
		emudeck_changeLine 'cloudEnabled' 'cloudEnabled=false' $EmuDeckPath/settings.sh
		emudeck_changeLine 'systemCloud' 'systemCloud=false' $EmuDeckPath/settings.sh
		emudeck_changeLine 'cloudProvider' 'cloudProvider=false' $EmuDeckPath/settings.sh

		exit
	fi
		
	if [ $retcode -eq 3 ]; then
		exit
	fi
	

fi


#Install
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "EmuDeck CloudSync" \
	-m "This App will configure OnionOS\n to sync your states\n with your cloud provider" \
	"Next" \
	"Exit"

retcode=$?		

if [ $retcode -eq 1 ]; then
	exit
fi

#EmuDeck or Miyoo?
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "EmuDeck CloudSync" \
	-m "Do you want to sync with your\n EmuDeck installation?" \
	"Yes" \
	"No, I don't have EmuDeck" \
	"Exit" \

retcode=$?		

if [ $retcode -eq 0 ]; then
	emudeck=true
fi
if [ $retcode -eq 1 ]; then
	emudeck=false
fi

if [ $retcode -eq 2 ]; then
	exit
fi

if [ $emudeck = "true" ]; then
	
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "EmuDeck CloudSync" \
	-m "EmuDeck uses different states \nand core configurations.\n We need to do some setup first." \
	"Next" \
	"Exit"
		
	retcode=$?	
		
	if [ $retcode -eq 0 ]; then
		echo "next"
	fi	
	if [ $retcode -eq 1 ]; then
		exit
	fi

	
	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "EmuDeck CloudSync" \
	-m "These are the cores we will change:\nSuper Nintendo: Snex9x\nSega Systems: Genesis Plus GX\nTo use old states saved using other cores\n you need set each game with the old core as default" \
	"Next" \
	"Exit"
	
	retcode=$?	
	
	if [ $retcode -eq 1 ]; then
		exit
	fi


	LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "EmuDeck CloudSync" \
	-m "OnionOS separates your states in subfolders.\nWe will change your configuration to use\nthe classic RetroArch states folder\nYou won't lost any saved games\nIf you want to go back,\n you'll need to do it manually" \
	"Install" \
	"Exit"
	
	retcode=$?	
	
	if [ $retcode -eq 1 ]; then
		exit
	fi
	systemCloud="Emudeck"
	
else
	systemCloud="OnionOS"
fi #if [ $emudeck = "true" ]


LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "EmuDeck CloudSync" \
	-m "Select your Cloud provider" \
	"DropBox" \
	"Google Drive" \
	"Box" \
	"NextCloud" \
	"pCloud" \

	retcode=$?		
	
	if [ $retcode -eq 0 ]; then
		provider="$systemCloud-DropBox"
	elif [ $retcode -eq 1 ]; then
		provider="$systemCloud-Gdrive"
	elif [ $retcode -eq 2 ]; then
		provider="$systemCloud-Box"
	elif [ $retcode -eq 3 ]; then
		provider="$systemCloud-NextCloud"
	elif [ $retcode -eq 4 ]; then
		provider="$systemCloud-pCloud"
	fi
	
emudeck_changeLine 'systemCloud' "systemCloud=\"${systemCloud}\"" "$EmuDeckPath/settings.sh"	
emudeck_changeLine 'cloudProvider' "cloudProvider=\"${provider}\"" "$EmuDeckPath/settings.sh"	
emudeck_changeLine 'cloudEnabled' 'cloudEnabled=true' $EmuDeckPath/settings.sh	

#Set proper date
emudeck_getSetDate

infoPanel -t "Installing" -m "Please wait..." --auto &


#We copy the startup script
cp ./emudeck.sh /mnt/SDCARD/.tmp_update/startup/emudeck.sh


if [ $emudeck = "true" ]; then
	#We change the states config
	RAConfig="/mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg"	
	cp $RAConfig "$RAConfig.bak"
	
	emudeck_changeLine 'sort_savefiles_by_content_enable = ' 'sort_savefiles_by_content_enable = "false"' "$RAConfig"
	emudeck_changeLine 'sort_savefiles_enable =' 'sort_savefiles_enable = "false"' "$RAConfig"
	emudeck_changeLine 'sort_savestates_by_content_enable=' 'sort_savestates_by_content_enable = "false"' "$RAConfig"
	emudeck_changeLine 'sort_savestates_enable= ' 'sort_savestates_enable = "false"' "$RAConfig"
	emudeck_changeLine 'sort_screenshots_by_content_enable =' 'sort_screenshots_by_content_enable = "false"' "$RAConfig"
	
	#We copy all the states to their parent folder
	states_folder="/mnt/SDCARD/Saves/CurrentProfile/states/"
	find "$states_folder" -type f -exec sh -c '
	 # No file prefix
	 filename=$(basename "$0")
	
	 # we create a copy, never delete
	 cp "$0" "$1/$filename"
	' {} "$states_folder" \;
	
	#We copy all the saves to their parent folder
	states_folder="/mnt/SDCARD/Saves/CurrentProfile/saves/"
	find "$states_folder" -type f -exec sh -c '
	 # No file prefix
	 filename=$(basename "$0")
	
	 # we create a copy, never delete
	 cp "$0" "$1/$filename"
	' {} "$states_folder" \;
	
	#We set the EmuDeck cores
	
	#SNES
	if ! [ -f /mnt/SDCARD/Emu/SFC/launch.sh.bak ]; then
		cp /mnt/SDCARD/Emu/SFC/launch.sh /mnt/SDCARD/Emu/SFC/launch.sh.bak
	fi	
	sed -i 's/mednafen_supafaust_libretro/snes9x_libretro/g'/mnt/SDCARD/Emu/SFC/launch.sh

	#Genesis
	if ! [ -f /mnt/SDCARD/Emu/MD/launch.sh.bak ]; then
		cp /mnt/SDCARD/Emu/MD/launch.sh /mnt/SDCARD/Emu/MD/launch.sh.bak
	fi	
	sed -i 's/picodrive_libretro/genesis_plus_gx_libretro/g'/mnt/SDCARD/Emu/MD/launch.sh
	
	#Master System
	if ! [ -f /mnt/SDCARD/Emu/MS/launch.sh.bak ]; then
		cp /mnt/SDCARD/Emu/MS/launch.sh /mnt/SDCARD/Emu/MS/launch.sh.bak
	fi	
	sed -i 's/picodrive_libretro/genesis_plus_gx_libretro/g'/mnt/SDCARD/Emu/MS/launch.sh

		
	#GameGear
	if ! [ -f /mnt/SDCARD/Emu/GG/launch.sh.bak ]; then
		cp /mnt/SDCARD/Emu/GG/launch.sh /mnt/SDCARD/Emu/GG/launch.sh.bak
	fi	
	sed -i 's/picodrive_libretro/genesis_plus_gx_libretro/g'/mnt/SDCARD/Emu/GG/launch.sh
	
	
	
	
	
	#Get Token
	# code=$(cat ./.code_temp)
	# rm -rf ./.code_temp
	# json=$(curl -s "https://patreon.emudeck.com/hastebin.php?code=$code")     
	# json_object=$(echo $json | jq .)
	
	# section=$(echo $json | jq .section)
	# token=$(echo $json | jq .token)
	
	# Cleanup
	# token=$(echo "$token" | sed "s/\"//g")
	# token=$(echo "$token" | sed "s/'/\"/g")            
	# section=$(echo "$section" | sed 's/[][]//g; s/"//g') 
	
	#iniFieldUpdate "/mnt/SDCARD/rclone.conf" "EmuDeck_DropBox" "token" "$token"    
	
fi


#We install rclone to the root so its easier for the user to copy their .conf
cp ./rclone /mnt/SDCARD/rclone

if [ $emudeck = "false" ]; then
	cp ./rclone.conf /mnt/SDCARD/rclone.conf
fi

touch /tmp/dismiss_info_panel

if [ $emudeck = "true" ]; then	
	message="Now you need to copy your \nEmulation/tools/rclone/rclone.conf file\nfrom EmuDeck to the root of this SDCard"
else
	message="Now you need to setup\nyour Cloud Provider credentials,\n check our Wiki to know how to set them up"
fi

LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "CloudSync Installed!" \
	-m "$message" \
	"I've copied my rclone.conf file"	
retcode=$?
		
	if [ $retcode -eq 0 ]; then
		echo "continue"
	fi
	
	
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "CloudSync First Setup" \
	-m "We will now do the first Sync.\nWhere are your latest saves stored?\n" \
	"In this Miyoo, upload them to the cloud now" \
	"In my other device, download everything"
retcode=$?
	
	if [ $retcode -eq 0 ]; then
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/uploading.json --persistent &
		emudeck_getSetDate
		timestamp=$(date +%s)
		echo $timestamp > $EmuDeckPath/.pending_upload
		while [ -f $EmuDeckPath/.pending_upload ]; do emudeck_cloud_upload; done
		touch /tmp/dismiss_info_panel
	fi
	
		
	if [ $retcode -eq 1 ]; then
		infoPanel --images-json /mnt/SDCARD/App/EmuDeckCloudSync/downloading.json --persistent &
		emudeck_getSetDate
		timestamp=$(date +%s)
		echo $timestamp > $EmuDeckPath/.pending_download
		while [ -f $EmuDeckPath/.pending_download ]; do emudeck_cloud_download; done
		touch /tmp/dismiss_info_panel
	fi
	
LD_PRELOAD=/mnt/SDCARD/miyoo/lib/libpadsp.so /mnt/SDCARD/.tmp_update/bin/prompt -r \
	-t "All Done!" \
	-m "Everytime you start a game we will\ndownload your states from the cloud\nand we will upload them as you exit the game.\n" \
	"Exit"
if [ $retcode -eq 0 ]; then
	exit
fi