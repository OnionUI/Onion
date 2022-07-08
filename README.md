# Onion (and Eggs)
#### A performant and straightforward multi language retro gaming museum in your pocket.
### [Download newest release here](https://github.com/Sichroteph/Onion/releases)

<img src="https://user-images.githubusercontent.com/16885275/166160523-67b6d683-5360-4786-ba87-c612eea32acd.png" width="600">

#### This release was made possible by Totofaki, Eggs, JimGray, Pixelshift, Shauninman and many more from the Onion community.

* [Installation](#installation)
* [Features](#features)
* [Shortcuts](#shortcuts)

#### [Check our wiki for additional documentation!](https://github.com/Sichroteph/Onion/wiki)
  
  
## Installation

Firmware 20220419 is needed for Onion v3.10 and above.    
(You can check your current version in Settings - Device info - Version)  
If you can't or don't want to update, there are other solutions like older Onion versions and MiniUI.    
We can't take responsibility if your device is bricked in the process.   
Update instructions : [Here](https://user-images.githubusercontent.com/16885275/170205258-8add4be7-1a1e-4ae5-a8f2-cb13c6703e06.png)


#### Installation (Fresh Install):
- Format your SD card as FAT32. (Use a fast, trusted brand. The SD card sent with your Miyoo Mini is slow, and will likely ruin your experience and data.)
- Unzip the `.tmp_update` folder and its contents to the root of your SD.
- If using a Mac : This OS will add .-* files everywhere in your SD, you need to remove them before continue.
- Boot up your Miyoo Mini and follow the on-screen instructions to get started.
- Your Mini will turn off once installation is complete.
 (Don't delete the .tmp_update folder, it is now part of the Onion installation.)
- Copy your Bios files into the `BIOS` folder.
- Go to the console page, and refresh your rom lists by pressing the menu button.

#### Installation from any Onion version:
- *(Take note thats from V3.9 and below, your saves are located here : retroarch / .retroarch / saves | states)*
- Delete all folders from your SD except `BIOS`, `Roms`, `Saves` and `Themes` (Save your ports if you used some in `Emu\PORTS`)
- Note : Save your `Saves` folder to copy it back if you want to keep your custom emulator settings.
- Unzip the `.tmp_update` folder to the root of your SD.
- Boot up your Miyoo Mini and follow the on-screen instructions to get started.
- Your Mini will turn off once installation is complete.
 (Don't delete the `.tmp_update` folder, it is now part of the Onion installation.)

<img src="https://user-images.githubusercontent.com/16885275/164891118-efbcfc2e-bd25-4b88-8851-34862f550690.png" width="456">

# Features

### Theme Switcher App and Custom Themes
<img src="https://user-images.githubusercontent.com/16885275/172026971-2640251c-b781-4083-8715-e5e076e2cbfe.png" width="350">
<img src="https://user-images.githubusercontent.com/16885275/172026972-c3602c24-b0df-43c6-942e-c975a2eac71b.png" width="350">


<img src="https://user-images.githubusercontent.com/16885275/154789504-84253d41-373d-4c84-b194-547c6343f904.png" width="350">

- Preview and change themes on the fly, no reboot required.
- [Check themes from our community](https://github.com/jimgraygit/Onion/wiki/4.-Theme-Repository)

## Onion Installer

<img src="https://user-images.githubusercontent.com/16885275/162589315-1d127c72-d404-4487-b379-3bde8179f566.png" width="350">

- Install/uninstall only the systems and apps you need. All actions are reversible.
- Community presets, homebrews and more.

### Play Activity
<img src="https://user-images.githubusercontent.com/16885275/162589339-e779c6dd-4e9d-47f0-8543-4646d84fc748.png" width="350">

- Track your game play times.

### Onion Launcher
<img src="https://user-images.githubusercontent.com/16885275/164890313-c83dfc84-a684-416b-bd40-9f54c9f4e7db.png" width="350">

Although it's optional, we consider the launcher to be central to the Onion OS experience.  
The Onion Launcher is a user interface designed to be triggered when the Miyoo Mini starts.  
It allows you to launch the last game played in a few seconds, and, with a simple press of a button, save your progress and turn off the console.  
It also allows you to quickly change games and many other features:

- Quick boot your last game played.
- Quickly switch games from your history.
- Full overlay with accurate brightness, battery readings and playtimes.
- Improved Sleep Mode with full game suspension (menu + power).
- Custom charging screen that prevents screen burn-in.
- Removed low bat flickering icon.
- Large new low battery (<10%) indicator with periodic warning rumble.
- When the battery goes below 4%, the game is automatically saved to prevent losing progress, and the handheld is turned off.

### RetroArch Rebuilt From Scratch
<img src="https://user-images.githubusercontent.com/16885275/154791260-d1a4d0b2-5582-45cc-a291-bead843a5171.png" width="350">

1. Rotation support for arcade games.
2. Custom video filters. You can now modify, or create your own.

Some examples made for this RetroArch port :  
Normal_3X    
<img src="https://user-images.githubusercontent.com/16885275/166151805-fa2315c6-f783-4c7d-ba59-fd1996352fef.png" width="500"> 

LCD filters, click to magnify : (Integer + scaling)
![unknown](https://user-images.githubusercontent.com/16885275/173250366-dbaed067-640e-485d-8244-c62b9a7bd722.png)

- Compiled and partially rebuilt by Eggs (Discord user no 968407509364772924) for precision and performance.
    Custom audio driver.
    Custom scalers.
    New display driver.
    New input driver.
- Minimal input and audio lag.
- Customs cores.
- Fine-tuned with optimal best settings for the Miyoo Mini in mind.
- Crisp 640*480 resolution.
- Also a game launcher (cores embedded, playlists and favorites unlocked).

## Guest mode 
<img src="https://user-images.githubusercontent.com/16885275/168300116-7e0895c6-c28a-49ef-bf2a-009d4c9196a8.png" width="50">

Guest mode is a separate profile so you can safely lend your device to your kid or your friend.

This profile include another space where can be saved :
Saves
States
Video filter and scaling options.
Core options
Custom button remapping layouts.
RetroArch history (The Onion Launcher list will be different to reflect this.)
RetroArch playlists and favorites.
Timers for your games (Play activity app)
RGUI config (RetroArch Graphic User Interface)


## Your personal files are now gathered in one place.
Your saves and most of your preferences are now in SDCARD / Saves. (Example: D:/Saves/)
This also includes your secondary profile, your config files, RetroArch lists and play activity database.
Simply copy this folder when you want to backup your data.

## Miyoo audio server adoption
<img src="https://user-images.githubusercontent.com/16885275/168315341-d07bdf2f-b424-44d2-a845-e56ca67b04f1.png" width="80">
The "pop" sound that appears each time a binary is launched is now gone.   

The audio sever lag has been removed. (Credit: Eggs)

## Main UI improvments
### Battery percentage is now visible on the main menu. 
You can configure the battery percentage visibility and color in the theme configuration file. 
It is also possible to make a theme that only shows the percentage text.

No more audio lag (Credit: Eggs) 
New default theme inspired by the Lilla theme by Evolve. (Credit: DiMo) 
Onionos icon by Evolve.
Textures are compression for faster results (Credit: DiMo)
Box art size fix on the included themes (Credit: DiMo)
For more information check the [FAQ](https://github.com/jimgraygit/Onion/wiki/3.-FAQ-(Frequently-asked-questions))


### Others
- Many additional systems supported.
- Experimental Arduboy support (credit: JMARoeder).
- Updated PICO emulator to Fake08 standalone (credit: Supergrom).
- Updated screenshot tool to v4 (credit: eggs).
- Support for Icon Packs. Drag and drop custom icons into the "Icons" folder at the root of your SD.
- Boxart moved to /Roms/_systemname_/Imgs for easier scraping.
- Various bug fixes and optimizations.

### Shortcuts

<img src="https://user-images.githubusercontent.com/16885275/165266775-63e24f1b-d734-4eee-99c5-8bad502cd87e.png" width="500">

Menu button: Exit Game

Select + R2: Brightness up  
Select + L2: Brightness down

Power button long press : Save progress and shutdown  
Power button short press : Sleep mode  
Menu + Power: Shortcut

Menu + R2: Save state  
Menu + L2: Load state

Menu + R: Fast forward  
Menu + L: Rewind (if enabled)

Menu + X: Toggle FPS display

R2/L2: Change palette (Game Boy only)

Power button for 5s : force close the current app
Power button for 10s : force turn off
Menu + Select : Retroarch menu.  

<img src="https://user-images.githubusercontent.com/16885275/164891039-665fffcf-b454-4b3c-87c6-13a92cb88a8b.png" width="500">
<img src="https://user-images.githubusercontent.com/16885275/164891137-2bbdcfb5-e2c2-4658-8049-79b01d57dfed.png" width="500">
