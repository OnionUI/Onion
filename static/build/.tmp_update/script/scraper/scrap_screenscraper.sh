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
Overwrite=$3

Screenscraper_information () {
clear 

cat << 	EOF

===================================
      = Screenscraper.fr =
===================================

Maximum number of requests per day:

-----------------------------------

Non-registered : 10.000 requests per day

level 0 to 19  : 20.000 requests per day
level 19 to 29 : 50.000 requests per day
level > 29     : 100.000 requests per day

1 or 2€  donation : 50.000  requests per day
5 or 10€ donation : 100.000 requests per day
moderator/Admin     : 200.000 requests per day

-----------------------------------





EOF

read -n 1 -s -r -p "Press A to continue"
clear 

cat << 	EOF

===================================
      = Screenscraper.fr =
===================================

Additional information :

-----------------------------------

Increase your level requires to participate in the
evolution of the Screenscraper database and media. 

Quota counters are reset every day at midnight 
(French time - UTC+2) 

Download speed also increase with participation 
to database or with donations.

All is detailed at : 
https://www.screenscraper.fr/faq.php

-----------------------------------



EOF

read -n 1 -s -r -p "Press A to continue"
clear
}


get_sc_id(){
	#SS ID systems for screenscraper scraping
	case $1 in
		ADVMAME)
			 ssID="75";;	#Mame
		AMIGA)
			 ssID="64";;	# ""Commodore : Amiga"
		AMIGACD)
			 ssID="134";;	# "Commodore : Amiga CD"
		ARCADE)   #Mame
			 ssID="75";;	# "Mame"
		ARDUBOY)
			 ssID="263";;	# "Arduboy"
		ATARI)   #Atari 2600
			 ssID="26";;	# "Atari : Atari 2600"
		ATARIST)
			 ssID="42";;	# "Atari : Atari ST"
		# CHAI) # chailove
			 # ssID="";;
		COLECO)
			 ssID="183";;	# "Coleco"
		COMMODORE)     #  Commodore 64/VIC-20/PET
			 ssID="66";;	# "Commodore : Commodore 64"
		CPC)   #Amstrad CPC
			 ssID="65";;	# "Amstrad : CPC"
		CPS1)
			 ssID="6";;	# "Capcom : Capcom Play System"
		CPS2)
			 ssID="7";;	# "Capcom : Capcom Play System 2"
		CPS3)
			 ssID="8";;	# "Capcom : Capcom Play System 3"
		DAPHNE)
			 ssID="49";;	# "Daphne"
		DOS)
			 ssID="135";;
		EASYRPG)
			 ssID="231";;
		EBK)
			 ssID="93";;
		EIGHTHUNDRED)  # Atari 800 / Atari 8bit
			 ssID="43";;
		FAIRCHILD)  # Fairchild Channel F
			 ssID="80";;
		FBA2012)
			 ssID="75";;
		FBALPHA)
			 ssID="75";;
		FBNEO)
			 ssID="";;
		FC)  # NES
			 ssID="3";;
		FDS)   # Famicom Disk System
			 ssID="106";;
		FIFTYTWOHUNDRED)   # Atari 5200
			 ssID="40";;
		GB)
			 ssID="9";;
		GBA)
			 ssID="12";;
		GBC)
			 ssID="10";;
		GG)
			 ssID="21";;  # "Sega - Game Gear"
		GW)   #  Nintendo - Game & Watch
			 ssID="52";;
		INTELLIVISION)
			 ssID="115";;
		JAGUAR)
			 ssID="27";;
		LUTRO)
			 ssID="206";;
		LYNX)
			 ssID="28";;
		MAME2000)
			 ssID="75";;
		MAME2003)
			 ssID="75";;
		MBA)
			 ssID="75";;
		MD)   # "Sega - Mega Drive - Genesis"
			 ssID="1";;
		MDHACKS)
			 ssID="1";;
		MEGADUCK)
			 ssID="90";;
		# MICROW8)
			 # ssID="";;
		MS)
			 ssID="2";;
		MSX)
			 ssID="113";;
		NEOCD)
			 ssID="70";;
		NEOGEO)
			 ssID="142";;
		NGP)
			 ssID="25";;
		ODYSSEY)  # Videopac / Magnavox Odyssey 2
			 ssID="104";;
		OPENBOR)
			 ssID="214";;
		PALM)
			 ssID="219";;
		PANASONIC)   #3DO
			 ssID="29";;
		PCE)   # NEC TurboGrafx-16 / PC Engine
			 ssID="31";;
		PCECD)
			 ssID="114";;
		PCEIGHTYEIGHT)   # NEC - PC-8000 & PC-8800 series / NEC PC-8801
			 ssID="221";;
		PCFX)   # NEC - PC-FX 
			 ssID="72";;
		PCNINETYEIGHT)  # NEC - PC-98 / NEC PC-9801
			 ssID="208";;
		PICO)
			 ssID="234";;
		# POKE)
			 # ssID="";;
		PORTS)   # "Microsoft : PC Win9X"
			 ssID="137";;
		PS)
			 ssID="57";;
		SATELLAVIEW)
			 ssID="107";;
		SCUMMVM)
			 ssID="123";;
		SEGACD)
			 ssID="20";;
		SEGASGONE)   # Sega SG-1000
			 ssID="109";;
		SEVENTYEIGHTHUNDRED)  # Atari 7800
			 ssID="41";;
		SFC)
			 ssID="4";;
		SGB)
			 ssID="127";;
		SGFX)   #  NEC - PC Engine SuperGrafx
			 ssID="105";;
		SUFAMI)
			 ssID="108";;
		SUPERVISION)
			 ssID="207";;
		THIRTYTWOX)   # Sega - 32X 
			 ssID="19";;
		THOMSON)
			 ssID="141";;
		# TI83)
			 # ssID="";;
		TIC)   # TIC-80
			 ssID="222";;
		UZEBOX)
			 ssID="216";;
		VB)
			 ssID="11";;
		VECTREX)
			 ssID="102";;
		VIC20) # Commodore : Vic-20
			 ssID="73";;
		VIDEOPAC)
			 ssID="104";;
		VMU)   # dreamcast (useless)
			 ssID="23";;
		WS)  # Bandai WonderSwan & Color
			 ssID="45";;
		X68000)
			 ssID="79";;
		XONE)   # Sharp X1
			 ssID="220";;
		ZXEIGHTYONE) # Sinclair - ZX-81
			 ssID="77";;
		ZXS)  # Sinclair ZX Spectrum
			 ssID="76";;
		*)
			 echo -n "unknown"
		;;
	esac
}






saveMetadata=false
clear
echo -e "\n*****************************************************"
echo -e "******************* SCREENSCRAPER *******************"
echo -e "*****************************************************\n\n"

echo -e "Scraping $CurrentSystem...\n"

#We check for existing credentials

ScraperConfigFile=/mnt/SDCARD/.tmp_update/config/scraper.json
if [ -f "$ScraperConfigFile" ]; then

    config=$(cat $ScraperConfigFile)
	
    userSS=$(echo "$config" | jq -r '.screenscraper_username')
    passSS=$(echo "$config" | jq -r '.screenscraper_password')
    ScrapeInBackground=$(echo "$config" | jq -r '.ScrapeInBackground')
	MediaType=$(echo "$config" | jq -r '.MediaType')

	u=$(echo "U2FsdGVkX18PKpoEvELyE+5xionDX8iRxAIxJj4FN1U=" | openssl enc -aes-256-cbc -d -a -pbkdf2 -iter 10000 -salt -pass pass:"3x0tVD3jZvElZWRt3V67QQ==")
	p=$(echo "U2FsdGVkX1/ydn2FWrwYcFVc5gVYgc5kVaJ5jDOeOKE=" |openssl enc -aes-256-cbc -d -a -pbkdf2 -iter 10000 -salt -pass pass:"RuA29ch3zVoodAItmvKKmZ+4Au+5owgvV/ztqRu4NjI=")

    if [ "$userSS" = "null" ] || [ "$passSS" = "null" ] || [ "$userSS" = "" ] || [ "$passSS" = "" ]; then
        userStored=false
    else
        userStored=true
        echo "screenscraper username: $userSS"
        echo -e "screenscraper password: xxxx\n\n"
    fi
fi



# TODO : improve or remove this part (now in options)
if [ "$userStored" = "false" ] && ! [ "$ScrapeInBackground" = "true" ]; then
    while true; do
	    Mychoice=$( echo -e "No\nYes\nScreenscraper information" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "Add your screenscraper account ?" -b "Press A to validate your choice.")
        
        if [ "$Mychoice" = "Yes" ]; then
            clear
            echo -e "Press X to display the keyboard and \nenter your screenscraper username\n\n"
            read -p "username : " userSS
            clear
            
            echo -e "Press X to display the keyboard and \nenter your screenscraper password\n\n"
            read -p "password : " passSS
            clear
            
            ScraperConfigFile=/mnt/SDCARD/.tmp_update/config/scraper.json
            config=$(cat $ScraperConfigFile)
            config=$(echo "$config" | jq --arg user "$userSS" --arg pass "$passSS" '.screenscraper_username = $user | .screenscraper_password = $pass')
            echo "$config" > $ScraperConfigFile

            break
            
        elif [ "$Mychoice" = "Screenscraper information" ]; then
            clear
			Screenscraper_information
        else
            clear
			break
        fi

    done
fi
		
clear
echo -e "\n*****************************************************"
echo -e "******************* SCREENSCRAPER *******************"
echo -e "*****************************************************\n\n"

echo -e "Scraping $CurrentSystem...\n"

####################################################################################################################################
 #ls /mnt/SDCARD/Roms/$CurrentSystem
 get_sc_id $CurrentSystem

 #ls /mnt/SDCARD/Roms/$CurrentSystem
 mkdir -p /mnt/SDCARD/Roms/$CurrentSystem/Imgs > /dev/null

# =================
#this is a trick to manage spaces from find command, do not indent or modify
IFS='
'
set -f
# =================


#Roms loop


#if ! [ -z "$CurrentRom" ]; then
#    romfilter="-name  '*$CurrentRom*'"
#fi

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
    

for file in $(eval "find /mnt/SDCARD/Roms/$CurrentSystem -maxdepth 2 -type f \
	! -name '.*' ! -name '*.xml' ! -name '*.miyoocmd' ! -name '*.cfg' ! -name '*.db' \
	! -path '*/Imgs/*' ! -path '*/.game_config/*' $romfilter"); do
	
    echo "-------------------------------------------------"
    let romcount++;
    
    # Cleaning up names
    romName=$(basename "$file")
    romNameNoExtension=${romName%.*}	
    
    
    romNameTrimmed="${romNameNoExtension/".nkit"/}"
    romNameTrimmed="${romNameTrimmed//"!"/}"
    romNameTrimmed="${romNameTrimmed//"&"/}"
    romNameTrimmed="${romNameTrimmed/"Disc "/}"
    romNameTrimmed="${romNameTrimmed/"Rev "/}"
    romNameTrimmed="$(echo "$romNameTrimmed" | sed -e 's/ ([^()]*)//g' -e 's/ [[A-z0-9!+]*]//g' -e 's/([^()]*)//g' -e 's/[[A-z0-9!+]*]//g')"
    romNameTrimmed="${romNameTrimmed//" - "/"%20"}"
    romNameTrimmed="${romNameTrimmed/"-"/"%20"}"
    romNameTrimmed="${romNameTrimmed//" "/"%20"}"
    
    
    echo $romNameNoExtension
    #echo $romNameTrimmed # for debugging


	if [ -f "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png" ] && [ "$Overwrite" != "1" ]; then
		echo -e "${YELLOW}already Scraped !${NONE}"
		let Scrap_notrequired++;
	
	else
        url="https://www.screenscraper.fr/api2/jeuInfos.php?devid=${u#???}&devpassword=${p%??}&softname=onion&output=json&ssid=${userSS}&sspassword=${passSS}&crc=&systemeid=${ssID}&romtype=rom&romnom=${romNameTrimmed}.zip"
        # TODO : search by CRC/MD5/shal or gameid
        # TODO : managing multithread for users who have it.
        
        #ID Game
        
        echo Searching on screenscraper ...
        api_result=$(curl -k -s "$url") 
		
        # Don't check art if screenscraper is closed
        if echo $api_result | grep -q "API closed"; then
        	echo -e "The Screenscraper API is currently down, please try again later."
        	echo -e "Press the ${RED}A button${NONE} to finish"
        	let Scrap_Fail++;
        	break;
        fi

        # Don't check art if max threads for leechers is used
        if echo $api_result | grep -q "The maximum threads"; then
        	echo -e "The Screenscraper API is too busy for non-users. please try again later (or register)."
        	echo -e "Press the ${RED}A button${NONE} to finish"
        	let Scrap_Fail++;
        	break;
        fi

        # Don't check art after a failed curl request
        if [[ "$api_result" == "" ]]; then
            echo -e "${RED}Request failed${NONE}"
        	# echo -e "Request failed to send for $romNameNoExtensionTrimmed, ${YELLOW}skipping${NONE}"    # for debugging
        	echo "Request failed for $romNameNoExtensionTrimmed" >> /mnt/SDCARD/.tmp_update/logs/scrap.log
        	let Scrap_Fail++;
        	continue;
        fi
        
        # Don't check art if screenscraper can't find a match
        if echo $api_result | grep -q "^Erreur"; then
        	#echo -e "Couldn't find a match for $romNameNoExtensionTrimmed, ${YELLOW}skipping${NONE}"   € for debugging
        	echo -e "${RED}No match found${NONE}"
        	echo "Couldn't find a match for $romNameNoExtensionTrimmed" >> /mnt/SDCARD/.tmp_update/logs/scrap.log
        	let Scrap_Fail++;
        	continue;
        fi
    	
    
        gameIDSS=$(echo $api_result | jq -r '.response.jeu.id')
    	echo "gameID = $gameIDSS"
    	
    	# Don't check art if we didn't get screenscraper game ID
        if ! [ "$gameIDSS" -eq "$gameIDSS" ] 2> /dev/null; then
            echo -e "${RED}Failed to get game ID${NONE}"
            let Scrap_Fail++;
            continue;
        fi
        
        # Here we choose the kind of media that we want :
            # sstitle	        Screenshot of Title Screen	(recommended)
            # ss	            Screenshot	(recommended)
            # fanart	        Fan Art	
            # screenmarquee	    Screen Marquee	
            # steamgrid	        HD Logos	
            # wheel	            Wheel	
            # wheel-hd	        HD Logos	
            # box-2D	        Box Art	(default) (recommended)
            # box-2D-side   	Box: Side	
            # box-2D-back   	Box: Back	
            # box-texture   	Box: Texture	
            # support-texture	Stand: Texture	
            # box-3D            Box 3D Art
			# mixrbv1			RecalBox Mix V1
			# mixrbv2			RecalBox Mix V2

		# TODO: Use the region defined in the rom's name to dictate which meida the user should receive, unless overridden
		# Get the URL of media in this order : world, us, usa, na, eu, uk, oceania, au, nz, jp and then the first entry available
		url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "wor") | .url' | head -n 1)
		if [ -z "$url" ]; then
			url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "us") | .url' | head -n 1)
			if [ -z "$url" ]; then
				url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "usa") | .url' | head -n 1)
				if [ -z "$url" ]; then
					url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "eu") | .url' | head -n 1)
					if [ -z "$url" ]; then
						url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "uk") | .url' | head -n 1)
						if [ -z "$url" ]; then
							url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "au") | .url' | head -n 1)
							if [ -z "$url" ]; then
								url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "nz") | .url' | head -n 1)
								if [ -z "$url" ]; then
									url=$(echo $api_result | jq --arg MediaType "$MediaType" '.response.jeu.medias[] | select(.type == $MediaType) | select(.region == "jp") | .url' | head -n 1)
								fi
							fi
						fi
					fi
				fi
			fi
		fi        
        # TODO : if default media not found search in other media types
        
        if [ -z "$url" ]; then
            echo -e "${YELLOW}Game match but no media found!${NONE}"
            let Scrap_Fail++;
            continue;
        fi
        
        # echo -e "Downloading Images for $romNameNoExtension \nScreenscraper ID : $gameIDSS \n url :$url\n\n"        # for debugging

        url=$(echo "$url" | sed 's/"$/\&maxwidth=250\&maxheight=360"/')
        urlcmd=$(echo "wget -q --no-check-certificate "$url" -P \"/mnt/SDCARD/Roms/$CurrentSystem/Imgs\" -O \"$romNameNoExtension.png\"")
        
        # directl download trigger an error
        #wget --no-check-certificate "$url" -P "/mnt/SDCARD/Roms/$CurrentSystem/Imgs" -O "$romNameNoExtension.png"
        #wget $urlcmd

        echo $urlcmd>/tmp/rundl.sh
        sh /tmp/rundl.sh

        echo -e "${GREEN}Scraped!${NONE}"
        let Scrap_Success++;
        
        
        # echo -e "\n\n ==$url== \n\n"
        #pngscale "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png" "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
    fi
   

		#####################################################################################################################################
    #   saveMetadata=false
    
    #   if [ $saveMetadata == true ]; then
    #       mkdir -p /mnt/SDCARD/Roms/$CurrentSystem/info > /dev/null
    #   
    #       if [ -f "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt" ]; then
    #           echo -e "${YELLOW}Metadata already Scraped !${NONE}"
    #       else
    #           genre_array=$( echo $api_result | jq -r '[foreach .response.jeu.genres[].noms[] as $item ([[],[]]; if $item.langue == "en" then $item.text else "" end)]'  )
    #           echo "" >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo game: $romNameNoExtension >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo file: $romName >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo developer: $( echo $api_result | jq -r  '.response.jeu.developpeur.text' ) >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo publisher: $( echo $api_result | jq -r  '.response.jeu.editeur.text'  ) >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo genre: $( echo $genre_array | jq '. - [""] | join(", ")' ) | sed 's/[\"]//g' >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo description: $( echo $api_result | jq -r  '.response.jeu.synopsis[0].text'  ) >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo release: $( echo $api_result | jq -r  '.response.jeu.dates[0].text'  ) >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo players: $( echo $api_result | jq -r  '.response.jeu.joueurs.text'  ) >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo rating: $( echo $api_result | jq -r  '.response.jeu.classifications[0].text'  ) >> "/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #           echo -e "Metadata saved to :\n/mnt/SDCARD/Roms/$CurrentSystem/info/$romNameNoExtension.txt"
    #       fi
    #   fi
		#####################################################################################################################################
				
#TODO : get manual	
				
				
				

done



echo -e "\n--------------------------"
echo "Total scanned roms   : $romcount"
echo "--------------------------"
echo "Successfully scraped : $Scrap_Success"
echo "Alread present       : $Scrap_notrequired"
echo "Failed or not found  : $Scrap_Fail"
echo -e "--------------------------\n"	 
sleep 2
echo "********   Screenscraper scraping finished   ********"

echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
