![onionlogo](https://user-images.githubusercontent.com/85693713/147612352-97b0cf56-bb0b-4078-992f-1c408a42d088.png)
CFW for Miyoo Mini, based off the stock OS.
Pre-configured, debloated, skinnable.

**Credit to Shaun Inman for the Commander port, Onionizer Theme Engine and Onion logo**

Note: Please upgrade to the newest official firmware using the instructions here: (link). 

**DO NOT ATTEMPT TO INSTALL THIS FIRMWARE WITHOUT FIRST PERFORMING THIS UPGRADE**

**Disclaimer: This is an untested WIP. I am not liable for bricks, borks, bugs or barf. Proceed with caution and use at your own risk.
**

Supported systems:
Arcade
Atari 2600
Atari 5200
Atari 7800
Atari 800
Fairchild Channel F

Installation:
1. Format your microSD to FAT32.
2. Unzip the contents of Onion to the root of your SD Card. 
3. Drag and drop your legally supplied ROMs to the appropriate folders (see Wiki for details)
4. Drag and drop your legally acquired Retroarch BIOS set to "/RetroArch/system"

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
