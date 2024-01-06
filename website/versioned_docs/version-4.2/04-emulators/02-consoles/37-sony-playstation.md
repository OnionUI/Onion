---
slug: /emulators/psx
---

# Sony - PlayStation ✔

<img src="https://user-images.githubusercontent.com/44569252/188292823-4d971dd0-9c8a-4c99-a132-db16416e352a.png" align="right" width="240" />

- Alias: *PS*, *PS1*, *PSX*
- Emulator: **lr-pcsx-rearmed**
- Rom Folder: `PS`
- Extensions: `.chd` `.pbp` `.bin/.cue` `.img` `.mdf` `.toc` `.cbn` `.m3u` `.ccd` (must be lowercase)
- Bios: `PSXONPSP660.bin`, `scph101.bin`, `scph7001.bin`, `scph5501.bin`, `scph1001.bin`
- Recommended Romset: `"files for CHD-PSX-USA"`

Roms in `.bin` format _must_ have accompanying `.cue` files or they will not be displayed.  
Onion can generate `.cue` files automatically (`Apps` > `Tweaks` > `Tools` > `Generate Cue Files for PSX Games`).  
Alternatively, you can create `.cue` files using a free online tool such as [cue-maker](https://www.duckstation.org/cue-maker/).  

All Bios files are 512kb in size and are case sensitive, they must be named _exactly_ as shown above.  
Using the `PSXONPSP660.bin` bios is recommended for best compatibility.  
If you experience issues loading games, you can use [md5 checker](http://getmd5checker.com/) to verify your bios files against the md5sum provided in the [official core documentation](https://docs.libretro.com/library/pcsx_rearmed/).


https://www.youtube.com/watch?v=5DdSP1KxzSE


## Notes on performance

- Rewind and Fast Forward capability should be disabled while playing PSX as performance may suffer greatly. 

- PAL/EUR games run at 50fps, NTSC/USA/JP games run at 60fps. This is by design and as per original hardware.

- Some titles (e.g. Looney Tunes Sheep Rider, Jedi Power Battles, and 2xtreme/espn extreme games) need to have `SMC Checks` disabled or games will eventually slow down and crash. Go to RetroArch Quick Menu (<kbd>MENU</kbd>+<kbd>SELECT</kbd> while in-game), select `Options` and toggle on `(Speed Hack) Disable SMC Checks`.  
