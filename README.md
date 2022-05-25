# Onion (and Eggs)
#### A performant and straightforward retro gaming museum in your pocket.
### [Download newest release here](https://github.com/jimgraygit/Onion/releases)

<img src="https://user-images.githubusercontent.com/16885275/166160523-67b6d683-5360-4786-ba87-c612eea32acd.png" width="600">

#### This release was made possible by Totofaki, Eggs, JimGray, Pixelshift, Shauninman and many more from the Onion community.

* [Installation](#installation)
* [Features](#features)
* [Shortcuts](#shortcuts)

#### [Check our wiki for additional documentation!](https://github.com/jimgraygit/Onion/wiki)

## Installation

The Miyoo firmware 20220419 is needed from Onion v3.10   
If you can't or don't want to update, there are other solutions: Older Onion versions and MiniUI are compatible with all firmwares.
I can't take responsibility if your device is bricked in the process.
Update instructions : [Here](https://user-images.githubusercontent.com/16885275/170203242-a69d9295-02b6-4e0c-a97b-44019e309692.png)

#### Installation (Fresh Install):
- Format your SD card as FAT32. (Use a fast, trusted brand. The SD card sent with your Miyoo Mini is slow, and will likely ruin your experience and data.)
- Unzip the `.tmp_update` folder and its contents to the root of your SD.
- Boot up your Miyoo Mini and follow the on-screen instructions to get started.
- Your Mini will turn off once installation is complete.
 (Don't delete the .tmp_update folder, it is now part of the Onion installation.)
- Copy your Bios files into the `BIOS` folder.

#### Installation (Upgrade):
- (If coming from <= V3.09, backup your saves (retroarch / .retroarch / saves | states), you will have to copy them afterwards in the `Saves` folder of your SD)
- Delete all folders from your SD except `BIOS`, `Roms`, `Saves` and `Themes`.
- Note : Save your `Saves` folder to copy it back if you want to keep your custom emulator settings.
- Unzip the `.tmp_update` folder to the root of your SD.
- Boot up your Miyoo Mini and follow the on-screen instructions to get started.
- Your Mini will turn off once installation is complete.
 (Don't delete the `.tmp_update` folder, it is now part of the Onion installation.)

<img src="https://user-images.githubusercontent.com/16885275/164891118-efbcfc2e-bd25-4b88-8851-34862f550690.png" width="456">

# Features

### Theme Switcher App and Custom Themes
<img src="https://user-images.githubusercontent.com/16885275/164838712-d45b3779-b30f-491c-b5ff-0bbc2a10865b.png" width="350">
<img src="https://user-images.githubusercontent.com/16885275/164838718-326f5590-96c2-4644-8fa2-1dd56f36a9bc.png" width="350">

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
- Improved brightness curve for better low-light gaming.
- Improved Sleep Mode with full game suspension (menu + power).
- Custom charging screen that prevents screen burn-in.
- Removed low bat flickering icon.
- Large new low battery (<10%) indicator with periodic warning rumble.
- When the battery goes below 4%, the game is automatically saved to prevent losing progress, and the handheld is turned off.

### RetroArch Rebuilt From Scratch
<img src="https://user-images.githubusercontent.com/16885275/154791260-d1a4d0b2-5582-45cc-a291-bead843a5171.png" width="350">

- Compiled and partially rebuilt by Eggs (Discord user no 968407509364772924) for precision and performance.
    Custom audio driver.
    Custom scalers.
    New display driver.
    New input driver.
- Minimal input and audio lag.
- Customs cores.
- Fine-tuned with optimal best settings for the Miyoo Mini in mind.
- Crisp 640*480 resolution.
- Lag free.
- Also a game launcher (cores embedded, playlists and favorites unlocked).

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

Power button: Save progress and shutdown  
Menu + Power: Light sleep

Menu + R2: Save state  
Menu + L2: Load state

Menu + R: Fast forward  
Menu + L: Rewind (if enabled)

Menu + X: Toggle FPS display

R2/L2: Change palette (Game Boy only)

Start + Select + R2 + L2: Force turn off  
Menu + Select : Retroarch menu.
   
Start + Select + Menu + R2 + L2: Force disable the launcher   

<img src="https://user-images.githubusercontent.com/16885275/164891039-665fffcf-b454-4b3c-87c6-13a92cb88a8b.png" width="500">
<img src="https://user-images.githubusercontent.com/16885275/164891137-2bbdcfb5-e2c2-4658-8049-79b01d57dfed.png" width="500">
