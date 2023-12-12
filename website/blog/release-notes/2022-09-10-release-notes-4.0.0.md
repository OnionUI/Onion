---
slug: release-notes/4.0.0
authors: [aemiii91]
tags: [release-notes, stable]
image: ./assets/banner-release-4.0.png
---

# Release notes: Onion V4

<img src={assets.image} />

*Official release notes for Onion V4.0.0*

This release introduces a new app, *Tweaks*, containing everything you need to *make Onion your own!* Built-in global search. Improved theme support.

<Truncate />

## Changelog

- **Minimal UI:** Recents are now hidden by default and GameSwitcher can be launched by pressing <kbd>MENU</kbd>
- **Tweaks:** A new app for making Onion your own!
   - System settings: Startup behavior, auto-save and exit, vibration
   - **Custom shortcuts:** Single/long/double press <kbd>MENU</kbd>, and launch apps or tools via <kbd>X</kbd> or <kbd>Y</kbd> in MainUI
      - **Known limitation:** Some apps can't be launched this way (for now only Music Player / GMU is known not to support this)
   - Personalize the UI: Show/hide recents/expert tabs, theme overrides
   - Advanced: Quick access to advanced emulator settings, plus a submenu to reset different settings
   - **Tools:** Sort favorites, fix thumbnails, remove OSX system files
- MainUI context menu is now mapped to <kbd>SELECT</kbd> (<kbd>MENU</kbd> default action is instead GameSwitcher - this can be changed in Tweaks)
- **GameSwitcher:** Minimal mode (press <kbd>Y</kbd> to toggle - hold <kbd>Y</kbd> for fullscreen image)
- **Package Manager:** Besides the new name, the old "Onion Installer" has also gotten a massive overhaul:
   - "Changes count" now shown for each tab and the total emu/apps added/removed are shown in the top bar
   - Press <kbd>X</kbd> to toggle all items in selected tab
   - Press <kbd>Y</kbd> to reset all changes in selected tab
- **Search app** is now native to Onion ([more info](https://github.com/OnionUI/Onion#search))
- **RetroArch:** supports save state thumbnails, less on-screen notifications
- **File Explorer:** Updated color scheme, fullscreen image viewer (with navigation)
- Retired unused cores: `mame2003` (use `mame2003-plus` instead), `fbalpha` (use `fbalpha2012` instead)
- Updated/added cores: ` Fake-08 (pico8)` , `blueMSX` and `fMSX`, 
- New experimental emulator: `PCSX-ReARMed standalone` and `mame2003-extreme`
- New theme features: custom boot/shutdown/save splash, charging animation (up to 24 frames), more options for battery percentage

## Ports collection

- The *Ports collection* binaries have moved to `Roms/PORTS`, and a reinstall via Package Manager is required.

## ScummVM game list

- We've changed the way ScummVM games are indexed, which now allows showing the games as single list items with the possibility of having boxart ([more info](https://github.com/OnionUI/Onion/wiki/Emulators#scummvm-))

## GB/GBA Fast Forward

- Fast forward is no longer additionally mapped to <kbd>R</kbd> in the Game Boy emulators.
- This is to unify the shortcut for fast forward across all systems (<kbd>MENU</kbd> + <kbd>R</kbd>).
- *Note: [Click here](https://github.com/OnionUI/Onion/wiki/Frequently-Asked-Questions-(FAQ)#how-do-i-bind-fast-forward-to-a-single-button:~:text=How%20do%20I%20bind%20Fast%20Forward%20to%20a%20single%20button%3F) for information on how to bind fast forward to a single button.*

# Installation

- Check out the new [installation guide](https://github.com/OnionUI/Onion/wiki/Installation).
- **Important:** If you choose "Update" during installation, it is recommended to reinstall included apps using the Package Manager.
- **Note:** if you experience slower gameplay than usual, we recommend you create an in-game save and delete your save states.

## Updating (from v4.0.0-RC)

- If updating from the release candidate you can leave out `BIOS` and `Icons` when copying files to your SD card.

## Battery icon hotfix

- If you're having trouble with the battery icon not showing when in MainUI, unzip the contents of the [`battery-icon-hotfix.zip`](https://github.com/OnionUI/Onion/releases/download/v4.0.0/battery-icon-hotfix.zip) to your SD card.
  - *This fix has already been applied to the Onion install zip below.*
