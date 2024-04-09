---
slug: /apps/advancemenu
---


# AdvanceMENU

*An alternative user interface with animated game previews*

---


## About

[AdvanceMENU](http://www.advancemame.it/menu-readme) is an awesome frontend which has the ability to display video thumbnails of games. The performance of this frontend is impressive on low CPU devices like the Miyoo Mini. The animated thumbnails require a particular format, `.mng` files, which are a type of animated `.png` file and a separated `.mp3` sound file.

*This app is included in Onion 4.1.2*

<table><tr>
<td><img src="https://user-images.githubusercontent.com/34470397/228361104-0879a3a7-a5ea-4cc9-bc3e-c971bf5a0e24.png" /></td>
<td><img src="https://user-images.githubusercontent.com/34470397/228361129-051badae-5c95-4ca1-8595-b7d7aeabc70c.png" /></td>
<td><img src="https://user-images.githubusercontent.com/34470397/228361157-e6a7f53f-4d4a-4450-98f3-0b03bcae9600.png" /></td>
</tr><tr>
<td><img src="https://user-images.githubusercontent.com/34470397/228361197-86608c11-fc89-4b49-b029-5d536b3d75df.png" /></td>
<td><img src="https://user-images.githubusercontent.com/34470397/228361211-72b4049a-1c87-4eaf-a8e7-85bdff0a9d35.png" /></td>
<td><img src="https://user-images.githubusercontent.com/34470397/228361229-dbb78d48-d96a-43c8-a66d-3b83106cc4b7.png" /></td>
</tr></table>


## Instructions

1. AdvanceMENU from the Apps section of the package manager
2. Copy your snap files (`mng`+`mp3`) to `%ROMROOTFOLDER%/Snaps`
3. Launch the AdvanceMENU app from the Apps section
4. To show games from a specific emulator:
   - Press <kbd>START</kbd> and navigate to Emulators
   - Press <kbd>Y</kbd> to deselect each of the emulators
   - Press <kbd>A</kbd> to confirm
   - Now to can press <kbd>A</kbd> on any system to show games specific to that system
   - If you want to browse more then one system, you can select them again by pressing <kbd>Y</kbd> and confirming with <kbd>A</kbd>

From any regular game list, press <kbd>Y</kbd> and select "Open AdvanceMENU" to launch it to the list's specific emulator.

Furthermore, you can also set AdvanceMENU to be the start application, this is done via `Apps` › `Tweaks` › `System` › `Startup...` › `Start application`.


<table><tr>
<td><img src="https://user-images.githubusercontent.com/34470397/228361949-cf88379b-0693-414e-acc0-8836e05c3d5e.png" /></td>
<td><img src="https://user-images.githubusercontent.com/34470397/228361983-122d0187-96c6-4d12-b84a-f8a1833df675.png" /></td>
<td><img src="https://user-images.githubusercontent.com/34470397/228362003-2171d9b5-9f74-46d8-ac23-84c7015dd5b9.png" /></td>
</tr></table>



## Instructions for scraping your ROM collection

1. Download Skraper (https://www.skraper.net/)
2. Scrape your ROM collection every system at a time and on the media tab select only videos
3. On the OnionUI SDCARD find the folder `App\AdvanceMENU\tools\Roms_and_Medias_Management\mp4_to_mng`, copy it to your PC, make a new folder named videos and then copy the video files there, every system at a time, and run the script to convert to mng+mp3 files
4. Skraper video files start with 10 frames that are black so I made a script that removes the first 10 frames of every .mng file inside a folder. Download the script, copy it to your mng folder then run it. This step is optional. ([remove10.zip](https://github.com/OnionUI/Onion/files/11100644/remove10.zip))
5. Copy the mng folder content to the Snaps folder in every systems folder

This process can take some time and it can be a little bit tricky. Videos scraped with Skraper don't have the best resolution, they are not as sharp as the snap pack for MAME. 



## Premade snap packs

If you don't want to scrape your own ROM collection you can use the premade snap packs below, they are just drag and drop to the SDCARD. The pack uses videos that have the 10 first frames already removed so it's ready to go.

Download AdvanceMAME snap Pack (https://www.advancemame.it/download). There are 8 packs with sound, or 1 without sound.

Download Tiny Best Set Snap Pack 12 GB ([GDRIVE](https://drive.google.com/file/d/1KoHhrSxNa3u--W2rhGIwRdPQguP52tqD/view?usp=sharing)).



## Other

AdvanceMAME requires MAME 0.106 romset to work, you can find some rompacks on the link with the AdvanceMAME snap pack.

If you download the complete romset I recommend you run BestArcade (https://github.com/Voljega/BestArcade) to remove clones, duplicates and not working roms. This will not guarantee that every ROM will run on the Mini but it's a good start.

For PSX, if you use .m3u files and you don't want to have duplicates, you will need to edit `BIOS\.advance\advmenu.rc`, find `emulator_roms_filter "PSX"` and leave only `*.m3u`. 



## Configured Keymap

### AdvanceMENU

<table align="center">
<thead><th>  Button                             </th><th>  Action  </th></thead>
<tr><td>  <kbd>A</kbd>                          </td><td>  Validate  </td></tr>
<tr><td>  <kbd>B</kbd> or <kbd>MENU</kbd>       </td><td>  Back or exit  </td></tr>
<tr><td>  <kbd>X</kbd>                          </td><td>  Select sort mode for the game list (by name, emulator, type, year, size, manufacturer, play times...)  </td></tr>
<tr><td>  <kbd>Y</kbd>                          </td><td>  Select current item, useful for type filter menu  </td></tr>
<tr><td>  <kbd>L</kbd>/<kbd>R</kbd>             </td><td>  Prev/next page  </td></tr>
<tr><td>  <kbd>L2</kbd>/<kbd>R2</kbd>           </td><td>  Go to prev/next category (if you filter per type of game it goes to the next type)  </td></tr>
<tr><td>  <kbd>START</kbd>                      </td><td>  Display main menu  </td></tr>
<tr><td>  <kbd>SELECT</kbd>                     </td><td>  Change current display mode (Full, mixed, tile small, tile big, enormous...)  </td></tr>
</table>


### AdvanceMAME

<table align="center">
<thead><th>  Button                            </th><th>  Action    </th></thead>
<tr><td>  <kbd>A</kbd> <sub><sup>(Space)</sup></sub>                 </td><td>  Button 2  </td></tr>
<tr><td>  <kbd>B</kbd> <sub><sup>(Left Ctrl)</sup></sub>             </td><td>  Button 1  </td></tr>
<tr><td>  <kbd>X</kbd> <sub><sup>(Left Shift)</sup></sub>            </td><td>  Button 5  </td></tr>
<tr><td>  <kbd>Y</kbd> <sub><sup>(Left Alt)</sup></sub>              </td><td>  Button 4  </td></tr>
<tr><td>  <kbd>L1</kbd> <sub><sup>(E)</sup></sub>                    </td><td>  Button 3  </td></tr>
<tr><td>  <kbd>R1</kbd> <sub><sup>(T)</sup></sub>                    </td><td>  Button 6  </td></tr>
<tr><td>  <kbd>L2</kbd> <sub><sup>(Tab)</sup></sub>                  </td><td>  </td></tr>
<tr><td>  <kbd>R2</kbd> <sub><sup>(Backspace)</sup></sub>            </td><td>  </td></tr>
<tr><td>  <kbd>START</kbd> <sub><sup>(Enter)</sup></sub>             </td><td>  Configuration menu  </td></tr>
<tr><td>  <kbd>SELECT</kbd> <sub><sup>(Right Ctrl)</sup></sub>       </td><td>  Hotkey  </td></tr>
<tr><td colspan="2">&nbsp;</td></tr>
<tr><td>  <kbd>SELECT</kbd> + <kbd>START</kbd>            </td><td>  Config Menu  </td></tr>
<tr><td>  <kbd>SELECT</kbd> + <kbd>X</kbd>                </td><td>  Toggle FPS  </td></tr>
<tr><td>  <kbd>SELECT</kbd> + <kbd>Y</kbd>                </td><td>  <b>On-Screen Display:</b> little in game menu (volume, gamma, brightness, and overclock)  </td></tr>
<tr><td>  <kbd>SELECT</kbd> + <kbd>L2</kbd>/<kbd>R2</kbd> </td><td>  Increase / decrease frameskip  </td></tr>
<tr><td colspan="2">&nbsp;</td></tr>
<tr><td>  <kbd>START</kbd> + <kbd>UP</kbd>                </td><td>  Pause  </td></tr>
<tr><td>  <kbd>START</kbd> + <kbd>DOWN</kbd>              </td><td>  Reset game  </td></tr>
<tr><td>  <kbd>START</kbd> + <kbd>B</kbd>                 </td><td>  Start recording mng file, <kbd>START</kbd> to stop  </td></tr>
</table>


