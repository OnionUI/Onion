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
    Option2="Modify your screenscraper.fr account."
    Option3="Scraping in background ?"
    Option4="Back to Main Menu"
    
    Mychoice=$( echo -e "$Option1\n$Option2\n$Option3\n$Option4" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "      --== CONFIGURATION MENU ==--" -b "Press A to validate your choice.")

    [ "$Mychoice" = "$Option1" ] && Menu_Config_ScrapingSelection
    [ "$Mychoice" = "$Option2" ] && Menu_Config_ScreenscraperAccount
    [ "$Mychoice" = "$Option3" ] && Menu_Config_BackgroundScraping
    [ "$Mychoice" = "Back to Main Menu" ] && Menu_Main
}

Menu_Config_ScreenscraperAccount()
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
					echo -e "Press X to display the keyboard and \nenter your screenscraper username\n\n"
					read -p "username : " userSS
					;;
				*Password\ :*)
					clear
					echo -e "Press X to display the keyboard and \nenter your screenscraper password\n\n"
					read -p "password : " passSS
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
    Retroarch_enabled=$(echo "$json" | jq -r '.Retroarch_enabled')
    Screenscraper_enabled=$(echo "$json" | jq -r '.Screenscraper_enabled')
    Launchbox_enabled=$(echo "$json" | jq -r '.Launchbox_enabled')
    

    [ "$Retroarch_enabled" = "true" ] && Retroarch="[x] Retroarch" || Retroarch="[ ] Retroarch"
    [ "$Screenscraper_enabled" = "true" ] && Screenscraper="[x] Screenscraper" || Screenscraper="[ ] Screenscraper"
    [ "$Launchbox_enabled" = "true" ] && Launchbox="[x] Launchbox" || Launchbox="[ ] Launchbox"
    
    
    
    while true; do
    
        Mychoice=$( echo -e "$Retroarch\n$Screenscraper\n$Launchbox\nBack to Configuration Menu" | /mnt/SDCARD/.tmp_update/script/shellect.sh -t "      --== SCRAPER SELECTION ==--" -b "Press A to validate your choice.")
        
        [ "$Mychoice" = "[ ] Retroarch" ] && Retroarch="[x] Retroarch"
        [ "$Mychoice" = "[x] Retroarch" ] && Retroarch="[ ] Retroarch"
        [ "$Mychoice" = "[ ] Launchbox" ] && Launchbox="[x] Launchbox"
        [ "$Mychoice" = "[x] Launchbox" ] && Launchbox="[ ] Launchbox"
        [ "$Mychoice" = "[ ] Screenscraper" ] && Screenscraper="[x] Screenscraper"
        [ "$Mychoice" = "[x] Screenscraper" ] && Screenscraper="[ ] Screenscraper"
        [ "$Mychoice" = "Back to Configuration Menu" ] && break
    
    done
    
    
    [ "$Retroarch" = "[x] Retroarch" ] && Retroarch_enabled="true" || Retroarch_enabled="false"
    [ "$Launchbox" = "[x] Launchbox" ] && Launchbox_enabled="true" || Launchbox_enabled="false"
    [ "$Screenscraper" = "[x] Screenscraper" ] && Screenscraper_enabled="true" || Screenscraper_enabled="false"
    

    # Modify the JSON string with the new variable values
    json=$(echo "$json" | jq --arg Retroarch "$Retroarch_enabled" --arg Screenscraper "$Screenscraper_enabled" --arg Launchbox "$Launchbox_enabled" '. + { Retroarch_enabled: $Retroarch, Screenscraper_enabled: $Screenscraper, Launchbox_enabled: $Launchbox }')
    

    # Rewrite the modified JSON content to the file
    #echo "$json"    # for debugging
    echo "$json" > "$ScraperConfigFile"
    #echo "$json" | jq '.' > "$ScraperConfigFile"
    
    Menu_Config


}





Launch_Scraping ()
{
    
    rm -f /tmp/scraper_script.sh
	
	if [ "$(ip r)" = "" ]; then 
		echo "You must be connected to wifi to use Scraper"
		sleep 3
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
    Retroarch_enabled=$(echo "$config" | jq -r '.Retroarch_enabled')
    Screenscraper_enabled=$(echo "$config" | jq -r '.Screenscraper_enabled')
    Launchbox_enabled=$(echo "$config" | jq -r '.Launchbox_enabled')
    ScrapeInBackground=$(echo "$config" | jq -r '.ScrapeInBackground')
    
    
    
    [ "$onerom" = "1" ] && onerom="$romname" || onerom=""
    
    # Check the value of each variable and run the corresponding script if the value is "true"
    
    if [ "$Retroarch_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_retroarch.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_retroarch.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then echo exiting $romimage; exit; fi;  # exit if only one rom must be scraped and is already found
    
    if [ "$Screenscraper_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_screenscraper.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_screenscraper.sh $CurrentSystem "$onerom"
        fi
    fi
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then echo exiting $romimage; exit; fi;  # exit if only one rom must be scraped and is already found
    
    if [ -f "$romimage" ] && ! [ "$onerom" = "" ] ; then  echo exiting $romimage ;  exit;fi;
    
    if [ "$Launchbox_enabled" = "true" ]; then
        if [ "$ScrapeInBackground" = "true" ]; then
            echo "/mnt/SDCARD/.tmp_update/script/scraper/scrap_launchbox.sh $CurrentSystem" \"$onerom\" >>/tmp/scraper_script.sh
        else
            /mnt/SDCARD/.tmp_update/script/scraper/scrap_launchbox.sh $CurrentSystem "$onerom"
        fi
    fi

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







