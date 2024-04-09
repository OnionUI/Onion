---
slug: /emulators/superbroswar
---

# Super Mario War ⚠

<img
  title="Nintendo - DS"
  src={require('./assets/supermariowar.webp').default}
  style={{ width: '240px', float: 'right' }}
/>

- Emulator: **lr-km_superbroswar_libretro**, Super Bros War Standalone (from Ports repo)
- Rom Folder: `SUPERBROSWAR`
- Extensions: `.game`
- Bios: None


Super Mario War / Super Cat Wars is a fan-made multiplayer Super Mario Bros. Very customizable with many mods.
Onion includes the Retroarch core. The standalone version (which includes the multiplayer mode) will be distributed as a port in the Onion Ports repository.
The RetroArch core version allows you to fight against enemies controlled by the computer.

## 2 main assets:

### Super Mario War
- Download the last linux version from [the official repo](https://github.com/mmatyas/supermariowar/releases/tag/continuous), "supermariowar_2023-11-23_linux.zip" for example
- Extract the "data" folder contained in this archive on your SD card in the folder "Roms/SUPERBROSWAR/.data"
- Rename this "data" folder in "Super Mario War"
- Create an empty text file named "Super Mario War.game" in "Roms/SUPERBROSWAR" folder (it will be the shortcut which must have the same name as the folder in ".data")

### Super Cat Wars LITE
- Download the [last version](https://bot.libretro.com/assets/cores/Super%20Bros%20War/Super%20Cat%20Wars%20LITE.zip)
- Extract the "Super Cat Wars LITE" folder and its content in the folder "Roms/SUPERBROSWAR/.data"
- Move the file "Super Cat Wars LITE.game" from "Roms/SUPERBROSWAR/.data/Super Cat Wars LITE" folder to "Roms/SUPERBROSWAR" (it will be the shortcut which must have the same name as the folder in ".data")


## example

<table><td>

- 📁`Roms/SUPERBROSWAR/`
  - 📄`Super Cat Wars LITE.game`
  - 📄`Super Mario War.game`
  - 📁`.data/`
    - 📁`Super Cat Wars LITE/`
    - 📁`Super Mario War/`

</td></table>


:::note
The official website contains many maps and skins: http://smwstuff.net
:::

:::note
"Super Cat Wars LITE" contains more assets than "Super Mario War" which are all loaded in memory at start, so it can take some time to load and rarely might even crash due to lack of memory
:::

:::note
Only the standalone version includes the multiplayer mode (not tested)
:::
