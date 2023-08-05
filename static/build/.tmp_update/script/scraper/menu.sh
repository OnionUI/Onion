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



Menu_Config()
{
    Option1="Select your scraping sources"
    Option2="Toggle background scraping"
    Option3="Screenscraper: account settings"
    Option4="Screenscraper: media preferences"
    Option5="Back to Main Menu"
    
    Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4\n$Option5" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "      --== CONFIGURATION MENU ==--" -b "Press A to validate your choice.")

    [ "$Mychoice" = "$Option1" ] && Menu_Config_ScrapingSelection
    [ "$Mychoice" = "$Option2" ] && Menu_Config_BackgroundScraping
    [ "$Mychoice" = "$Option3" ] && Menu_Config_SSAccountSettings
    [ "$Mychoice" = "$Option4" ] && Menu_Config_SSMediaPreferences
    [ "$Mychoice" = "$Option5" ] && Menu_Main
}

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
            echo "username: $userSS"
            echo "password: xxxx"
        fi
    fi
    
    

        while true; do
        
			Mychoice=$( echo -e "Username : $userSS\nPassword : xxxx\nScreenscraper information\nBack to configuration menu." | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "     --== SCREENSCRAPER ACCOUNT ==--" -b "Press A to validate your choice.")
			
			case "$Mychoice" in
				*Username\ :*)
					clear
					echo -ne "\e[?25h"  # display the cursor
					echo -e "Press X to display the keyboard and \nenter your screenscraper username\n\n"
					readline -m "username: "
					userSS=$(cat /tmp/readline.txt)
					userSS="${userSS// /}"  # removing spaces
					rm /tmp/readline.txt


					;;
				*Password\ :*)
					clear
					echo -ne "\e[?25h"  # display the cursor
					echo -e "Press X to display the keyboard and \nenter your screenscraper password\n\n"
					readline -m "password: "
					passSS=$(cat /tmp/readline.txt)
					passSS="${passSS// /}"  # removing spaces
					rm /tmp/readline.txt
					;;
				*Screenscraper\ information*)
					Screenscraper_information
					;;
				*Back\ to\ configuration\ menu.*)
					clear
					Menu_Config
					break
					;;
				*)
					false
					;;
			esac

			

			config=$(cat $ScraperConfigFile)
			config=$(echo "$config" | jq --arg user "$userSS" --arg pass "$passSS" '.screenscraper_username = $user | .screenscraper_password = $pass')
			echo "$config" > $ScraperConfigFile
               
        done

    clear
}


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
    echo -e "Background scraping allows you to use your\nMiyoo during scraping.\n\nYou may experience slowdowns,so choose \nnon-demanding emulators.\n\nHowever you will not see download in live if enabled.\n\n"
    echo -e "====================================================\n\n\n"
    read -n 1 -s -r -p "Press A to continue"
    clear
    
    config=$(cat "$ScraperConfigFile")
    ScrapeInBackground=$(echo "$config" | jq -r '.ScrapeInBackground')
    
    
    	Mychoice=$( echo -e "No\nYes" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "Background scraping ? (Currently: $ScrapeInBackground)" -b "Press A to validate your choice.")
        # TODO : add a new option to display the tail of the log

        if [ "$Mychoice" = "Yes" ]; then
            ScrapeInBackground="true"
        else
            ScrapeInBackground="false"
        fi

        config=$(cat $ScraperConfigFile)
        config=$(echo "$config" | jq --arg ScrapeInBackground "$ScrapeInBackground" '.ScrapeInBackground = $ScrapeInBackground')
        echo "$config" > $ScraperConfigFile
        
        Menu_Config
}

Menu_Config_SSMediaPreferences()
{
    # Check if the configuration file exists
    if [ ! -f "$ScraperConfigFile" ]; then
      echo "Error: configuration file not found"
      read -n 1 -s -r -p "Press A to continue"
      exit 1
    fi
    
    clear
    echo -e 
    echo -e "====================================================\n\n"
    echo -e "The Media Type affects the style of the graphics \nand images returned in ScreenScraper results.\n\n"
    echo -e "To prevent other scraping sources from overriding\nthis setting, ensure to deactivate them.\n\n"    
    echo -e "====================================================\n\n\n"
    read -n 1 -s -r -p "Press A to continue"
    clear
    
    config=$(cat "$ScraperConfigFile")
    MediaType=$(echo "$config" | jq -r '.MediaType')

    	Mychoice=$( echo -e "Box Art (default)\nBox Art - 3D\nScreenshot - Title Screen\nScreenshot - In Game\nWheel\nMarquee\nScreenscraper Mix V1\nScreenscraper Mix V2" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "Media Type ? (currently: $MediaType)" -b "Press A to validate your choice.")
        # TODO : add a new option to display tail of the log

        # TODO: Create a dictionary so we can support display and system names throughout the utility
        case "$Mychoice" in
            "Box Art (default)")
                MediaType="box-2D"
                ;;
            "Box Art - 3D")
                MediaType="box-3D"
                ;;
            "Screenshot - Title Screen")
                MediaType="sstitle"
                ;;
            "Screenshot - In Game")
                MediaType="ss"
                ;;
            "Wheel")
                MediaType="wheel"
                ;;
            "Marquee")
                MediaType="screenmarqueesmall"
                ;;
            "Screenscraper Mix V1")
                MediaType="mixrbv1"
                ;;
            "Screenscraper Mix V2")
                MediaType="mixrbv2"
                ;;
            *)
                false
                ;;                
        esac

        config=$(cat $ScraperConfigFile)
        config=$(echo "$config" | jq --arg MediaType "$MediaType" '.MediaType = $MediaType')
        echo "$config" > $ScraperConfigFile
        
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

    # retrieve current media settings
    config=$(cat "$ScraperConfigFile")
    RetroarchMediaType=$(echo "$config" | jq -r ('RetroarchMediaType')
    ScreenscraperMediaType=$(echo "$config" | jq -r ('ScreenscraperMediaType')
    LaunchBoxMediaType=$(echo "$config" | jq -r ('LaunchBoxMediaType')

    # Display Welcome
    clear
    echo -e 
    echo -e "====================================================\n\n"
    echo -e "The Media Type affects the style of the graphics \nand images returned in ScreenScraper results.\n\n"
    echo -e "To prevent other scraping sources from overriding\nthis setting, deactivate them.\n\n"    
    echo -e "====================================================\n\n\n"

    echo -e "Current Settings:\n"
    echo -e "-------------------------------------------------"
    echo -e "Rectroarch:    $RetroarchMediaTypee\n"
    echo -e "Screenscraper: $ScreenscraperMediaType\n"
    echo -e "LaunchBox:     $LaunchBoxMediaType\n\n"
    read -n 1 -s -r -p "Press A to continue"
    clear

    # Retroarch
    Option01="Box Art (default)"
    Option02="Screenshot - Title Screen"
    Option03="Screenshot - In Game"
 
    Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4\n$Option5" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t \
    "Retroarch scraping media type? (currently: $RetroarchMediaType))" -b "Press A to validate your choice.")
    
    [ "$Mychoice" = "$Option01" ]  && MediaType="Named_Boxarts"
    [ "$Mychoice" = "$Option02" ]  && MediaType="Named_Snaps"
    [ "$Mychoice" = "$Option03" ]  && MediaType="Named_Titles"
    clear

    # Screenscreaper.fr
    Option01="Box Art (default)"
    Option02="Screenshot - Title Screen"
    Option03="Screenshot - In Game"
    Option04="Wheel"
    Option05="Marquee"
    Option06="Screenscraper Mix V1"
    Option07="Screenscraper Mix V2"
    Option08="Box Art 3D"

    Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n'$Option4\n$Option5'n$Option6\n$Option7\n'$Option8\n$Option9'n$Option1" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t\ \
    "Screenscraper scraping media type ? (currently) $ScreenscraperMediaType)" -b "Press A to validate your choice.")
    
    [ "$Mychoice" = "$Option1" ]  && MediaType="box-2d"
    [ "$Mychoice" = "$Option2" ]  && MediaType="sstitle"
    [ "$Mychoice" = "$Option3" ]  && MediaType="ss"        
    [ "$Mychoice" = "$Option4" ]  && MediaType="wheel"
    [ "$Mychoice" = "$Option5" ]  && MediaType="screenmarqueesmall"
    [ "$Mychoice" = "$Option6" ]  && MediaType="mixrbv1" 
    [ "$Mychoice" = "$Option7" ]  && MediaType="mixrbv2" 
    [ "$Mychoice" = "$Option" ]  && MediaType="box-3D" 
    clear

    # Launchbox - libretro.org
    Option1="Box Art (default)"
    Option2="Screenshot - Title Screen"
    Option3="Screenshot - In Game"

    Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4\n$Option5" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t \
    "Launchbox scraping type ? (currently: $RetroarchMediaType)" -b "Press A to validate your choice.")

    [ "$Mychoice" = "$Option1" ]  && MediaType="box2dfront"
    [ "$Mychoice" = "$Option2" ]  && MediaType="wheel"
    [ "$Mychoice" = "$Option3" ]  && MediaType="screem"
    clear

    # TODO: Create a dictionary so we can support display and system names throughout the utility
    # Issues = Issues retrieving type, should define fallback regions for the type (see: https://github.com/zayamatias/EmulationStation/blob/52706db98a4affb2c1653e6ea3ae767d19f3ca78/es-app/src/scrapers/ScreenScraper.h#L23C11-L23C12)


    config=$(cat $ScraperConfigFile)
    config=$(echo "$config" | jq --arg RetroarchMediaType "$MediaType" '.RetroarchMediaType = $RetroarchMediaType')
    config=$(echo "$config" | jq --arg ScreenscraperMediaType "$MediaType" '.ScreenscraperMediaType = $ScreenscraperMediaType')
    config=$(echo "$config" | jq --arg LaunchboxMediaType "$MediaType" '.LaunchboxMediaType = $LaunchboxMediaType')        
    echo "$config" > $ScraperConfigFile
    
    Menu_Config






}












##########################################################################################

Menu_Config_ScrapingSelection()
{
    clear
    echo "Loading..."
    echo "==================================================="
    # The JSON file to modify
    

    [ ! -f "$ScraperConfigFile" ] && jq -n '{}' > "$ScraperConfigFile"     # If the file does not exist, create an empty JSON object
        
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
    #echo "$json" | jq '.' > "$ScraperConfigFile"
    
    Menu_Config


}





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
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then echo exiting $romimage; exit; fi;  # exit if only one rom must be scraped and is already found

    if [ "$Launchbox_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_launchbox.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_launchbox.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then  echo exiting $romimage ;  exit;fi;
    
    if [ "$Retroarch_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_retroarch.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_retroarch.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then  echo exiting $romimage ;  exit;fi;

	rm -f /tmp/stay_awake
    exit
}



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



Delete_Rom_Cover ()
{
    clear
    rm "/mnt/SDCARD/Roms/$CurrentSystem/Imgs/$romNameNoExtension.png"
    echo -e "$romNameNoExtension.png\nRemoved\n"
    read -n 1 -s -r -p "Press A to continue"
    clear
    Menu_Main
}




Menu_Main ()

{

Option1="Scrape all $(basename "$CurrentSystem") roms"
[ -f "$romimage" ] && Option2="" || Option2="Scrape current rom: $romname"
[ -f "$romimage" ] && Option3="Delete cover: $romNameNoExtension.png" || Option3=""
Option4="Configuration"
Option5="Exit"


Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4\nExit" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "           --== MAIN MENU ==--" -b "                     Menu : Exit        A : Validate ")

[ "$Mychoice" = "$Option1" ] && (onerom=0; Launch_Scraping;)
[ "$Mychoice" = "$Option2" ] && (onerom=1; Launch_Scraping;)
[ "$Mychoice" = "$Option3" ] && Delete_Rom_Cover
[ "$Mychoice" = "$Option4" ] && Menu_Config
[ "$Mychoice" = "$Option5" ] && exit


}



Menu_Main







