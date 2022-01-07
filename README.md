![githubcover](https://user-images.githubusercontent.com/85693713/148580152-0bc4aec1-310d-405c-8ab3-e7655991a7f3.png)

Custom Drag and Drop Solution for Miyoo Mini, based off the stock OS.
Pre-configured, debloated, fully skinnable.

Disclaimer: Proceed with caution and use at your own risk. (It all boots off SD so there really should be zero risk)

Credit (I am a glorified Project Manager, these folks are the reason this exists): 
Shaun Inman - Inspiration, Onion Logo, Onionizer theme engine, logo, Commander port
StubbornPixel for the Lightbox photos
PixelShift#2272 for the Galaxy theme, Black and invaluable input
Totofaki - Hotkey Mapping and Retroarch assistance
e1000 - Newly compiled cores for most systems
Boiler - Port of Powder (Licensed under a deprecated CC license (https://creativecommons.org/licenses/sampling+/1.0/))
Team Ruka - Various July 2021 cores compiled for RK3128
The whole Simple30 team for their PSX core
Home screen icons: Detailed Outline Icons by khld939 on Flaticon (https://www.flaticon.com/premium-icon/credit-card_2470194?related_id=2470194&origin=pack)
All my amazing testers!

Install Instructions:
1. IMPORTANT: Update your Miyoo Mini to the 12262021 Update or newer.
2. Format your microSD to FAT32.
3. Unzip the contents of Onion to the root of your SD Card.
4. Drag and drop your legally supplied ROMs to "/Roms/*system name*
5. Drag and drop your legally acquired Retroarch BIOS set to "/BIOS" (Currently provided for testing purposes)

Button Shortcuts:
Hotkey Enable - Menu Button
Hotkey + L - Load State
Hotkey + R - Save State
Hotkey + L2 - Rewind
Hotkey + R2 - Fast Forward
Hotkey + Select - Toggle Menu
Hotkey + Start - Quit
Select + start - Toggle Menu

Note: Please upgrade to the newest official firmware using the instructions here: https://github.com/TriForceX/MiyooCFW/wiki/Miyoo-Mini#firmware-update-guide 
**DO NOT USE ONION WITHOUT FIRST PERFORMING THIS UPGRADE**

Supported systems:<br>
Arcade (FBA2012 by default)<br>
Atari 2600<br>
Atari 7800<br>
Atari Lynx<br>
Bandai Wonderswan/Color<br>
Fairchild Channel F<br>
Famicom Disk System<br>
GCE Vectrex<br>
MSX/MSX2<br>
NEC PC Engine<br>
NEC PC Engine CD<br>
NEC SuperGrafx<br>
Nintendo Entertainment System<br>
Nintendo Game & Watch<br>
Nintendo Game Boy<br>
Nintendo Game Boy Color<br>
Nintendo Game Boy Advance<br>
Nintendo Pokemon Mini<br>
Nintendo Super NES<br>
PICO-8<br>
Sega 32X<br>
Sega CD<br>
Sega GameGear<br>
Sega Genesis<br>
Sega Master System Mk. III<br>
Sega SG-1000<br>
SNK NEOGEO<br>
SNK NEOGEO CD
SNK NEOGEO Pocket/Color<br>
Sony Playstation<br>
& an entire section of experimental cores for the tinkerers out there!<br>

Installation:
1. Format your microSD to FAT32.
2. Unzip the contents of Onion to the root of your SD Card. 
3. Drag and drop your legally supplied ROMs to the appropriate folders (see Wiki for details)
4. Drag and drop your legally acquired Retroarch BIOS set to "/RetroArch/system"
5. OPTIONAL: Open Apps > Onionizer on your Miyoo Mini. The device will shutdown after about 10 seconds.
6. OPTIONAL: Reinsert your SD card into your computer and navigate to the new /App/Onionizer/output folder.
7. OPTIONAL: Inside that folder are two hidden folders: .onion and .tmp_update. Copy **only the .tmp_update folder** to the root of your SD card. If you copy the .onion folder, you will reset the theme. See below for how to update your theme.

Onionizer Theme Engine by Shaun Inman:
The Onionizer will create a skin-able copy of the stock launcher 
that lives on your SD card instead of NAND (which is write-only
and cannot be modified).

1. Copy the Onionizer folder into the /App folder on your SD card.
2. Open Apps > Onionizer on your Miyoo Mini. The device will shutdown after about 10 seconds.
3. Reinsert your SD card into your computer and navigate to the new /App/Onionizer/output folder.
4. Inside that folder are two hidden folders: .onion and .tmp_update. Copy both to the root of your SD card.
5. Customize the images in the .onion folder
6. Next time you boot your Miyoo Mini it will launch the MainUI binary in .tmp_update that has been patched to load images from .onion.
NOTE: To use a prebuilt theme, make sure you have run Onionizer at least once since your last firmware update, then simply download the
theme and drag and drop the ".onion" folder to the root of your SD card.
