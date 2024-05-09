---
slug: /emulators/pico-8-standalone
description: PICO-8 Standalone by XK9274
---
# Pico-8 Standalone ⚠

<img src="https://user-images.githubusercontent.com/44569252/188293050-691f7376-544e-4275-a612-bb042576dbe3.png" align="right" width="240" />

- Emulator: **PICO-8**
- Rom Folder: `PICO`
- Extensions: `.p8` `.png`
- Bios: None

---

The PICO-8 Native Wrapper allows you to play PICO-8 without the need for emulation (hence providing full compatibility with all PICO-8 games), and grants access to Splore, a "frontend" app that allows you to browse, search and play through the entirety of the PICO-8 catalogue (as indexed by the [Lexaloffle BBS](https://www.lexaloffle.com/bbs/)), as long as you have an internet connection. 

Splore also allows you to navigate through and play local cartridge files.

This is an alternative to the RetroArch `fake-08` core, which has access to all typical RetroArch features such as rewinding, fast forward and save state management, but can have compatibility with a portion of the PICO-8 library, specially recently released games.

## How to Enable and Launch

1. Enable the `PICO-8 (PICO-8 standalone)` emulator using the Onion Package Manager's *Expert* section.

2. In order to use the wrapper, you need to have previously purchased PICO-8. It is available as a single purchase at [Lexaloffle's website](https://www.lexaloffle.com/games.php?page=updates). Once downloaded, place the `pico8_dyn` and `pico8.dat` files from the **Raspberry Pi** package in the `/RApp/PICO-8/bin` directory of your Onion SD card.

:::tip
Do not use the FTP wireless transfer service to place these files on the SD card, as it may strip their execution privileges and cause the application to fail to launch.
:::

3. Afterwards, launch Splore from the `Run PICO-8 with Splore` option under the Onion *Expert* submenu (available under *Apps* in the main menu).

## Basic Usage

Using Splore you can browse, search and play through cartridges listed on the Lexaloffle PICO-8 BBS. Use the <kbd>LEFT</kbd> and <kbd>RIGHT</kbd> buttons to switch through the following sections:

* **Favorites**: Lists the games you have previously marked as favorites.
* **New**: Shows the most recently indexed cartridges on the Lexaloffle BBS.
* **Featured**: Shows a list of curated and highly rated cartridges by the Lexaloffle team.
* **Work in Progress**: Incomplete cartridges currently in development.
* **Jam**: Cartridges developed for game jam events hosted regularly by Lexaloffle.
* **Lucky Draw**: A random game selection.
* **Search**: Allows you to search for cartridges based on their names and description. Use the the up/down buttons to select an alphanumeric character and the right/left buttons to move between slots.
* **/ (File Browser)**: Allows you to browse the local filesystem. You can access the cache for previously launched Splore games (`splore`) or the cartridges located in your `/Roms/PICO/` directory.

Press <kbd>START</kbd> while selecting a game to access game options. From here you can:

* Run Cart
* Options (Sound and Fullscreen ON/OFF)
* Add or remove from favorites
* Show other carts by the cartridge creator
* Show similar carts

While in-game, press the <kbd>X</kbd> button to bring up the **cartridge menu**. From here you can:

* Continue play
* Add or remove from favorites
* Toggle options (volume, fullscreen, check button mappings)
* Reset the cart
* Exit back to Splore

You can also press <kbd>X</kbd> while in the Splore menu to exit Splore (or use the <kbd>SELECT</kbd>+<kbd>MENU</kbd> shortcut).

:::caution
Do not choose "*Exit to Console*" on the Splore menu or you'll be stuck there due to implementation limitations. If stuck, press <kbd>SELECT</kbd>+<kbd>MENU</kbd> to exit *Splore* and then reenter from the Onion *Apps* menu.
:::

## Shortcuts

The following shortcuts are provided by the Native Wrapper for your convenience while using Splore on your Miyoo Mini or Miyoo Mini Plus:


<table align="center">
    <thead>
        <tr>
            <th>Button combination</th>
            <th>Action</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><kbd>X</kbd></td>
            <td>Acts as the `Escape` button on Splore and during cartridge execution (brings up splore/cart menu)</td>
        </tr>
        <tr>
            <td><kbd>SELECT</kbd>+<kbd>LEFT</kbd>/<kbd>RIGHT</kbd></td>
            <td>Change or disable the current screen bezel (you can add new ones by placing your file in `/RApp/PICO-8/res/bezel` directory in your SD card)</td>
        </tr>
        <tr>
            <td><kbd>SELECT</kbd>+<kbd>UP</kbd>/<kbd>DOWN</kbd></td>
            <td>Raises/lowers the CPU clock rate in increments of 50 (you can override this value by editing the `/RApp/PICO-8/cfg/onioncfg.json` file)</td>
        </tr>
        <tr>
            <td><kbd>SELECT</kbd>+<kbd>L1</kbd></td>
            <td>Restart the current cart</td>
        </tr>
        <tr>
            <td><kbd>MENU</kbd></td>
            <td>Exit PICO-8 and go back to Onion</td>
        </tr>
        <tr>
            <td><kbd>L2</kbd></td>
            <td>Enable/Disable Mouse Mode</td>
        </tr>
    </tbody>
</table>

## Using Local Carts

In order to use local cartridge files, place them under the `/Roms/PICO/` directory. You should be able to launch these game files from the Expert menu as you would with any other emulator. When in Splore, browse to the local files section, and your cartridges should show up here. Games downloaded through Splore are stored in the `/Roms/PICO/splore/` subdirectory.

If you have multi-cart games, place all required cartridge files on the same directory (or a subdirectory to keep things cleaner) and follow the same procedure.
