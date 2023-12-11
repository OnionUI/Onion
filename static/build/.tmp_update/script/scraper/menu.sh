#!/bin/sh
#echo $0 $*    # for debugging

sysdir=/mnt/SDCARD/.tmp_update
PATH="$sysdir/bin:$PATH"
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/mnt/SDCARD/.tmp_update/lib:$sysdir/lib/parasyte"


romname=$(basename "$1")
CurrentSystem=$(echo "$1" | grep -o '/Roms/[^/]*' | cut -d'/' -f3)
romNameNoExtension=${romname%.*}
romimage="/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
ScraperConfigFile=/mnt/SDCARD/.tmp_update/config/scraper.json


# # for debugging
# echo "CurrentSystem : $CurrentSystem"
# echo "romname : $romname"
# echo "romimage : $romimage"
# echo "romNameNoExtension : $romNameNoExtension"
# echo "romimage : $romimage"
# read -n 1 -s -r -p "Press A to continue"



##########################################################################################

Menu_Config()
{
    Option1="Media preferences"
	Option2="Region selection"
	Option3="Scraping sources"
    Option4="Screenscraper: account settings"
    Option5="Toggle background scraping"
    Option6="Back to Main Menu"
    
    Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4\n$Option5\n$Option6" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "      --== CONFIGURATION MENU ==--" -b "Press A to validate your choice.")

    [ "$Mychoice" = "$Option1" ] && Menu_Config_MediaType
    [ "$Mychoice" = "$Option2" ] && Menu_RegionSelection
    [ "$Mychoice" = "$Option3" ] && Menu_Config_ScrapingSource
    [ "$Mychoice" = "$Option4" ] && Menu_Config_SSAccountSettings
    [ "$Mychoice" = "$Option5" ] && Menu_Config_BackgroundScraping
	[ "$Mychoice" = "$Option6" ] && Menu_Main
	
	sync
}


##########################################################################################

Screenscraper_accountState()
{

	clear
	if [ "$userSS" = "null" ] || [ "$passSS" = "null" ] || [ "$userSS" = "" ] || [ "$passSS" = "" ]; then
		echo -e "Login or Password is empty!\nCheck user IDs!\n\n\n\n\n\n\n\n"
		read -n 1 -s -r -p "Press A to continue"
		return
	fi
	
	echo "Retrieve account information..."
	# Appel de l'API avec curl et stockage de la réponse JSON dans une variable
	url="https://www.screenscraper.fr/api2/ssuserInfos.php?devid=xxx&devpassword=yyy&softname=zzz&output=json&ssid=$userSS&sspassword=$passSS"
	api_result=$(curl -k -s "$url")


	if echo "$api_result" | grep -q "^Erreur"; then
		echo -e "Authentification failed.\nCheck user IDs!\n\n\n\n\n\n\n\n"
		read -n 1 -s -r -p "Press A to continue"
		return
	fi

	# Extraction des informations du JSON
	id=$(echo "$api_result" | jq -r '.response.ssuser.id')
	level=$(echo "$api_result" | jq -r '.response.ssuser.niveau')
	contribution=$(echo "$api_result" | jq -r '.response.ssuser.contribution')
	approvedParticipations=$(echo "$api_result" | jq -r '.response.ssuser.propositionok')
	maxThreads=$(echo "$api_result" | jq -r '.response.ssuser.maxthreads')
	maxDownloadSpeed=$(echo "$api_result" | jq -r '.response.ssuser.maxdownloadspeed')
	requestsToday=$(echo "$api_result" | jq -r '.response.ssuser.requeststoday')
	maxRequestsPerDay=$(echo "$api_result" | jq -r '.response.ssuser.maxrequestsperday')
	maxRequestsPerMinute=$(echo "$api_result" | jq -r '.response.ssuser.maxrequestspermin')
	lastScrape=$(echo "$api_result" | jq -r '.response.ssuser.datedernierevisite')

	# Affichage des informations extraites
	clear
	echo -e "\n*****************************************************"
	echo -e "*************** SCREENSCRAPER ACCOUNT ***************"
	echo -e "*****************************************************\n\n"
	echo -e "\n\n id: $id"
	echo " Level: $level"
	echo " Financial contribution: $contribution"
	echo " Approved participations: $approvedParticipations"
	echo " maxthreads: $maxThreads"
	echo " max download speed: $maxDownloadSpeed"
	echo " requests today: $requestsToday"
	echo " max requests per day: $maxRequestsPerDay"
	echo " max requests per minute: $maxRequestsPerMinute"
	echo -e " Last scrape: $lastScrape\n\n\n\n\n\n\n\n\n\n\n\n"

	read -n 1 -s -r -p "Press A to continue"

}


##########################################################################################


Menu_Config_SSAccountSettings()
{
    
    echo "-------------------------------------------------"
    saveMetadata=false
    clear
    echo "Loading..."
    echo "==================================================="
    
    #We check for existing credentials
    
    if [ -f "$ScraperConfigFile" ]; then
    
        config=$(cat $ScraperConfigFile)
    
        userSS=$(echo "$config" | jq -r '.screenscraper_username')
        passSS=$(echo "$config" | jq -r '.screenscraper_password')
    
    
        if [ "$userSS" = "null" ] || [ "$passSS" = "null" ] || [ "$userSS" = "" ] || [ "$passSS" = "" ]; then
            userStored=false
        else
            userStored=true
        fi
    fi
    
    

        while true; do
			echo "username: $userSS"
			if ! [ -z "$passSS" ]; then
				passwordState="Password: xxxx (hidden)"
			else
				passwordState="Password: (not set)"
			fi
			clear
			Mychoice=$( echo -e "Screenscraper information\nUsername : $userSS\n${passwordState}\nAccount state and stats\nBack to configuration menu." | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "     --== SCREENSCRAPER ACCOUNT ==--" -b "Press A to validate your choice.")
			
			case "$Mychoice" in
				*Username\ *)
					clear
					echo -ne "\e[?25h"  # display the cursor
					echo -e "Press X to display the keyboard.\nPress A to enter a key.\nPress L1 for shift.\nPress R1 for backspace.\nPress Enter to validate.\n\n\n\nEnter your screenscraper username.\n\n"
					readline -m "username: "
					userSS=$(cat /tmp/readline.txt)
					userSS="${userSS// /}"  # removing spaces
					rm /tmp/readline.txt
					config=$(cat $ScraperConfigFile)
					config=$(echo "$config" | jq --arg user "$userSS" '.screenscraper_username = $user')
					echo "$config" > $ScraperConfigFile
					sync
					;;
				*Password:\ *)
					clear
					echo -ne "\e[?25h"  # display the cursor
					echo -e "Press X to display the keyboard.\nPress A to enter a key.\nPress L1 for shift.\nPress R1 for backspace.\nPress Enter to validate.\n\n\n\nEnter your screenscraper password.\n\n"
					readline -m "password: "
					passSS=$(cat /tmp/readline.txt)
					passSS="${passSS// /}"  # removing spaces
					rm /tmp/readline.txt
					config=$(cat $ScraperConfigFile)
					config=$(echo "$config" | jq --arg pass "$passSS" '.screenscraper_password = $pass')
					echo "$config" > $ScraperConfigFile
					sync
					;;
				*Screenscraper\ information*)
					Screenscraper_information
					;;
				*Account\ state\ and\ stats*)
					Screenscraper_accountState
					;;
				*Back\ to\ configuration\ menu.*)
					Menu_Config 
					break
					;;
				*)
					false
					;;
			esac

        done

    clear
}


##########################################################################################

Menu_Config_BackgroundScraping()
{
    

    # Check if the configuration file exists
    if [ ! -f "$ScraperConfigFile" ]; then
      echo "Error: configuration file not found"
      read -n 1 -s -r -p "Press A to continue"
      exit 1
    fi
    
    clear
    echo -e "====================================================\n\n"
    echo -e "Background scraping allows you to use your\nMiyoo during scraping.\n\nYou may experience slowdowns,so choose \nnon-demanding emulators.\n\nHowever you will not see download in live if enabled.\n"
    echo -e "====================================================\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
    read -n 1 -s -r -p "Press A to continue"
    clear
    
    config=$(cat "$ScraperConfigFile")
    ScrapeInBackground=$(echo "$config" | jq -r '.ScrapeInBackground')
    
    
    	Mychoice=$( echo -e "No\nYes\nBack to Configuration Menu" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "Background scraping ? (Currently: $ScrapeInBackground)" -b "Press A to validate your choice.")
        # TODO : add a new option to display the tail of the log to show what happens in background

        if [ "$Mychoice" = "Yes" ]; then
            ScrapeInBackground="true"
        elif [ "$Mychoice" = "No" ]; then
            ScrapeInBackground="false"
		elif [ "$Mychoice" = "Back to Configuration Menu" ]; then
			Menu_Config
			return
        fi

        config=$(cat $ScraperConfigFile)
        config=$(echo "$config" | jq --arg ScrapeInBackground "$ScrapeInBackground" '.ScrapeInBackground = $ScrapeInBackground')
        echo "$config" > $ScraperConfigFile
        sync
        Menu_Config
}


##########################################################################################


Menu_Config_MediaType()
{
    # Check if the configuration file exists
    if [ ! -f "$ScraperConfigFile" ]; then
      echo "Error: configuration file not found"
      read -n 1 -s -r -p "Press A to continue"
      exit 1
    fi


    # Display Welcome
    clear
    echo -e 
    echo -e "====================================================\n\n"
    echo -e " All the media types are not available on\n each scraper engine.\n\n"
    echo -e "	SS = Screenscraper" 
    echo -e "	LB = Launchbox" 
    echo -e "	RA = Retroarch\n\n"
  
    echo -e "====================================================\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
	
    # retrieve current media settings
    config=$(cat "$ScraperConfigFile")
    RetroarchMediaType=$(echo "$config" | jq -r '.RetroarchMediaType')
    ScreenscraperMediaType=$(echo "$config" | jq -r '.ScreenscraperMediaType')
    LaunchBoxMediaType=$(echo "$config" | jq -r '.LaunchboxMediaType')
	
    read -n 1 -s -r -p "Press A to continue"
    clear

    # Screenscreaper.fr
    Option01="Box Art                    (available on SS,LB,RA)"
    Option02="Screenshot - Title Screen  (available on SS,LB,RA)"
    Option03="Screenshot - In Game       (available on SS,LB,RA)"
    Option04="Box Art - 3D               (available on SS,LB)"
    Option05="Wheel                      (available on SS,LB)"
    Option06="Marquee                    (available on SS,LB)"
    Option07="Screenscraper Mix V1       (available on SS)"
    Option08="Screenscraper Mix V2       (available on SS)"
	Option09="Back to Configuration Menu"

    Mychoice=$( echo -e "$Option01\n$Option02\n$Option03\n$Option04\n$Option05\n$Option06\n$Option07\n$Option08\n$Option09\n" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t\ "Current media type : $ScreenscraperMediaType" -b "Press A to validate your choice.")
    
    [ "$Mychoice" = "$Option01" ]  && SSmediaType="box-2D"                 && LBmediaType="Box - Front"                      && RAmediaType="Named_Boxarts"  
    [ "$Mychoice" = "$Option02" ]  && SSmediaType="sstitle"                && LBmediaType="Screenshot - Game Title"          && RAmediaType="Named_Titles"  
    [ "$Mychoice" = "$Option03" ]  && SSmediaType="ss"                     && LBmediaType="Screenshot - Gameplay"            && RAmediaType="Named_Snaps"  
    [ "$Mychoice" = "$Option04" ]  && SSmediaType="box-3D"                 && LBmediaType="Box - 3D"                         && RAmediaType=""  
    [ "$Mychoice" = "$Option05" ]  && SSmediaType="wheel"                  && LBmediaType="Clear Logo"                       && RAmediaType=""  
    [ "$Mychoice" = "$Option06" ]  && SSmediaType="screenmarqueesmall"     && LBmediaType="Banner"                           && RAmediaType=""  
    [ "$Mychoice" = "$Option07" ]  && SSmediaType="mixrbv1"                && LBmediaType=""                                 && RAmediaType=""  
    [ "$Mychoice" = "$Option08" ]  && SSmediaType="mixrbv2"                && LBmediaType=""                                 && RAmediaType=""  
	[ "$Mychoice" = "$Option09" ]  && Menu_Config && return
					  
    clear


    config=$(cat $ScraperConfigFile)
    config=$(echo "$config" | jq --arg RAmediaType "$RAmediaType" '.RetroarchMediaType = $RAmediaType')
    config=$(echo "$config" | jq --arg LBmediaType "$LBmediaType" '.LaunchboxMediaType = $LBmediaType')
    config=$(echo "$config" | jq --arg SSmediaType "$SSmediaType" '.ScreenscraperMediaType = $SSmediaType')        
    echo "$config" > $ScraperConfigFile
    sync
    Menu_Config

}



##########################################################################################


Menu_RegionSelection()
{
    # Check if the configuration file exists
    if [ ! -f "$ScraperConfigFile" ]; then
      echo "Error: configuration file not found"
      read -n 1 -s -r -p "Press A to continue"
      exit 1
    fi

    # retrieve current media settings
    config=$(cat "$ScraperConfigFile")
    ScreenscraperRegion=$(echo "$config" | jq -r '.ScreenscraperRegion')


    # Display Welcome
    clear
    echo -e 
    echo -e "====================================================\n\n"
    echo -e " Select your country.\n\n"
    echo -e " If no media is found for your country code," 
    echo -e " other countries will be searched as fallback.\n" 
    echo -e "====================================================\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
    read -n 1 -s -r -p "Press A to continue"
    clear


Option01="Australia (au)"
Option02="Brazil (br)"
Option03="Canada (ca)"
Option04="China (cn)"
Option05="Finland (fi)"
Option06="France (fr)"
Option07="Germany (de)"
Option08="Greece (gr)"
Option09="Italy (it)"
Option10="Japan (jp)"
Option11="Korea (kr)"
Option12="Netherlands (nl)"
Option13="Norway (no)"
Option14="Spain (sp)"
Option15="Sweden (se)"
Option16="United States (us)"
Option17="United Kingdom (uk)"
Option18="Back to Configuration Menu"

    Mychoice=$( echo -e "$Option01\n$Option02\n$Option03\n$Option04\n$Option05n$Option06\n$Option07\n$Option08\n$Option09\n$Option10\n$Option11\n$Option12\n$Option13\n$Option14\n$Option15\n$Option16\n$Option17\n$Option18\n" \
	| /mnt/SDCARD/.tmp_update/script/shellect.sh -t\ "Current selected region: $ScreenscraperRegion" -b "Press A to validate your choice.") \
    
	SSregion=$(echo "$Mychoice" | sed -n 's/.*(\(.*\))/\1/p')
	LBregion=$(echo "$Mychoice" | sed -n 's/\(.*\) (.*)/\1/p')

	# exceptions :
	case "$Mychoice" in
	  "$Option12") LBregion="The Netherlands" ;;
	  "$Option18") Menu_Config; return; ;;
	esac
 clear

    config=$(cat $ScraperConfigFile)
    config=$(echo "$config" | jq --arg LBregion "$LBregion" '.LaunchboxRegion = $LBregion')
    config=$(echo "$config" | jq --arg SSregion "$SSregion" '.ScreenscraperRegion = $SSregion')        
    echo "$config" > $ScraperConfigFile
	sync    
    Menu_Config

}


##########################################################################################

Menu_Config_ScrapingSource()
{
    clear
    echo "Loading..."
    echo "==================================================="
    # The JSON file to modify
            
    # Read the content of the JSON file and convert it to a JSON string
    json=$(jq -c '.' "$ScraperConfigFile")
    
    
    # Recovering the values of variables
    Screenscraper_enabled=$(echo "$json" | jq -r '.Screenscraper_enabled')
    Launchbox_enabled=$(echo "$json" | jq -r '.Launchbox_enabled')
    Retroarch_enabled=$(echo "$json" | jq -r '.Retroarch_enabled')

    [ "$Screenscraper_enabled" = "true" ] && Screenscraper="[x] Screenscraper" || Screenscraper="[ ] Screenscraper"
    [ "$Launchbox_enabled" = "true" ] && Launchbox="[x] Launchbox" || Launchbox="[ ] Launchbox"
    [ "$Retroarch_enabled" = "true" ] && Retroarch="[x] Retroarch" || Retroarch="[ ] Retroarch"    
    
    while true; do
    
        Mychoice=$( echo -e "$Screenscraper\n$Launchbox\n$Retroarch\nBack to Configuration Menu" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "      --== SCRAPER SELECTION ==--" -b "Press A to validate your choice.")
        
        [ "$Mychoice" = "[ ] Screenscraper" ] && Screenscraper="[x] Screenscraper"
        [ "$Mychoice" = "[x] Screenscraper" ] && Screenscraper="[ ] Screenscraper"
        [ "$Mychoice" = "[ ] Launchbox" ] && Launchbox="[x] Launchbox"
        [ "$Mychoice" = "[x] Launchbox" ] && Launchbox="[ ] Launchbox"
        [ "$Mychoice" = "[ ] Retroarch" ] && Retroarch="[x] Retroarch"
        [ "$Mychoice" = "[x] Retroarch" ] && Retroarch="[ ] Retroarch"

        [ "$Mychoice" = "Back to Configuration Menu" ] && break
    
    done
    
    [ "$Screenscraper" = "[x] Screenscraper" ] && Screenscraper_enabled="true" || Screenscraper_enabled="false"    
    [ "$Launchbox" = "[x] Launchbox" ] && Launchbox_enabled="true" || Launchbox_enabled="false"
    [ "$Retroarch" = "[x] Retroarch" ] && Retroarch_enabled="true" || Retroarch_enabled="false"
   

    # Modify the JSON string with the new variable values
    json=$(echo "$json" | jq --arg Screenscraper "$Screenscraper_enabled" --arg Launchbox "$Launchbox_enabled" --arg Retroarch "$Retroarch_enabled" '. + { Screenscraper_enabled: $Screenscraper, Launchbox_enabled: $Launchbox, Retroarch_enabled: $Retroarch }')

    # Rewrite the modified JSON content to the file
    #echo "$json"    # for debugging
    echo "$json" > "$ScraperConfigFile"
	sync
    #echo "$json" | jq '.' > "$ScraperConfigFile"
    
    Menu_Config

}


##########################################################################################


Launch_Scraping ()
{
    touch /tmp/stay_awake
    rm -f /tmp/scraper_script.sh
	
	if [ "$(ip r)" = "" ]; then 
		echo "You must be connected to wifi to use Scraper"
		read -n 1 -s -r -p "Press A to continue"
		exit
	fi

    # Check if the configuration file exists
    if [ ! -f "$ScraperConfigFile" ]; then
      echo "Error: configuration file not found"
      read -n 1 -s -r -p "Press A to continue"
      exit 1
    fi
    
    # Read the content of the configuration file
    config=$(cat "$ScraperConfigFile")
    
    # Get the values of the "Retroarch_enabled", "Screenscraper_enabled" and "Launchbox_enabled" keys
    Screenscraper_enabled=$(echo "$config" | jq -r '.Screenscraper_enabled')
    Launchbox_enabled=$(echo "$config" | jq -r '.Launchbox_enabled')
    Retroarch_enabled=$(echo "$config" | jq -r '.Retroarch_enabled')
    ScrapeInBackground=$(echo "$config" | jq -r '.ScrapeInBackground')
    MediaType=$(echo "$config" | jq -r '.MediaType')
    
    
    [ "$onerom" = "1" ] && onerom="$romname" || onerom=""
    
    # Check the value of each variable and run the corresponding script if the value is "true"

    if [ "$Screenscraper_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_screenscraper.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_screenscraper.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then echo exiting $romimage; pkill st; fi;  # exit if only one rom must be scraped and is already found

    if [ "$Launchbox_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_launchbox.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_launchbox.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then  echo exiting $romimage ; pkill st; fi;
    
    if [ "$Retroarch_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_retroarch.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_retroarch.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then  echo exiting $romimage ; pkill st; fi;

	rm -f /tmp/stay_awake
	pkill st
}

##########################################################################################

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

##########################################################################################

Delete_Rom_Cover ()
{
    clear
    rm "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
    echo -e "$romNameNoExtension.png\nRemoved\n"
    read -n 1 -s -r -p "Press A to continue"
    clear
    Menu_Main
}


##########################################################################################

Menu_Main ()

{
clear
Option1="Scrape all $(basename "$CurrentSystem") roms"
[ -f "$romimage" ] && Option2="" || Option2="Scrape current rom: $romname"
[ -f "$romimage" ] && Option3="Delete cover: $romNameNoExtension.png" || Option3=""
Option4="Configuration"
Option5="Exit"

clear
Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4\nExit" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "           --== MAIN MENU ==--" -b "                     Menu : Exit        A : Validate ")

[ "$Mychoice" = "$Option1" ] && (onerom=0; Launch_Scraping;)
[ "$Mychoice" = "$Option2" ] && (onerom=1; Launch_Scraping;)
[ "$Mychoice" = "$Option3" ] && Delete_Rom_Cover
[ "$Mychoice" = "$Option4" ] && Menu_Config
[ "$Mychoice" = "$Option5" ] && exit


}


/mnt/SDCARD/.tmp_update/script/scraper/config_repair.sh &
Menu_Main

