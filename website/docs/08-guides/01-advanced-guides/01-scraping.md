---
slug: /advanced/scraping
---

# Scraping artwork for games

<sup>Credit: f8less & Julian</sup>

:::note
Onion now includes an internal scraper ([Scraper](../apps/scraper)). However for big games collections it will be probably faster to make it from a PC so this guide will help you to configure your scraping.
:::

## File type and placement

The image file format for scraped artwork is _.PNG_, with a max size of 250px (H) X 360px (W). The images need to be placed into the `Roms/<gamesystem>/Imgs` folder. 

The `Imgs` folder name is case sensitive (must have a capital `I`). Examples would be like the following:    
`Roms/FC/Imgs`  
`Roms/SFC/Imgs`  
`Roms/GB/Imgs`  


## Box art/screenshot scraping process


### Install

- Download & Extract _Skraper_ from: https://www.skraper.net/   
- Log in to Skraper


### Initial setup

- Select "_Recalbox_"   
- Tick "_Include non-Recalbox rom folders_"   
   > (Some systems aren't recognized because their folder-names are non-standard. To add those systems, press the `+` symbol on the bottom left, select the systems that are missing & hit "OK". Then, click the added systems in the bar on the left, and correct the system's folders via the "_GAMES & FRONT END_" tab -> "_Games/Roms folder:_" field -or- by clicking the folder-icon to the right of it.
[You can find a list of all Rom folder names here](../emulators)


### Media selection

**Use a (_Great!_) Custom Image Template Designed for OnionOS:**

**_Examples:_**<br/>
![image](https://user-images.githubusercontent.com/56418567/212767886-753a83ae-2f56-4255-a22d-f658ba656690.png)
![image](https://user-images.githubusercontent.com/56418567/212768343-a1d7d47b-1384-45a2-8f35-3d80b10fff5c.png)
![image](https://user-images.githubusercontent.com/56418567/212769101-5d5d5c77-bc23-43a2-83fd-859d938a0466.png)<br/>

**_Live Example:_**<br/>
![image](https://user-images.githubusercontent.com/56418567/212769542-49a3e1f4-971b-4fd4-bf79-36d589aee97a.png)<br/>


- Download template from the Retro Game Handhelds Discord:<br/>
[Skraper_Mix_-_Miyoo_Mini_Big_Zoom_by_AchillesPDX.zip](https://github.com/OnionUI/Onion/raw/main/files/20240310/Skraper_Mix_-_Miyoo_Mini_Big_Zoom_by_AchillesPDX.zip) - _Template by: AchillesPDX_
- Extract the _.ZIP_ file into the following _Skraper_ folder: `.\Skraper-#.#.#\Mixes\`
- Via the "ALL SYSTEMS"selection on the left, go to the "_MEDIA_" tab -> Clear the "_Fetched Media List_" of all but one image type by pressing the "_minus_" button.
- Change the "_Media type:_" to: "_USER PROVIDED MIX_" and click on the file icon to the right
- Select the extracted: `Miyoo Mini Big Zoom.xml`
- Disable settings "_Resize width to_" , "_Resize height to_" , "_Keep Image Ratio_" 

> _**- OR -**_

**Customize Image:**  
- Via "ALL SYSTEMS" on the left, go to the "_MEDIA_" tab -> Enable & set "_Resize width to_" to 250 and enable "_Keep Image Ratio_"    
- Select the image type you don't want in the "_Fetched Media List_" and press the "_minus_" button, so there's only one picture.   
   > You can change how the image mix looks with the two buttons under "_Media type_",  I recommend changing "_4 IMAGES MIX_" to "_Screenscraper's Recalbox Mix V2_", but use whatever you prefer.
   > If you choose a picture that's taller than it's wide, like the boxart, set "_Resize height to_" to 360 and disable "_Keep Image Ratio_"


### Output settings

- Change "_Output folder_" to `%ROMROOTFOLDER%\Imgs`, with a capital `I`.
- Under 'Gamelist Link' ensure that "_Link from node '&lt;thumbnail&gt;'_" is ticked as well as '_Optimize media storage_' (these are important if you wish to use the generated gamelist.xml to create a miyoogamelist.xml for use in Onion (more info on this below on this page).    
- Now click the system you want to scrape for on the left side, or "all system", and press the play button in the bottom right corner.    

This will automatically scrape images to the correct folders for Miyoo Mini.


### YouTube tutorials

#### Add Stunning Boxart To Your Miyoo Mini In Two Easy Ways _by RetroBreeze_

https://www.youtube.com/watch?v=RFu2DKRDq7o


#### EASY Scraping art for retro games (MiYoo Mini Skraper tutorial on Onion OS) _by TechDweeb_

https://www.youtube.com/watch?v=DguILcIyZQE&ab_channel=TechDweeb
