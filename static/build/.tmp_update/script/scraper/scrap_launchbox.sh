#!/bin/sh
#echo $0 $*    # for debugging


# Function to execute the SQL query and fetch the result
execute_sql_query() {
  local query="$1"
  sqlite3 "/mnt/SDCARD/.tmp_update/script/scraper/launchbox_database/$platform.db" "$query" | head -n 1
}

# Function to retrieve the URL of the media box
get_url_media_box() {
  local romName="$1"
  
  
  
		# regionsDB="/mnt/SDCARD/.tmp_update/script/scraper/screenscraper_database/regions.db"
		# RegionOrder=$(sqlite3 $regionsDB "SELECT lb_tree FROM regions WHERE lb_nom_en = '$SelectedRegion';")
	if ! echo "$romName" | grep -q "and"; then
	  local query="SELECT Images.FileName 
				   FROM Games JOIN Images ON Games.DatabaseID = Images.DatabaseID 
				   WHERE Games.Name LIKE '%$romName' 
				   AND Images.Type = '${MediaType}'
				   ORDER BY CASE 
							  WHEN Region = '${Region1}' THEN 1
							  WHEN Region = '${Region2}' THEN 2
							  WHEN Region = '${Region3}' THEN 3
							  WHEN Region = '${Region4}' THEN 4
							  WHEN Region = '${Region5}' THEN 5
							  WHEN Region = '${Region6}' THEN 6
							  WHEN Region = '${Region7}' THEN 7
							  WHEN Region = '${Region8}' THEN 8
							  ELSE 9
							END 
				  ;"
	else   # if the rom name contains "and" then we do a more complete search :
	  romNameTrimmed_Ampersand=${romNameTrimmed//and/&}
	  romNameTrimmed_WithoutAnd=${romNameTrimmed//and/%}
	  local query="SELECT Images.FileName 
				   FROM Games JOIN Images ON Games.DatabaseID = Images.DatabaseID 
				   WHERE (Games.Name LIKE '%$romName' OR Games.Name LIKE '%$romNameTrimmed_Ampersand' OR Games.Name LIKE '%$romNameTrimmed_WithoutAnd')
				   AND Images.Type = '${MediaType}'
				   ORDER BY CASE 
							  WHEN Region = '${Region1}' THEN 1
							  WHEN Region = '${Region2}' THEN 2
							  WHEN Region = '${Region3}' THEN 3
							  WHEN Region = '${Region4}' THEN 4
							  WHEN Region = '${Region5}' THEN 5
							  WHEN Region = '${Region6}' THEN 6
							  WHEN Region = '${Region7}' THEN 7
							  WHEN Region = '${Region8}' THEN 8
							  ELSE 9
							END 
				  ;"
	fi

    
			  
  urlMediaBox=$(execute_sql_query "$query")
  
  unset $romNameTrimmed_Ampersand
  unset $romNameTrimmed_WithoutAnd
}


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

get_launchbox_alias() {
  # find the corresponding platform for launchbox scraping
  case $1 in
    ADVMAME)              platform="Arcade";;
    AMIGA)                platform="Commodore Amiga";;
    AMIGACD)              platform="Commodore Amiga CD32";;
    ARCADE)               platform="Arcade";;
    # ARDUBOY)            platform="";;
    ATARI)                platform="Atari 2600";;
    ATARIST)              platform="Atari ST";;
    # CHAI)               platform="";;
    COLECO)               platform="ColecoVision";;
    COMMODORE)            platform="Commodore 64";;
    CPC)                  platform="Amstrad CPC";;
    CPS1)                 platform="Arcade";;
    CPS2)                 platform="Arcade";;
    CPS3)                 platform="Arcade";;
    DAPHNE)               platform="Arcade";;
    DOS)                  platform="MS-DOS";;
    # EASYRPG)            platform="";;
    # EBK)                platform="";;
    EIGHTHUNDRED)         platform="Atari 800";;
    FAIRCHILD)            platform="Fairchild Channel F";;
    FBA2012)              platform="Arcade";;
    FBALPHA)              platform="Arcade";;
    FBNEO)                platform="Arcade";;
    FC)                   platform="Nintendo Entertainment System";;
    FDS)                  platform="Nintendo Famicom Disk System";;
    FIFTYTWOHUNDRED)      platform="Atari 5200";;
    GB)                   platform="Nintendo Game Boy";;
    GBA)                  platform="Nintendo Game Boy Advance";;
    GBC)                  platform="Nintendo Game Boy Color";;
    GG)                   platform="Sega Game Gear";;
    # GME)                platform="";;
    GW)                   platform="Nintendo Game & Watch";;
    INTELLIVISION)        platform="Mattel Intellivision";;
    JAGUAR)               platform="Atari Jaguar";;
    # JAVA)               platform="";;
    # LUTRO)              platform="";;
    LYNX)                 platform="Atari Lynx";;
    MAME2000)             platform="Arcade";;
    MAME2003)             platform="Arcade";;
    MBA)                  platform="Arcade";;
    MD)                   platform="Sega Genesis";;
    MDHACKS)              platform="Sega Genesis";;
    MEGADUCK)             platform="Mega Duck";;
    # MICROW8)            platform="";;
    MS)                   platform="Sega Master System";;
    MSX)                  platform="Microsoft MSX";;
    NDS)                  platform="Nintendo DS";;
    NEOCD)                platform="SNK Neo Geo CD";;
    NEOGEO)               platform="SNK Neo Geo AES";;
    NGP)                  platform="SNK Neo Geo Pocket";;
    ODYSSEY)              platform="Magnavox Odyssey";;
    OPENBOR)              platform="OpenBOR";;
    # PALM)               platform="";;
    PANASONIC)            platform="3DO Interactive Multiplayer";;
    PCE)                  platform="NEC TurboGrafx-16";;
    PCECD)                platform="NEC TurboGrafx-CD";;
    PCEIGHTYEIGHT)        platform="NEC PC-8801";;
    PCFX)                 platform="NEC PC-FX";;
    PCNINETYEIGHT)        platform="NEC PC-9801";;
    # PICO)               platform="";;
    POKE)                 platform="Nintendo Pokemon Mini";;
    # PORTS)              platform="";;
    PS)                   platform="Sony Playstation";;
    SATELLAVIEW)          platform="Nintendo Satellaview";;
    SCUMMVM)              platform="ScummVM";;
    SEGACD)               platform="Sega CD";;
    SEGASGONE)            platform="Sega Model 1";;
    SEVENTYEIGHTHUNDRED)  platform="Atari 7800";;
    SFC)                  platform="Super Nintendo Entertainment System";;
    SGB)                  platform="Nintendo Game Boy Color";;
    SGFX)                 platform="PC Engine SuperGrafx";;
    # SUFAMI)             platform="";;
    SUPERVISION)          platform="Watara Supervision";;
    THIRTYTWOX)           platform="Sega 32X";;
    # THOMSON)            platform="";;
    # TI83)               platform="";;
    # TIC)                platform="";;
    # UZEBOX)             platform="";;
    VB)                   platform="Nintendo Virtual Boy";;
    VECTREX)              platform="GCE Vectrex";;
    VIC20)                platform="Commodore VIC-20";;
    VIDEOPAC)             platform="Philips Videopac+";;
    # VMU)                platform="dreamcast";;
    WS)                   platform="WonderSwan";;
    X68000)               platform="Sharp X68000";;
    XONE)                 platform="Sharp X1";;
    ZXEIGHTYONE)          platform="Sinclair ZX-81";;
    ZXS)                  platform="Sinclair ZX Spectrum";;
    zxspectrum)           platform="Sinclair ZX Spectrum";;
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



ScraperConfigFile=/mnt/SDCARD/.tmp_update/config/scraper.json
config=$(cat $ScraperConfigFile)
MediaType=$(echo "$config" | jq -r '.LaunchboxMediaType')
SelectedRegion=$(echo "$config" | jq -r '.LaunchboxRegion')
if [ -z "$MediaType" ]; then
	ssMediaType=$(echo "$config" | jq -r '.ScreenscraperMediaType')
    echo -e " The currently selected media ($ssMediaType)\n is not compatible with Launchbox scraper.\n\n\n\n\n\n\n\n\n\n\n\n Exiting."
	sleep 5
	exit
fi
echo "Media Type: $MediaType"
echo -e "Current Region: $SelectedRegion\n\n"
echo -e "Scraping $CurrentSystem...\n"


regionsDB="/mnt/SDCARD/.tmp_update/script/scraper/screenscraper_database/regions.db"
RegionOrder=$(sqlite3 $regionsDB "SELECT lb_tree FROM regions WHERE lb_nom_en = '$SelectedRegion';")
mkdir -p /mnt/SDCARD/Roms/$CurrentSystem/Imgs > /dev/null
get_launchbox_alias $CurrentSystem
# we split the RegionOrder in each region variable (do not indent)
IFS=';' read -r Region1 Region2 Region3 Region4 Region5 Region6 Region7 Region8 <<EOF
$RegionOrder
EOF
 
# =================
#this is a trick to manage spaces from find command, do not indent or modify
IFS='
'
set -f
# =================

if ! [ -z "$CurrentRom" ]; then
    romfilter="-name \"*$CurrentRom*\""
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
    if echo "$romNameTrimmed" | grep -q ", The"; then
        romNameTrimmed="${romNameTrimmed/, The/}"
        romNameTrimmed="The $romNameTrimmed"
    fi

    romNameTrimmed="${romNameTrimmed//","/}"
     # For debugging
     # echo romNameNoExtension= $romNameNoExtension
     # echo romNameTrimmed= $romNameTrimmed
	
	romNameTrimmed=${romNameTrimmed// /%}
	romNameTrimmed=${romNameTrimmed//\'/%}
	
	 # echo romNameTrimmed percent= $romNameTrimmed

	if [ -f "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png" ]; then
		echo -e "${YELLOW}already Scraped !${NONE}"
		let Scrap_notrequired++;
	else
        	


	urlMediaBox=""
	get_url_media_box "$romNameTrimmed"
	
	
###### same searchs but  more permissive search : romname*  , could find some false positive, if you have "Football" it could find "Football Pro"  ######

	if [ -z "$urlMediaBox" ]; then  
		get_url_media_box "${romNameTrimmed}%"
	fi
			

#########################################################
		# TO DO : manage other media types from options : https://docs.google.com/spreadsheets/d/1jFWhlt4MPcPGox45OCBAQVUrCOviDbh5P0DgoOlQ1vI/edit?usp=sharing
			# Banner , # Box - 3D , # Box - Front , # Screenshot - Gameplay , # Screenshot - Game Title , # Clear Logo
			
#########################################################
			# TO DO :  Manage regions priority : 
				# Europe , # Spain , # North America , # World , # Canada , # Japan , # France , # Germany , # Australia , # China , # United States , # United Kingdom ,
				# Russia , # Oceania , # Brazil , # Italy , # Korea , # The Netherlands , # Sweden , # Asia , # South America , # Greece , # Finland , # Norway , # Hong Kong
				
#########################################################

        ## TODO : The SQL search request could be improved.

#########################################################        


		if ! [ -z "$urlMediaBox" ]; then
			
            mediaextension="${urlMediaBox##*.}"
            # echo "wget --no-check-certificate "http://images.launchbox-app.com/${urlMediaBox}" -P \"/mnt/SDCARD/Roms/$CurrentSystem/Imgs\" -O \"$romNameNoExtension.png\""      # for debugging
             wget -q --no-check-certificate "http://images.launchbox-app.com/${urlMediaBox}" -P "/mnt/SDCARD/Roms/$CurrentSystem/Imgs" -O "$romNameNoExtension.$mediaextension"
             
			 if [ -f "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension" ]; then
				 if ! [ "$mediaextension" = "png" ]; then   # if the image is a jpg we convert it thanks to Eggs tool or ImageMagick
					filename=$(basename -- "$urlMediaBox")
					#magick "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension" -resize 250x360 "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
					jpgconverstion=$(jpg2png "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension")
					if [ $? -eq 0 ]; then  # we manage jpg2png crash on big images due to GFX  memory limit
						jpgconverstion=$(echo "$jpgconverstion" | awk -F "[ :]" '{printf("w:%d h:%d -> w:%d h:%d\n", $2, $4, $6, $8)}')
					else
						jpg2png_stb "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.$mediaextension" "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png" 250 360
					fi
					
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
		unset urlMediaBox
							
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