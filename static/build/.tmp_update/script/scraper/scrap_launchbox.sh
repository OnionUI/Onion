#!/bin/sh
#echo $0 $*    # for debugging

if [ -z "$1" ]
then
  echo -e "\nusage : scrap_screenscraper.sh emu_folder_name [rom_name]\nexample : scrap_screenscraper.sh SFC\n"
  exit
fi

sysdir=/mnt/SDCARD/.tmp_update
PATH="$sysdir/bin:$PATH"
LD_LIBRARY_PATH="/mnt/SDCARD/.tmp_update/lib:$sysdir/lib/parasyte:$LD_LIBRARY_PATH"

echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
#$sysdir/bin/freemma > /dev/null

NONE='\033[00m'
RED='\033[01;31m'
GREEN='\033[01;32m'
YELLOW='\033[01;33m'
PURPLE='\033[01;35m'
CYAN='\033[01;36m'
WHITE='\033[01;37m'
BOLD='\033[1m'
UNDERLINE='\033[4m'
BLINK='\x1b[5m'

romcount=0
Scrap_Success=0
Scrap_Fail=0
Scrap_notrequired=0

CurrentSystem=$1
CurrentRom="$2"
Overwrite="$3"

get_launchbox_alias(){
	#find the corresponding platform for launchbox scraping
	case $1 in				
		ADVMAME)
			 platform="mame";;
		AMIGA)
			 platform="amiga";;
		# AMIGACD)
			 # platform="";;
		ARCADE)
			 platform="mame";;
		# ARDUBOY)
			 # platform="";;
		ATARI)
			 platform="atari2600";;
		ATARIST)
			 platform="atarist";;
		# CHAI)
			 # platform="";;
		COLECO)
			 platform="colecovision";;
		# COMMODORE)
			 # platform="";;
		CPC)
			 platform="amstradcpc";;
		# CPS1)
			 # platform="";;
		# CPS2)
			 # platform="";;
		# CPS3)
			 # platform="";;
		# DAPHNE)
			 # platform="";;
		# DOS)
			 # platform="";;
		# EASYRPG)
			 # platform="";;
		# EBK)
			 # platform="";;
		# EIGHTHUNDRED)
			 # platform="";;
		# FAIRCHILD)
			 # platform="";;
		# FBA2012)
			 # platform="";;
		# FBALPHA)
			 # platform="";;
		# FBNEO)
			 # platform="";;
		FC)
			 platform="nes";;
		# FDS)
			 # platform="";;
		# FIFTYTWOHUNDRED)
			 # platform="";;
		GB)
			 platform="gb";;
		GBA)
			 platform="gba";;
		GBC)
			 platform="gbc";;
		GG)
			 platform="gamegear";;
		# GME)
			 # platform="";;
		GW)
			 platform="gameandwatch";;
		INTELLIVISION)
			 platform="intellivision";;
		# JAGUAR)
			 # platform="";;
		# JAVA)
			 # platform="";;
		# LUTRO)
			 # platform="";;
		LYNX)
			 platform="lynx";;
		MAME2000)
			 platform="mame";;
		MAME2003)
			 platform="mame";;
		MBA)
			 platform="mame";;
		MD)
			 platform="genesis";;
		MDHACKS)
			 platform="genesis";;
		# MEGADUCK)
			 # platform="";;
		# MICROW8)
			 # platform="";;
		MS)
			 platform="mastersystem";;
		MSX)
			 platform="MSX";;
		NEOCD)
			 platform="neogeocd";;
		NEOGEO)
			 platform="neogeo";;
		NGP)
			 platform="ngp";;
		# ODYSSEY)
			 # platform="";;
		# OPENBOR)
			 # platform="";;
		# PALM)
			 # platform="";;
		PANASONIC)
			 platform="3do";;
		PCE)
			 platform="pcengine";;
		# PCECD)
			 # platform="";;
		# PCEIGHTYEIGHT)
			 # platform="";;
		# PCFX)
			 # platform="";;
		# PCNINETYEIGHT)
			 # platform="";;
		# PICO)
			 # platform="";;
		# POKE)
			 # platform="";;
		# PORTS)
			 # platform="";;
		# PS)
			 # platform="";;
		# SATELLAVIEW)
			 # platform="";;
		SCUMMVM)
			 platform="scummvm";;
		SEGACD)
			 platform="segacd";;
		# SEGASGONE)
			 # platform="";;
		# SEVENTYEIGHTHUNDRED)
			 # platform="";;
		# SFC)
			 # platform="";;
		# SGB)
			 # platform="";;
		# SGFX)
			 # platform="";;
		# SUFAMI)
			 # platform="";;
		# SUPERVISION)
			 # platform="";;
		# THIRTYTWOX)
			 # platform="sega32x";;
		# THOMSON)
			 # platform="";;
		# TI83)
			 # platform="";;
		# TIC)
			 # platform="";;
		# UZEBOX)
			 # platform="";;
		VB)
			 platform="virtualboy";;
		VECTREX)
			 platform="vectrex";;
		# VIC20)
			 # platform="";;
		# VIDEOPAC)
			 # platform="";;
		VMU)
			 platform="dreamcast";;
		# WS)
			 # platform="";;
		# X68000)
			 # platform="";;
		# XONE)
			 # platform="";;
		# ZXEIGHTYONE)
			 # platform="";;
		# ZXS)
			# platform="";;
		zxspectrum)
			platform="zxspectrum";;
		*)
			echo "unknown system, exiting."
			exit
		;;
	esac
}


saveMetadata=false
clear
echo -e "\n*****************************************************"
echo -e "*******************   LAUNCHBOX   *******************"
echo -e "*****************************************************\n\n"

echo "Scraping $CurrentSystem..."
mkdir -p /mnt/SDCARD/Roms/$CurrentSystem/Imgs > /dev/null
get_launchbox_alias $CurrentSystem


#content=$(cat /mnt/SDCARD/.tmp_update/script/scraper/metadata.json)   # finally faster to parse file from SD card than from memory.
 
# =================
#this is a trick to manage spaces from find command, do not indent or modify
IFS='
'
set -f
# =================



if ! [ -z "$CurrentRom" ]; then
 #   CurrentRom_noapostrophe=${CurrentRom//\'/\\\'}    # replacing   '   by    \'
 #   romfilter="-name  '*$CurrentRom_noapostrophe*'"
 #   
 #   CurrentRom="Link's"
 #   romfilter="-name  '*$CurrentRom*'"
 #   
 #   romfilter="-name  '*Link's*'"
    romfilter="-name \"*$CurrentRom*\""
    #romfilter="-name  '*$CurrentRom*'"
    
fi


#Roms loop
for file in $(eval "find /mnt/SDCARD/Roms/$CurrentSystem -maxdepth 2 -type f \
	! -name '.*' ! -name '*.xml' ! -name '*.miyoocmd' ! -name '*.cfg' ! -name '*.db' \
	! -path '*/Imgs/*' ! -path '*/.game_config/*' $romfilter"); do

    echo "-------------------------------------------------"
    let romcount++;
    # Cleaning up names
    romName=$(basename "$file")
    romNameNoExtension=${romName%.*}	
    echo "$romNameNoExtension"
    
    romNameTrimmed="${romNameNoExtension/".nkit"/}"
    romNameTrimmed="${romNameTrimmed//"!"/}"
    romNameTrimmed="$(echo "$romNameTrimmed" | sed -e 's/&/and/g')"
    romNameTrimmed="${romNameTrimmed/"Disc "/}"
    romNameTrimmed="${romNameTrimmed/"Rev "/}"
    romNameTrimmed="$(echo "$romNameTrimmed" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
    romNameTrimmed="${romNameTrimmed//" - "/" "}"
    romNameTrimmed="${romNameTrimmed/"-"/" "}"

    
    # we put "The" at the beginning of the rom name
    if echo "$variable" | grep -q ", The"; then
        romNameTrimmed="${romNameTrimmed/, The/}"
        romNameTrimmed="The $romNameTrimmed"
    fi

    romNameTrimmed="${romNameTrimmed//","/}"
    
    
    # For debugging
    #echo romNameNoExtension= $romNameNoExtension
    #echo romNameTrimmed= $romNameTrimmed


	if [ -f "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png" ] && [ "$Overwrite" = "" ]; then
		echo -e "${YELLOW}already Scraped !${NONE}"
		let Scrap_notrequired++;
	else
        	
        #box2dfront=$(jq -r --arg games "$games" '.platform."3do".games[$games].medias.box2dfront' fichier.json)
		urlMediaBox=$( cat /mnt/SDCARD/.tmp_update/script/scraper/metadata.json | jq -r ".platform.\"$platform\".games.\"$romNameTrimmed\".medias.box2dfront" )    
		#urlMediaSs=   $( echo $content | jq -r  ".platform.$CurrentSystem.games.\"$romNameTrimmed\".medias.screenshot" )
    	#urlMediaWheel=$( echo $content | jq -r  ".platform.$CurrentSystem.games.\"$romNameTrimmed\".medias.wheel" )
		
        ## TODO : split database for each system
        ## TODO : if media not found search in other media types
        ## TODO : more permissive search
        
        #echo $content | jq -r  ".platform.GB.games."Super Mario Land".medias.box2dfront" 
			

		if ! [ "$urlMediaBox" = "null" ]; then
            mediaextension="${urlMediaBox##*.}"
            #echo "wget --no-check-certificate "$urlMediaBox" -P \"/mnt/SDCARD/Roms/$CurrentSystem/Imgs\" -O \"$romNameNoExtension.png\""      # for debugging
             wget -q --no-check-certificate "$urlMediaBox" -P "/mnt/SDCARD/Roms/$CurrentSystem/Imgs" -O "$romNameNoExtension.$mediaextension"
             
			 if [ -f "$romNameNoExtension.$mediaextension" ]; then
				 if ! [ "$mediaextension" = "png" ]; then
					filename=$(basename -- "$urlMediaBox")
					#magick "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension" -resize 250x360 "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
					jpgconverstion=$(jpg2png "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension")
					jpgconverstion=$(echo "$jpgconverstion" | awk -F "[ :]" '{printf("w:%d h:%d -> w:%d h:%d\n", $2, $4, $6, $8)}')
					echo "jpg to png :  $jpgconverstion"
					rm "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension"
				else
					pngScale "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension" "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
				 fi
				 
				 echo -e "${GREEN}Scraped!${NONE}"
				 let Scrap_Success++;
			else
				 echo -e "${RED}Download failed.${NONE}"
				 let Scrap_Fail++;
			fi
		else
		    echo -e "${RED}No match found${NONE}"
        	echo "Couldn't find a match for $romNameTrimmed" >> /mnt/SDCARD/.tmp_update/logs/scrap.log
		    #echo -e "Couldn't find a match for $romNameTrimmed, ${YELLOW}skipping${NONE}"    # for debugging
		    let Scrap_Fail++;
		fi
							
	fi		

done

#unset content
echo -e "\n--------------------------"
echo "Total scanned roms   : $romcount"
echo "--------------------------"
echo "Successfully scraped : $Scrap_Success"
echo "Alread present       : $Scrap_notrequired"
echo "Failed or not found  : $Scrap_Fail"
echo -e "--------------------------\n"
sleep 2
echo "**********   Launchbox scraping finished   **********"

echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor