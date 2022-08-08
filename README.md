<p>&nbsp;</p>

# <img alt="Onion" src="https://user-images.githubusercontent.com/44569252/179510333-40793fbc-f2a3-4269-8ab9-569b191d423f.png" width="196px">

*A performant and straightforward multi-language retro gaming museum in your pocket.*  
<sub>Made possible by Totofaki, Eggs, JimGray, Pixelshift, Shauninman and many more from the Onion community.</sub>

<p>&nbsp;</p>

<!--![onion_banner_new](https://user-images.githubusercontent.com/44569252/179326948-606421ec-5586-463e-a8a3-ed1ee1a50e5d.png)-->
![onion_banner_nologo](https://user-images.githubusercontent.com/44569252/179347740-4aee6e33-a133-4d02-8f0d-f13055c0a180.png)

<p>&nbsp;</p>

<h3 align="center"><a href="https://github.com/OnionUI/Onion/releases/latest"><img src="https://user-images.githubusercontent.com/44569252/179315622-e29e0971-87c8-4855-85e6-fc8de4ccd940.png" width="74"><br/>Download latest release</a><br><br>&nbsp;&nbsp; <sup>Read the installation guide below</sup> ⤸</h3>

<p>&nbsp;</p>

---

<p>&nbsp;</p>

* [Installation](#installation)
* [Features](#features)
* [System controls](#system-controls)
  
<p>&nbsp;</p>

<h1 align="center">Installation</h1>

<p>&nbsp;</p>

<p align="center"><img src="https://user-images.githubusercontent.com/16885275/164891118-efbcfc2e-bd25-4b88-8851-34862f550690.png" width="456"></p>

<p>&nbsp;</p>

**Attention:** Firmware version `20220419****` is needed for Onion v3.10 and above.    
You can check your current version in `Settings` ➜ `Device Info` ➜ `Version`. 

> If you can't or don't want to update, there are other solutions like older Onion versions and MiniUI.
> We can't take responsibility if your device is bricked in the process of upgrading the firmware version.
> Read the *firmware upgrade guide* <a href="https://user-images.githubusercontent.com/16885275/170205258-8add4be7-1a1e-4ae5-a8f2-cb13c6703e06.png" target="_blank">here</a>.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179306127-e8a2c99c-a078-46b0-9561-47abf5c16208.png" width="54"></sup>Fresh install

> *Note: Use a fast, trusted brand SD Card as the one included with the Miyoo Mini is slow and will fail/corrupt data.*
> *The following process will erase everything on your SD card. Please ensure any Roms, Bios & Saves are backed up to your PC.*

- Format your SD card as FAT32. (You will need to use a third party application to do this on Windows).
- Unzip the `.tmp_update` folder and its contents to the root of your SD. Nothing else should be on the SD card.
- If on Mac: This OS will add .\_* files everywhere in your SD, you need to remove them before continuing. You can go to your SD root in the **Terminal** and input `find . -name "._*" -depth -exec rm {} \;` to delete all the .\_* files 
- Boot up your Miyoo Mini and follow the on-screen instructions to get started.
- When installation is complete, press <kbd>A</kbd> and wait for the device to turn off.
- Keep the .tmp_update folder, it is now part of the Onion installation.
- Copy your Bios files into the `BIOS` folder and your roms into the applicable `Roms` subfolders for each system.
- Go to the console page, and refresh your rom lists by pressing the menu button.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179321292-8198613d-380c-4022-8ce6-ea020cc9b347.png" width="54"></sup>Upgrading from any Onion version

> *Note: In older versions up to V3.9, your saves and states were located here: `RetroArch/.retroarch/saves` | `states`.*

- Delete all folders from your SD except `BIOS`, `Roms`, `Saves` and `Themes` (Save your ports if you used some in `Emu/PORTS`).
- *Note:* Save your `Saves` folder to copy it back if you want to keep your custom emulator settings.
- Unzip the `.tmp_update` folder to the root of your SD.
- Boot up your Miyoo Mini and follow the on-screen instructions to get started.
- When installation is complete, press <kbd>A</kbd> and wait for the device to turn off.
- Remember to keep the `.tmp_update` folder, it is now part of the Onion installation.

<p>&nbsp;</p>

> <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179306509-8472e8a2-4989-4416-9fcf-d3b033c9e2fb.png" width="40"></sup>
> <h3><a href="https://github.com/OnionUI/Onion/wiki">Visit our wiki for additional information!</a></h3>

<p>&nbsp;</p>

<h1 align="center">Features</h1>

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179304061-647e63ff-5113-4a8a-aaa9-4dfda248d54e.png" width="54"></sup>Theme Switcher (with custom themes)
<img src="https://user-images.githubusercontent.com/16885275/172026971-2640251c-b781-4083-8715-e5e076e2cbfe.png" width="32%"> <img src="https://user-images.githubusercontent.com/16885275/172026972-c3602c24-b0df-43c6-942e-c975a2eac71b.png" width="32%"> <img src="https://user-images.githubusercontent.com/16885275/154789504-84253d41-373d-4c84-b194-547c6343f904.png" width="32%">

- Preview and change themes on the fly, no reboot required.
- Check out the [custom themes](https://github.com/OnionUI/Themes/blob/main/README.md) from our <sup><sub>❤️</sub></sup> community

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179302769-4169e57a-860f-4c0e-8792-007e7557ba48.png" width="54"></sup>The Onion Installer

<img src="https://user-images.githubusercontent.com/16885275/162589315-1d127c72-d404-4487-b379-3bde8179f566.png" width="350">

- Install/uninstall only the systems and apps you need. All actions are reversible.
- Community presets, homebrews and more.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179302722-7fa82e4d-d72d-4b1e-bb4d-96a2a52aaa62.png" width="54"></sup>Play Activity
<img src="https://user-images.githubusercontent.com/16885275/162589339-e779c6dd-4e9d-47f0-8543-4646d84fc748.png" width="350">

- Track your game playtimes.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179301923-635b60fa-22de-4cf3-894a-2f3c34702d64.png" width="54"></sup>Game Switcher
<img src="https://user-images.githubusercontent.com/16885275/164890313-c83dfc84-a684-416b-bd40-9f54c9f4e7db.png" width="350">

Although it's optional, we consider the game switcher to be central to the Onion UI experience.  
The Game Switcher is a user interface designed to be triggered when the Miyoo Mini starts.
It allows you to launch the last game played in a few seconds, and, with the simple press of a button, save your progress and turn off the console.  
It also allows you to quickly change games and many other features:

- Quick boot your last game played.
- Quickly switch games from your history.
- Full overlay with accurate brightness, battery readings and playtimes.
- Improved *Sleep Mode* with full game suspension (press <kbd>POWER</kbd>).
- Custom charging screen that prevents screen burn-in.
- Removed low bat flickering icon.
- Large new low battery (<10%) indicator with periodic warning rumble.
- When the battery goes below 4%, the game is automatically saved to prevent losing progress, and the handheld is turned off.

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179318731-7e262588-cb92-4ea3-9001-2991c4f8ccbe.png" width="54"></sup>RetroArch (rebuilt from scratch)
<img src="https://user-images.githubusercontent.com/16885275/154791260-d1a4d0b2-5582-45cc-a291-bead843a5171.png" width="350">

1. Rotation support for arcade games
2. Custom video filters  
   > You can now modify, or create your own

Some examples made for this RetroArch port:

**Normal_3X:**  
<img src="https://user-images.githubusercontent.com/16885275/166151805-fa2315c6-f783-4c7d-ba59-fd1996352fef.png" width="500"> 

**LCD filters, click to magnify (Integer scaling):**  
![](https://user-images.githubusercontent.com/16885275/173250366-dbaed067-640e-485d-8244-c62b9a7bd722.png)

- Compiled and partially rebuilt by [Eggs](https://discordapp.com/users/778867980096241715) for precision and performance  
  > Custom audio driver, custom scalers, new display driver, and a new input driver
- Minimal input and audio lag
- Customs cores
- Fine-tuned with optimal best settings for the Miyoo Mini in mind
- Crisp 640×480 resolution
- Can also work as a game launcher  
  > Cores embedded, playlists and favorites unlocked

<p>&nbsp;</p>

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179304290-8c7e5140-5fb4-4ae1-bd90-faa416f9a37c.png" width="54"></sup>Guest mode

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

<!--## <sup><img align="left" src="https://user-images.githubusercontent.com/16885275/168315341-d07bdf2f-b424-44d2-a845-e56ca67b04f1.png" width="54"></sup>Miyoo audio server adoption

The *pop* sound that appears each time a binary is launched is now gone.   

The audio sever lag has been removed. (Credit: Eggs)

<p>&nbsp;</p>-->

## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179305495-2ae1f522-6918-469f-a2eb-63dd30262e17.png" width="54"></sup>Main UI improvements

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
- For more information check the [FAQ](https://github.com/OnionUI/Onion/wiki/3.-Frequently-Asked-Questions-(FAQ))

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

<h1 align="center">System controls</h1>

<p>&nbsp;</p>

<p align="center"><img src="https://user-images.githubusercontent.com/16885275/178003377-10332587-c44e-46f6-b3cc-315df906a00b.png" width="500"></p>

<p>&nbsp;</p>

<table align="center">
<thead>
<tr>
<th>Button combination</th>
<th>Action</th>
<th><em>Active...</em></th>
</tr>
</thead>
<tbody>
<tr>
<td><kbd>MENU</kbd></td>
<td>Save and exit to Game Switcher</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> <sup>hold</sup></td>
<td>Save and exit to menu</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>L2</kbd></td>
<td>Load state</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>R2</kbd></td>
<td>Save state</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>L</kbd></td>
<td>Toggle rewind (if available)</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>R</kbd></td>
<td>Toggle fast forward</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>SELECT</kbd></td>
<td>RetroArch menu</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>X</kbd></td>
<td>Toggle FPS display</td>
<td><em>In-game</em></td>
</tr>
<tr>
<td><kbd>MENU</kbd> + <kbd>POWER</kbd></td>
<td>Take screenshot</td>
<td><em>Always</em></td>
</tr>
<tr>
<td><kbd>SELECT</kbd> + <kbd>L2</kbd> / <kbd>R2</kbd></td>
<td>Adjust brightness</td>
<td><em>Always</em></td>
</tr>
<tr>
<td><kbd>POWER</kbd></td>
<td>Sleep mode</td>
<td><em>Always</em></td>
</tr>
<tr>
<td><kbd>POWER</kbd> <sup>hold</sup></td>
<td>Save and turn off</td>
<td><em>Always</em></td>
</tr>
<tr>
<td><kbd>POWER</kbd> <sup>hold 5s</sup></td>
<td>Force quit the current app</td>
<td><em>Always</em></td>
</tr>
<tr>
<td><kbd>POWER</kbd> <sup>hold 10s</sup></td>
<td>Force turn off</td>
<td><em>Always</em></td>
</tr>
<tr>
<td><kbd>L2</kbd> / <kbd>R2</kbd></td>
<td>Change color palette</td>
<td><em>For GB only</em></td>
</tr>
</tbody>
</table>

<p>&nbsp;</p>

---

<p>&nbsp;</p>

<p align="center"><img src="https://user-images.githubusercontent.com/16885275/164891039-665fffcf-b454-4b3c-87c6-13a92cb88a8b.png" width="500"></p>
<p align="center"><img src="https://user-images.githubusercontent.com/16885275/164891137-2bbdcfb5-e2c2-4658-8049-79b01d57dfed.png" width="500"></p>

<p>&nbsp;</p>

<p align="right"><sub><i>Icons by <a href="https://icons8.com" target="_blank">Icons8</a></i></sub></p>
