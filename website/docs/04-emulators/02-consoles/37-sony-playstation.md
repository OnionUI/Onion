---
slug: /emulators/psx
---

# Sony - PlayStation ✔

<img src="https://user-images.githubusercontent.com/44569252/188292823-4d971dd0-9c8a-4c99-a132-db16416e352a.png" align="right" width="240" />

- Alias: *PS*, *PS1*, *PSX*
- Emulator: **lr-pcsx-rearmed**, km_duckswanstation_xtreme_amped, PCSX-ReARMed standalone
- Rom Folder: `PS`
- Extensions: `.chd` `.pbp` `.bin/.cue` `.img` `.mdf` `.toc` `.cbn` `.m3u` `.ccd` (must be lowercase)
- Bios: `PSXONPSP660.bin`, `scph101.bin`, `scph7001.bin`, `scph5501.bin`, `scph1001.bin`
- Recommended Romset: `"files for CHD-PSX-USA"`

Roms in `.bin` format _must_ have accompanying `.cue` files or they will not be displayed.  
Onion can generate `.cue` files automatically (`Apps` > `Tweaks` > `Tools` > `Generate Cue Files for PSX Games`).  
Alternatively, you can create `.cue` files using a free online tool such as [cue-maker](https://www.duckstation.org/cue-maker/).  

All Bios files are 512kb in size and are case sensitive, they must be named _exactly_ as shown above.  
Using the `PSXONPSP660.bin` bios is recommended for best compatibility.  
If you experience issues loading games, you can use [md5 checker](http://getmd5checker.com/) to verify your bios files against the md5sum provided in the [official core documentation](https://docs.libretro.com/library/pcsx_rearmed/#bios).


https://www.youtube.com/watch?v=5DdSP1KxzSE

:::note
Concerning multidisc games :
 Since Onion 4.3.0 Tweaks app contains a multidisc playlist (.m3u) generator so no need to create your m3u files manually or to convert your multidisc roms to `.pbp` anymore. More info [here](../faq#what-is-the-optimal-way-to-organize-multi-disk-games).
:::

## Notes on performance

- Most of PSX games run at 60 FPS natively on the Miyoo Mini. For most demanding games you'll find solutions here to run it at full speed.
- Rewind and Fast Forward capability should be disabled while playing PSX as performance may suffer greatly. 
- PAL/EUR games run at 50fps, NTSC/USA/JP games run at 60fps. This is by design and as per original hardware.
- Some titles (e.g. Looney Tunes Sheep Rider, Jedi Power Battles, and 2xtreme/espn extreme games) need to have `SMC Checks` disabled or games will eventually slow down and crash. Go to RetroArch Quick Menu (<kbd>MENU</kbd>+<kbd>SELECT</kbd> while in-game), select `Options` and toggle on `(Speed Hack) Disable SMC Checks`.  
- PCSX-ReARMed standalone offer better performances than the default RetroArch core (see below for details)
- overclocking is also an excellent way to improve PCSX-ReARMed performances on most demanding games. [More information in the FAQ](../faq#how-to-overclock-my-miyoo-mini). 

## Notes on PCSX-ReARMed standalone

PCSX-ReARMed standalone (not a RetroArch core but the classical emulator) offers much improved performances. It allows for using *enhanced resolution* in most games.
However its integration in Onion is not perfect : no automatic save states, no resume at boot , different shortcuts...

## Notes on Duckstation/SwanStation

Duckstation is a core from Expert section compiled by KMFD. It is slower than PCSX, however it allows to run some games like [Fromage](https://www.reddit.com/r/MiyooMini/comments/190avun/swanstation_fromage/). Could be interesting on rhythm games too.