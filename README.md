![githubcover](https://user-images.githubusercontent.com/85693713/148580152-0bc4aec1-310d-405c-8ab3-e7655991a7f3.png)

**Community Drag and Drop Solution for Miyoo Mini, based off the stock OS. Made by the community FOR the community.
**
<br>Pre-configured, debloated, fully skinnable.

**Disclaimers - If you read nothing else on this page, read this part**: 
<br>Proceed with caution and use at your own risk. (It all boots off SD so there really should be zero risk)
<br>Please submit feedback/bugs/core requests to onionsarentgross@gmail.com
<br>Do not report bugs for the cores in the retroarch section, those are provided for experimental purposes and are mostly untested. Report fixes and enhancements only.

Please submit feedback/bugs/core requests to onionsarentgross@gmail.com

Install Instructions:
1. IMPORTANT: You must be on the 20220108 firmware. (Flashing instructions: https://github.com/jimgraygit/OnionOS/wiki/Official-Miyoo-Firmware)
2. Format your microSD to FAT32 (if upgrading from a previous Onion build, delete all folders except BIOS, Roms and Retroarch).
3. Unzip the contents of Onion to the root of your SD Card (if upgrading, overwrite all existing files).
4. Drag and drop your legally supplied ROMs to "/Roms/*system name*
5. Drag and drop your legally acquired Retroarch BIOS set to "/BIOS"
<br>Note for the visually impaired, after installing and before ejecting your SD card from your PC, navigate to /Themes/Business/ and copy and paste the contents to the root of your SD card. 

Theme Instructions:
- OnionOS comes with PixelShift's "Galaxy" theme preinstalled
- To swap a theme (using Dingux Commander or your PC) simply copy and paste the miyoo directory of your chosen theme's folder to the root of your SD card, overwriting any existing theme's files.
- Reboot and enjoy!

Coming soon:
- More native ports
- Detailed Wiki
- Drag and drop mods and themes.
- Cleaned up per-system configs to hide redundant files (disc-based systems, etc)
- More systems and enhancements!

Known Issues (Most are stock kernel related):
- Some NES roms crash. Seems to only pertain to ones that are meant for virtual console on 3DS.
- .bin/.cue not recognized by PSX core.
- Occasionally after adding or removing ROMs, ROM titles will vanish until you select them. Delete the *.db file in that systems ROM folder. 
- SNES core has some flickering on SuperFX chip games but runs faster than other cores. Will continue to try and find the best SNES core.
- Large romsets (500ish) crash the menu back to the home screen. The current workaround is to press right on the D-Pad to enter coverart preview mode and scroll through the list there.
- Favorites section missing. Only known workaround is to delete (or not copy over) .tmp_update folder. This *will* disable custom themes.
- Some systems refuse to recognize boxart. No known workaround.
- Due to a memory leak at the kernel level, sometimes after switching roms or systems multiple times (typically over a dozen), the UI glitches out and requires a reboot.

Credit (I am a glorified Project Manager, these folks are the reason this exists): 
<br>Shaun Inman - Inspiration, Onion Logo, Onionizer theme engine, logo, Commander port: http://shauninman.com/
<br>StubbornPixel - Lightbox photos and shoutouts: https://brbgaming.simplecast.com/
<br>PixelShift#2272 on Discord - Galaxy theme, Black and invaluable input
<br>Totofaki - Hotkey Mapping and Retroarch assistance
<br>e1000 - Newly compiled cores for most systems
<br>Triforce Team - Translated instructions and english update (https://github.com/TriForceX/MiyooCFW/wiki/Miyoo-Mini#firmware-update-guide)
<br>Boiler - Port of Powder (Licensed under a deprecated CC license (https://creativecommons.org/licenses/sampling+/1.0/))
<br>Team Ruka - Various July 2021 cores compiled for RK3128 - https://github.com/Ruka-CFW/rk3128-cfw
<br>Simple30 team for their PSX core: https://retrogamecorps.com/2021/02/13/introducing-simple30-an-optimized-pocketgo-s30-firmware/#BIOS
<br>Home screen icons: Detailed Outline Icons by khld939 on Flaticon (https://www.flaticon.com/premium-icon/credit-card_2470194?related_id=2470194&origin=pack)
<br>All our amazing testers, especially HYJINX187, Totofaki, Jutleys, jmaroeder, JDewitz, Tshroom, Sezuko, therecanonlybeoneandrew and NexLevel!

Button Shortcuts:
<br>Hotkey Enable - Menu Button
<br>Hotkey + L - Load State
<br>Hotkey + R - Save State
<br>Hotkey + L2 - Rewind
<br>Hotkey + R2 - Fast Forward
<br>Hotkey + Select - Toggle Menu
<br>Hotkey + Start - Quit
<br>Select + Start + L2 - Brightness Down
<br>Select + Start + R2 - Brightness Up (Only works on home screen)

Note: Please upgrade to the newest official firmware using the instructions here: https://github.com/TriForceX/MiyooCFW/wiki/Miyoo-Mini#firmware-update-guide 
<br>**DO NOT USE ONION WITHOUT FIRST PERFORMING THIS UPGRADE**

Supported systems - - - supported filetypes:<br>
Arcade (FBA2012 by default) - - - .zip<br>
Atari 2600 - - - .a26 - .zip<br>
Atari 7800 - - - .a78 - .zip<br>
Atari Lynx - - - .lnx - .zip<br>
Bandai Wonderswan/Color - - - .ws - .wsc - .zip<br>
Fairchild Channel F - - - .bin - .zip<br>
Famicom Disk System - - - .fds - zip<br>
GCE Vectrex - - - .vec - .zip<br>
MSX/MSX2 - - - .mx2 - .zip<br>
NEC PC Engine - - - .pce - .zip<br>
NEC PC Engine CD - - - .bin/cue - .chd - .zip<br>
NEC SuperGrafx - - - .pce - .zip<br>
Nintendo Entertainment System - - - .nes - .zip<br>
Nintendo Game & Watch - - - .mgw - .zip<br>
Nintendo Game Boy - - - .gb - .zip<br>
Nintendo Game Boy Color - - - .gbc - .zip<br>
Nintendo Game Boy Advance - - - .gba - .zip<br>
Nintendo Pokemon Mini - - - .min - .zip<br>
Nintendo Super NES - - - .sfc - .smc - .zip<br>
Nintendo Virtual Boy - - - .vb - .zip<br>
PICO-8 - - - .p8<br>
Sega 32X - - - .32x - .zip<br>
Sega CD - - - .bin/.cue - .chd<br>
Sega GameGear - - - .gg - .zip<br>
Sega Genesis - - - .bin - .gen - .md - .zip<br>
Sega Master System MKIII - - - .sms - .zip<br>
Sega SG-1000 - - - .sg<br>
SNK NEOGEO - - - .zip<br>
SNK NEOGEO CD - - - .chd<br>
SNK NEOGEO Pocket/Color - - - .ngp - .ngpc - .zip<br>
Sony Playstation - - - .chd - .pbp<br>
Watara Supervision - - - .bin - .zip<br>
& an entire section of experimental cores for the tinkerers out there!<br>

Required BIOS files:
<br>Atari Lynx - lynxboot.img
<br>FAMICOM DISK SYSTEM - disksys.rom
<br>GAME BOY (Optional - for boot logo) - gb_bios.bin
<br>GAME BOY COLOR (Optional - for boot logo) - gbc_bios.bin
<br>GAME BOY ADVANCE (Optional) - gba_bios.bin
<br>NEOGEO - neogeo.zip
<br>PLAYSTATION - scph1001.bin (optional but recommended - any scphXXXX.bin file should work)
<br>SEGA CD - bios_CD_E.bin, bios_CD_J.bin, bios_CD_U.bin
<br>TURBOGRAFX-CD - syscard1.pce, syscard2.pce, syscard3.pce

Tips & Tricks:
- Prefer an experimental core to one in the GAME section? Simply move the .so file and the launch.sh file from the core's folder in /RApps for the appropriate folder in /Emus
- Too many systems? To hide systems or cores simply delete their folder from the /Emus folder or the /RApps folder, respectively.
