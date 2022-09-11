<p>&nbsp;</p>

# <img alt="Onion" src="https://user-images.githubusercontent.com/44569252/179510333-40793fbc-f2a3-4269-8ab9-569b191d423f.png" width="196px">

*A performant and straightforward multi-language retro gaming museum in your pocket.*

<p>&nbsp;</p>

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189437613-6fe9a5ed-b47a-4d4f-babe-a4236f21d6e3.gif"></p>

<p>&nbsp;</p>

<h3 align="center"><a href="https://github.com/OnionUI/Onion/wiki"><img src="https://user-images.githubusercontent.com/44569252/179306509-8472e8a2-4989-4416-9fcf-d3b033c9e2fb.png" width="40"><br/>Getting Started Guide</a></h3>

<p>&nbsp;</p>

<h3 align="center"><a href="https://github.com/OnionUI/Onion/releases/latest"><img src="https://user-images.githubusercontent.com/44569252/179302769-4169e57a-860f-4c0e-8792-007e7557ba48.png" width="40"><br/>Download Latest Release</a></h3>

<p>&nbsp;</p>

<h1 align="center">Features</h1>

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/189434056-b1148ff9-393b-43d1-b362-20d97b64b393.png" width="54"></sup>Global Shortcuts

<p align="center"><a href="https://github.com/OnionUI/Onion/wiki/Global-Shortcuts"><img src="https://user-images.githubusercontent.com/44569252/189388343-3fa73c5d-31ca-4931-b310-4e7ba43c536d.png" width="640"></a></p>

<p align="center"><a href="https://github.com/OnionUI/Onion/wiki/Global-Shortcuts">Click here</a> to see all the shortcuts.</p>

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179301923-635b60fa-22de-4cf3-894a-2f3c34702d64.png" width="54"></sup>GameSwitcher

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189434217-72ef0daf-c630-4ec9-b96a-30fd76cb709f.png" width="320"> <img src="https://user-images.githubusercontent.com/44569252/189434224-aeba7b8f-c881-4784-ba2f-65c60d41d20c.png" width="320"></p>

The GameSwitcher is designed to be the central user interface of Onion.
Launch the GameSwitcher by pressing the <kbd>MENU</kbd> button.
The last game played will be resumed in a few seconds, and with the simple press of a button, save your progress and turn off the console.  
It also allows you to quickly change games and many other features:

- Quick boot your last played game.
- Quickly switch games from your history.
  - Remove a game by pressing <kbd>X</kbd>.
- Full overlay with accurate brightness, battery readings and playtimes.
  - Press <kbd>SELECT</kbd> to toggle playtime display.
- **Minimal view mode:** Press <kbd>Y</kbd> to toggle.
- Improved *Sleep Mode* with full game suspension (press <kbd>POWER</kbd>).
- Low battery indicator: Red frame when <15% (can be adjusted in Tweaks).
- When the battery goes below 4%, the device will automatically save and exit to prevent losing progress.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179315622-e29e0971-87c8-4855-85e6-fc8de4ccd940.png" width="54"></sup>Package Manager

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189438069-2926cf7b-1e72-476f-8b41-79132146d7c9.png" width="320"></p>

- Choose the systems and apps you need.
- Optimized presets, homebrew apps, and more.
- Packages are divided into three sections:
  - **Verified:** Contains all verified systems (see [Emulators](https://github.com/OnionUI/Onion/wiki/Emulators) for more information).
  - **Apps:** Contains all the included Onion and third-party apps.
  - **Expert:** Contains experimental/expert systems (see [Emulators](https://github.com/OnionUI/Onion/wiki/Emulators) for more information).
- To reinstall a package, first toggle it off and press <kbd>START</kbd> to apply, then open Package Manager, toggle it on and apply.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/189439761-e03a9684-ade4-4dcc-8fdd-c006a1663c05.png" width="54"></sup>Tweaks

*Make Onion your own!*

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189440370-23849320-e130-4cdf-a1d6-ca97d2696e3a.png" width="320"></p>

- **System settings:** Startup behavior, auto-save and exit, vibration
- **Custom shortcuts:** Single/long/double press <kbd>MENU</kbd>, and launch apps or tools via <kbd>X</kbd> or <kbd>Y</kbd> in MainUI
  - *Known limitation:* Some apps can't be launched this way (for now only Music Player / GMU is known not to support this)
- **User interface:** Show/hide recents/expert tabs, theme overrides
- **Advanced:** Quick access to advanced emulator settings, plus a submenu to reset different settings
- **Tools:** Sort favorites, fix thumbnails, remove OSX system files

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189440460-0afffde0-2e5e-41a4-b85e-0484b23c461a.png"></p>

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/189498482-2590f31f-cca2-46e9-a316-3af98828446a.png" width="54"></sup>Search

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189498639-8e2a43a6-4020-4492-b4b1-6e3f0c0d5fd6.png" width="320"> <img src="https://user-images.githubusercontent.com/44569252/189498645-f615dd73-ed0c-4505-a439-5fb5b611237d.png" width="320"></p>

- The *Search* app lets you search all your games.
  - *Note: For now only verified systems are supported (the ones that are in the `Games` tab).*
- You'll find Search under `Apps` - and it's a popular choice for mapping to <kbd>X</kbd> or <kbd>Y</kbd> (via Tweaks).
- When a search is active, the results will be shown under `Games` ➜ `Search`, you can remove the search again by choosing "Clear search".
- "Missing caches" lists all systems which haven't been cached yet, and thereby haven't been included in the search.
  - *Reason:* All game caches are cleared everytime you "Refresh all roms".
  - *Solution:* Go into each system you want included in the search.

**Filter**
- You can install *Filter* via Package Manager, which will add a "Filter" and "Refresh roms" shortcut to each game list.
  - **Filter:** Will prompt you to enter a keyword for filtering the selected game list.
  - **Refresh roms:** Will refresh roms only for the selected game list.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179304061-647e63ff-5113-4a8a-aaa9-4dfda248d54e.png" width="54"></sup>Themes

<p align="center"><a href="https://github.com/OnionUI/Themes/blob/main/README.md"><img src="https://user-images.githubusercontent.com/44569252/189352231-03ae9688-a859-41c2-a8e3-4eba8ef360c8.png"></a></p>

- Preview and change themes.
- [Get more themes](https://github.com/OnionUI/Themes/blob/main/README.md) from our <sup><sub>❤️</sub></sup> community

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179302722-7fa82e4d-d72d-4b1e-bb4d-96a2a52aaa62.png" width="54"></sup>Activity Tracker

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189435280-9de4f088-b013-4c71-a44b-71f953b128a8.png" width="320"></p>

- Track your game playtimes (format `h:m`).
- View your total playtime in the top bar (format `h:m`).
- Share your playtimes by taking a screenshot (press <kbd>MENU</kbd>+<kbd>POWER</kbd> - screenshot is saved in `Screenshots`).

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179318731-7e262588-cb92-4ea3-9001-2991c4f8ccbe.png" width="54"></sup>RetroArch

*Rebuilt for Miyoo Mini - synched with [RA main](https://github.com/libretro/RetroArch)*

<p align="center"><img src="https://user-images.githubusercontent.com/44569252/189438841-f419f936-6376-436e-89b5-78ce1f88494f.png" width="320"></p>

- Save State Thumbnails enabled
- Compiled and partially rebuilt by [Eggs](https://discordapp.com/users/778867980096241715) for precision and performance  
  > Custom audio driver, custom scalers, new display driver, and a new input driver
- Minimal input and audio lag
- Customs cores
- Fine-tuned with optimal best settings for the Miyoo Mini in mind
- Crisp 640×480 resolution
- Can also work as a game launcher  
  > Cores embedded, playlists and favorites unlocked
- Rotation support for arcade games
- Custom video filters  
  > You can now modify, or create your own

Some examples of video filters made for this RetroArch port (click to view full resolution):

**Normal_3X:**  
<img src="https://user-images.githubusercontent.com/16885275/166151805-fa2315c6-f783-4c7d-ba59-fd1996352fef.png" width="240"> 

**LCD filters, click to magnify (Integer scaling):**  
![](https://user-images.githubusercontent.com/16885275/173250366-dbaed067-640e-485d-8244-c62b9a7bd722.png)

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179304290-8c7e5140-5fb4-4ae1-bd90-faa416f9a37c.png" width="54"></sup>Guest Mode

Guest mode is a separate profile so you can safely lend your device to your kid or your friend. The profile contains the following properties:
- Saves
- States
- Video filter and scaling options
- Core options
- Custom button remapping layouts
- RetroArch history  
  > The Game Switcher list will be different to reflect this
- RetroArch playlists and favorites
- Timers for your games  
  <sup>Play Activity app</sup>
- RGUI config  
  <sup>RetroArch Graphic User Interface</sup>

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179305837-59922e34-e18f-45d1-b5fb-89593ee08152.png" width="54"></sup>Personal files gathered in one place

Your saves and most of your preferences are now in `/Saves` (Example: `D:/Saves/`).
This also includes your secondary profile, your config files, RetroArch lists and play activity database.
Simply copy this folder when you want to backup your data.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179305495-2ae1f522-6918-469f-a2eb-63dd30262e17.png" width="54"></sup>MainUI improvements

- Battery percentage is shown in the top bar  
  > You can configure the battery percentage visibility and color in the theme configuration file. It is also possible to make a theme that only shows the percentage text.
- No more audio lag  
  <sup>Credit: Eggs</sup>
- New default theme inspired by the Lilla theme by Evolve  
  <sup>Credit: DiMo</sup>
- OnionOS icon by Evolve
- Textures are compressed for faster results  
  <sup>Credit: DiMo</sup>
- Box art size fix in the included themes  
  <sup>Credit: DiMo</sup>
- For more information check the [FAQ](https://github.com/OnionUI/Onion/wiki/Frequently-Asked-Questions-%28FAQ%29)

### Others
- Many additional systems supported
- Experimental Arduboy support  
  <sup>Credit: JMARoeder</sup>
- Updated PICO emulator to Fake08 standalone  
  <sup>Credit: Supergrom</sup>
- Updated screenshot tool to v4  
  <sup>Credit: Eggs</sup>
- Support for game system icon packs   
  > Drag and drop custom icons into the `Icons` folder at the root of your SD
- Boxart moved to `/Roms/[SYSTEM]/Imgs` for easier scraping
- Various bug fixes and optimizations

<p>&nbsp;</p>

<p align="right"><sub><i>Icons by <a href="https://icons8.com" target="_blank">Icons8</a></i></sub></p>
