---
sidebar_position: 1
slug: /emulators
---

# Emulators

![All Systems](https://user-images.githubusercontent.com/98862735/177056342-dbdd3c7f-30de-4669-945a-d5e5d330b7ab.png)

[Arcade Systems](#arcade-systems)
&nbsp;•&nbsp; [Consoles and PC Systems](#consoles-and-pc-systems)
&nbsp;•&nbsp; [Handheld Systems](#handheld-systems)
&nbsp;•&nbsp; [Add-ons and Peripherals](#add-ons-and-peripherals)
&nbsp;•&nbsp; [Miscellaneous](#miscellaneous)
&nbsp;•&nbsp; [Rom Folders - Quick Reference](#rom-folders---quick-reference)

*This page describes the supported emulators and rom formats for each system.*

## Important Information

### **Verified systems** are marked with ✔

<table><td><br/>

These are found in the "Verified" section of Package Manager, and will be installed in the `Games` tab.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>

### **Experimental systems** are marked with ⚠

<table><td><br/>

These are found in the "Expert" section of Package Manager and will be installed in the `Expert` tab (hidden by default). The `Expert` tab can be accessed by installing the `Expert` shortcut or by enabling it via `Apps` › `Tweaks` › `User interface` › `Show expert mode`.

> **Note:** These may require additional learning, files and configuration by the user to get working. Some may have performance or compatibility issues, please research and refer to the libretro core documentation.

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>

### Bios files

* Bios files are essential for some systems, should be named exactly as stated below and placed in the root `/BIOS` folder, unless otherwise indicated in a specific systems notes further below. Bios file names and extensions are case sensitive!

### Rom files

* The roms on the included Miyoo Mini SD card are a mixed bag which can lead to incompatibility with some emulators. It is highly recommended to source your own roms to get the most out of the device.
* Rom folder names are case sensitive and differ in Onion vs the stock Miyoo SD card, copy your roms from system to system if coming from stock (copying the ‘Roms’ folder wholesale from the Miyoo SD card will not work).
* Subfolders can be used inside your rom folders but ONLY one level deep (i.e. `Roms/GB/Hacks`). 
* Subfolders cannot have an apostrophe (`'`) in the name and browsing subfolders is not compatible with miyoogamelist.xml (all games will appear as one flat list). 
* If you add new roms to a system, highlight the system in the Games tab and press <kbd>SELECT</kbd> › `Refresh all roms`. 
* To add a game to your Favorites list, highlight the game in the game list and press <kbd>SELECT</kbd> › `Add to favorites`.

> The details provided below are for guidance and not every emulator included in Onion has been documented.


## Arcade Systems

![Arcade](https://user-images.githubusercontent.com/98862735/177056379-3680d239-5ee7-4db7-b15d-ea484dcbf93e.png)

Arcade emulation is a little more complex than your typical console emulation and includes MAME (*Multiple Arcade Machine Emulator*) along with CPS1/2/3 (*Capcom Play System*), SNK Neo Geo (released both as an Arcade cabinet and Home Console), FB (*Final Burn*) and Daphne.

While MAME supports many classic arcade games, for best compatibility and performance, it is recommended to use the CPS and Neo Geo systems (which use dedicated & customised emulators) for those particular games and MAME (or FB) for everything else.

Both MAME and FB require your romsets to be matched exactly to the emulator version so you will need to source the recommended romsets. MAME romsets are not compatible with FB cores (and vice versa) and not every game in a full romset will be playable but the majority will be.  

Arcade roms in general do not play well with save states (some will work but the majority don’t), you can [disable auto-loading of save states](faq#how-do-i-disable-auto-load-save-states) in the Retroarch menu (and save content or core overrides).  

MAME and FB both rely on internal databases to translate the rom file name into the displayed game name, some games may be missing from the db. You should never rename these roms, instead you can use the [miyoogamelist](faq#how-can-i-use-a-miyoogamelistxml-to-customise-game-names) functionality to give your arcade roms appropriate names.   

For more detailed information and tips on Arcade emulation, checkout the awesome [Ultimate Miyoo Mini Arcade Guide for Onion](https://www.reddit.com/r/MiyooMini/comments/vfirs8/ultimate_miyoo_mini_arcade_guide_onion_os/) by lordelan.


### **Arcade** (Default) ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292473-5ce7149e-2d90-441f-9c17-fc259c674387.png" align="right" width="240" />

> _Alternative emulators_ available in Expert
- Emulator: **lr-mame2003plus** 
- Required rom set version: `MAME 2003-Plus Reference: Full Non-Merged Romsets`
- Rom Folder: `ARCADE` 
- Extensions: `.zip`
- Bios: `pgm.zip` (for PGM games only like *Knights of Valour* and *DoDonPachi*)
- Samples: Audio Samples for Mame2003Plus should be placed in the `/BIOS/mame2003-plus/samples` folder

> **Notes**
> 
> MAME does not play well with save states this is a core issue and cannot be fixed. It is recommended to disable 'auto-loading of save states' in the Retroarch menu (and save core overrides).
>
> An onscreen message saying ‘This Game Will Not Work’ is MAME letting you know that the game is not emulated in the MAME version and you will not find a compatible/playable rom for that game.
>
> Current/modern versions of MAME are not compatible with the Miyoo Minis limited hardware specification which is why MAME 2003Plus has been chosen as the default.  
>
> Check out the [MAME overview video](https://www.youtube.com/watch?v=-0riylHkJis) by *RetroBreeze*.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Arcade (Alternatives) ⚠

<table><td><br/>

- Emulator: lr-fbalpha2012, lr-fbneo, lr-mame2000, lr-mame2003-extreme
- Rom Folder: `FBA2012`, `FBNEO`, `MAME2000`, `ARCADE` (mame2003-extreme) 
- FBA2012 Required ROM Version: `"FBA Arcade Set v0.2.97.29 for FBA 2012 libretro core"` (use quotes when searching, to find an exact match)
- FBNEO Required ROM Version: FinalBurn Neo is compatible with FinalBurn Neo latest ROM sets.
- Extensions: `.zip` `.7z`

> **Notes**
> 
> The FB cores are a good alternative to MAME, they may not run as many games but you may find _some_ games are more accurate in FB. 
>
> FBNEO is the latest FB core and as such it has higher hardware expectations (beyond the mini for some games). While many games will run fine in FBNEO, you may see better performance (at the cost of accuracy) in FBA2012 which performs better on lower powered devices.  
>
> For best game compatibility, seek out the recommended romsets above. Alternative romsets for Final Burn cores may be largely compatible (i.e. v0.2.97.24, v0.2.97.39, v0.2.97.40, v0.2.97.44).

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Capcom - CP System** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190531524-bdbbde6d-2209-4bc5-afde-314041186468.png" align="right" width="200" />

- Alias: *CPS-1*
- Emulator: **fbalpha2012_cps1_libretro**
- Required ROM Version: `"FBA Arcade Set v0.2.97.29 for FBA 2012 libretro core"` (use quotes when searching, to find an exact match)
- Rom Folder: `CPS1`
- Extensions: `.zip` `.7z` `.bin/.cue`
- Bios: None
> For best game compatibility, seek out the recommended romset above. Alternative romsets for Final Burn cores may be largely compatible (i.e. v0.2.97.24, v0.2.97.39, v0.2.97.40, v0.2.97.44).

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Capcom - CP System II** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190531654-26fb22b4-27d1-4096-b607-55eb4b2d9797.png" align="right" width="200" />

- Alias: *CPS-2*
- Emulator: **fbalpha2012_cps2_libretro**
- Required ROM Version: `"FBA Arcade Set v0.2.97.29 for FBA 2012 libretro core"` (use quotes when searching, to find an exact match)
- Rom Folder: `CPS2`
- Extensions: `.zip` `.7z` `.bin/.cue`
- Bios: None
> For best game compatibility, seek out the recommended romset above. Alternative romsets for Final Burn cores may be largely compatible (i.e. v0.2.97.24, v0.2.97.39, v0.2.97.40, v0.2.97.44).

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Capcom - CP System III** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190531748-f97734d3-96d2-45be-bf65-851bce509b77.png" align="right" width="200" />

- Alias: *CPS-3*
- Emulator: **fbalpha2012_cps3_libretro**
- Required ROM Version: `"FBA Arcade Set v0.2.97.29 for FBA 2012 libretro core"` (use quotes when searching, to find an exact match)
- Rom Folder: `CPS3`
- Extensions: `.zip` `.7z` `.bin/.cue`
- Bios: None
> For best game compatibility, seek out the recommended romset above. Alternative romsets for Final Burn cores may be largely compatible (i.e. v0.2.97.24, v0.2.97.39, v0.2.97.40, v0.2.97.44).

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Daphne ⚠

<table><td><br/>

- Emulator: **daphne_libretro.so**
- Rom Folder: `DAPHNE`  
> **Notes**  
> Check out the [Daphne overview video](https://www.youtube.com/watch?v=aFtvL267D6w) by *RetroBreeze*.
> For more information please visit https://github.com/libretro/daphne.

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


## Consoles and PC Systems

![Consoles](https://user-images.githubusercontent.com/98862735/177056397-e0f7ec2d-adbd-4862-9361-ac2df58ce8d8.png)

### **Amstrad CPC** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190533717-0031945c-eb8b-4aa8-871f-3838de42e562.png" align="right" width="180" />

- Emulator: **lr-crocods**
- Rom Folder: `CPC`
- Extensions: `.sna` `.dsk` `.kcr`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Atari 800 ⚠

<table><td><br/>

- Emulator: **lr-atari800**
- Rom Folder: `EIGHTHUNDRED`
- Extensions: `.atr` `.zip` `.7z`
- Bios: `ATARIOSA.ROM` and `ATARIOSB.ROM` and `ATARIBAS.ROM`

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


   
### **Atari 2600** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292524-bbaa61be-9e75-448a-9980-72468c45e64b.png" align="right" width="240" />

- Alias: Atari Video Computer System (Atari VCS)
- Emulator: **lr-stella2014**
- Rom Folder: `ATARI`
- Extensions: `.a26` `.bin` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Atari 5200** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292538-1aa6d1c9-f615-432b-93b2-c2fc6f5b6984.png" align="right" width="240" />

- Alias: *Atari 5200 SuperSystem*
- Emulator: **lr-atari800**
- Rom Folder: `FIFTYTWOHUNDRED`
- Extensions: `.a52` `.zip` `.7z` `.bin`
- Bios: `5200.rom` and `ATARIBAS.ROM`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Atari 7800** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292545-4f0b8e59-69be-4ea3-8079-00ff418fcaa2.png" align="right" width="240" />

- Alias: *Atari 7800 ProSystem*
- Emulator: **lr-prosystem**
- Rom Folder: `SEVENTYEIGHTHUNDRED`
- Extensions: `.a78` `.zip`
- Bios: `7800 BIOS (U).rom`
> **Notes**  
> Roms must contain headers or they will not function. For an explanation of headers and why they are important, see this [comment](https://github.com/OpenEmu/OpenEmu/issues/4049#issuecomment-560232690)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Atari Jaguar ⚠

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292553-6bc03fc5-d4de-4b8d-b546-df5852a9bfa7.png" align="right" width="240" />

- Emulator: **Virtual Jaguar**
- Rom Folder: `JAGUAR`
- Extensions: `.j64` `.jag` `.rom` `.abs` `.cof` `.bin` `.prg` (must be lowercase)
- Bios: `virtualjaguar_bios`
> **Note:** *Slow to emulate*

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Atari ST/STE/TT/Falcon ⚠

<table><td><br/>

- Emulator: **hatari**
- Rom Folder: `ATARIST`
- Extensions: `.st` `.msa` `.zip` `.stx` `.dim` `.ipf` (must be lowercase)
- Bios: `tos.img`
> _The plain ST mode only works with TOS 1.00, 1.02, 1.04, or 2.06. STE mode requires any of the TOS versions 1.xx or 2.xx. TOS 3.0x is for TT, and TOS 4.0x is for Falcon._  
> For more information please review [the official core documentation](https://docs.libretro.com/library/hatari/)  

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Coleco - ColecoVision** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292566-4d493846-209b-4515-a1c9-3c440d27129e.png" align="right" width="240" />

- Alias: *CBS ColecoVision*
- Emulator: **lr-bluemsx**
- Rom Folder: `COLECO`
- Extensions: `.rom` `.ri` `.mx1` `.mx2` `.col` `.dsk` `.cas` `.sg` `.sc` `.m3u` `.zip` `.7z`
- Bios: `coleco.rom` (Verified working MD5: `2C66F5911E5B42B8EBE113403548EEE7`)

> **Notes**
> 
> The blueMSX core requires the `Databases` and `Machines` folders from a full installation of blueMSX ([more info here](https://docs.libretro.com/library/bluemsx/#bios)) to be placed inside the `BIOS` folder.
> 
> Check out the [ColecoVision overview video](https://www.youtube.com/watch?v=tDtrmM-hLno) by *RetroBreeze*.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Commodore 64/VIC-20/PET** ✔

<table><td><br/>

- Emulator: lr-vice\_x64
- Rom Folder: `COMMODORE`
- Extensons: `.d64` `.zip` `.7z` `.t64` `.crt` `.prg` `.nib` `.tap`
- Bios: None

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Commodore - Amiga & Amiga CD32**  ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292584-cdc85cef-c384-4134-91c0-1e87d3360d76.png" align="right" width="240" />

- Alias: Amiga CD32
- Emulator: **lr-puae**, lr-uae4arm
- Rom Folder: `AMIGA`
- Extensions: `.adf` `.hdf` `.lha` `.zip` (For CD32: `.bin/.cue` `.iso`)
- Bios: `kick33180.A500`, `kick34005.A500`, `kick34005.CDTV`, `kick37175.A500`, `kick37350.A600`, `kick39106.A1200`, `kick39106.A4000`, `kick40060.CD32`, `kick40060.CD32.ext`, `kick40063.A600`, `kick40068.A1200`, and `kick40068.A4000`
> **Notes**  
> For maximum compatibility add ALL above ‘kickstart roms’ to the `/BIOS` folder and ensure they are named exactly as detailed (lowercase).
> See [this link](https://docs.libretro.com/library/puae/) for more information.
> - <kbd>SELECT</kbd> toggles the onscreen keyboard, <kbd>L</kbd> & <kbd>R</kbd> are mapped to the mouse buttons.
> - This emulator will play Amiga CD32 games though some perform better than others.
> - Setting CPU speed to -700 (negative 700) in the Retroarch menu may improve A1200/CD32 performance.
> - Other Core Settings that might be useful for some games are frameskip set to 1, or setting off the blitter wait in Video options.
> - Some games will have stuttering audio or behave very slowly regardless of the CPU speed, frameskip etc. Many of these games will work fine if you find another format for them. .hdf (hard disk files) are the most problematic. .adf or .lha games usually work better although you might have to open RA options to switch or add floppy disks when required. If you find a game that won't run correctly no matter the options you set for it, simply try to find another version in another format.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Commodore - Amiga CD32 (uae4arm) ⚠

<table><td><br/>

- Emulator: lr-uae4arm
- Rom Folder: `AMIGACD`
- Extensions: `.bin/.cue` `.ccd` `.lha` `.nrg` `.mds` `.iso` `.m3u` `.chd`
- Bios: `kick33180.A500` and `kick34005.A500` and `kick40068.A1200`  
> See [this link](https://github.com/midwan/amiberry/wiki/Kickstart-ROMs-(BIOS)) for more details.

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Fairchild Channel F** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292604-695144e6-ed90-4b7e-a3a8-e29e59dfb714.png" align="right" width="240" />

- Alias: *Fairchild Video Entertainment System*
- Emulator: **lr-freechaf**
- Rom Folder: `FAIRCHILD`
- Extensions: `.bin` `.rom` `.chf` `.zip`
- Bios: `sl31253.bin` and `sl31254.bin` and `sl90025.bin`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **General Consumer Electronics - Vectrex** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292611-667e2986-7349-41f0-97af-dee83e0e08ba.png" align="right" width="240" />

- Emulator: **lr-vecx**
- Rom Folder: `VECTREX`
- Extensions: `.vec` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Magnavox Odyssey 2** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292626-f4dc1a87-e631-4151-ad56-16dead2f38ca.png" align="right" width="240" />

- Alias: *Philips Odyssey 2*, *Philips Videopac G7000*, *Philips Odyssey*, *Odyssey2*
- Emulator: **lr-o2em**
- Rom Folder: `ODYSSEY`
- Extensions: `.bin` `.zip` `.7z`
- Bios: `o2rom.bin`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Mattel - Intellivision** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190532224-87f7dc15-e0df-4d6e-9c3f-8fa8b836e8d4.png" align="right" width="240" />

- Emulator: **lr-freeintv**
- Rom Folder: `INTELLIVISION`
- Extensions: `.bin` `.int` `.zip` `.7z`
- Bios: `exec.bin`, `grom.bin`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Microsoft - MS-DOS** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292649-170f9e97-c656-4321-bbcb-e4a9a3bc9391.png" align="right" width="120" />

- Alias: *Microsoft DOS*
- Emulator: **lr-dosbox\_pure**
- Rom Folder: `DOS`
- Extensions: `.zip` `.dosz` `.exe` `.com` `.bat` `.iso` `.bin/.cue` `.ins` `.img` `.ima` `.vhd` `.jrc` `.tc` `.m3u` `.m3u8` `.conf`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **NEC - PC Engine SuperGrafx** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292668-4aa840eb-2005-45b7-bd0d-66265398ee61.png" align="right" width="180" />

- Alias: *SuperGrafx*
- Emulator: **lr-mednafen-supergrafx**
- Rom Folder: `SGFX`
- Extensions: `.pce` `.sgx` `.bin/.cue` `.ccd` `.chd` `.zip` `.7z`
- Bios: `syscard3.pce`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### NEC - PC-8000 & PC-8800 series ⚠

<table><td><br/>

- Emulator: quasi88
- Rom Folder: `PCEIGHTYEIGHT`
- Extensions: `.d88` `.fdi` `.hdi` `.zip`
- Bios: None

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### NEC - PC-98 ⚠

<table><td><br/>

- Emulator: lr-nekop2
- Rom Folder: `PCNINETYEIGHT`
- Extensions: `.d98` `.fdi` `.hdi` `.zip`  
  (see link below for complete list of supported extensions)
- Bios: See the following link for details regarding the required bios files (and core configuration). [https://docs.libretro.com/library/neko_project_ii_kai/#bios](https://docs.libretro.com/library/neko_project_ii_kai/#bios)
- IMPORTANT: Contrary to the link, the Bios files must be placed in `BIOS/np2` NOT in `BIOS/np2kai`

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### NEC - PC-FX ⚠

<table><td><br/>

- Emulator: lr-mednafen-pcfx
- Rom Folder: `PCFX`
- Extensions: `.chd` `.zip` `.bin/.cue` `.ccd` `.toc`
- Bios: `pcfx.rom`

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **NEC - TurboGrafx-16** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292695-08c9ac3c-b206-4ea5-aaba-c60683d4b4a0.png" align="right" width="90" />

- Alias: *PC Engine*
- Emulator: **lr-mednafen-pce-fast**
- Rom Folder: `PCE`
- Extensions: `.pce` `.chd` `.zip` `.7z` `.ccd` `.iso` `.img` `.bin/.cue`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo Entertainment System (NES)** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292739-d197122a-65b4-4946-b5b2-0b28fde498fc.png" align="right" width="240" />

- Alias: *Famicom*
- Emulator: **lr-fceumm**, lr-nestopia, lr-quicknes
- Rom Folder: `FC`
- Extensions: `.nes` `.zip` `.7z`
- Bios: None
> **Notes**  
> Roms must contain headers or they will not function. For an explanation of headers and why they are important, see this [comment](https://github.com/OpenEmu/OpenEmu/issues/4049#issuecomment-560232690)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Super Nintendo Entertainment System (SNES)** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292752-1876f4d6-62bb-40d2-afea-e7bdfd64f1f1.png" align="right" width="240" />

- Alias: *Super Nintendo*, *Super Famicom* (SFC)
- Emulator: **lr-mednafen-supafaust**, snes9x_libretro.so, lr-snes9x2005PLUS lr-snes9x2010, lr-snes9x2002, lr-snes9x2005, lr-snes9x2010 
- Rom Folder: `SFC`
- Extensions: `.sfc` `.smc` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Virtual Boy** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292767-6d3f308b-4c47-4582-b30f-7bb932ebfc85.png" align="right" width="240" />

- Emulator: **lr-mednafen-vb**
- Rom Folder: `VB`
- Extensions: `.vb` `.vboy` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Panasonic - 3DO Interactive Multiplayer ⚠

<table><td><br/>

- Alias: *3DO System*, *3DO*
- Emulator: **lr-opera**
- Rom Folder: `PANASONIC`
- Extensions: `.iso` `.chd` `.bin/.cue`
- Bios: `panafz1.bin` or `panafz10.bin` or `panafz10-norsa.bin` or `panafz10e-anvil.bin` or `panafz10e-anvil-norsa.bin` or `panafz1j.bin` or `panafz1j-norsa.bin` or `goldstar.bin` or `sanyotry.bin` or `3do_arcade_saot.bin`  
  See [this link](https://docs.libretro.com/library/opera/#bios) for more details.
> **Notes**  
> This will never run full speed!

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Philips Videopac+ G7400** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190532381-8df47ac6-7f5c-4a70-b943-a4c87e485e82.png" align="right" width="240" />

- Emulator: **lr-o2em**
- Rom Folder: `VIDEOPAC`
- Extensions: `.bin` `.zip` `.7z`
- Bios: `o2rom.bin`, `g7400.bin` (maybe, `c52.bin` and `jopac.bin`)

> **Notes**  
> Games made for 7000 and 7200 machines require bios files.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sega Genesis** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292779-d226f97d-0e91-4f55-b602-6d95144c7180.png" align="right" width="240" />

- Alias: *Mega Drive*
- Emulator: **lr-picodrive**, lr-genesis\_plus\_gx
- Rom Folder: `MD`
- Extensions: `.68k` `.mdx` `.md` `.sgd` `.smd` `.gen` `.bin` `.zip` `.7z`
- Bios: `bios_MD.bin` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sega - Master System** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292790-f4742672-c1b9-483d-8ae8-01ff9288c739.png" align="right" width="240" />

- Emulator: **lr-picodrive**, lr-genesis\_plus\_gx, lr-gearsystem
- Rom Folder: `MS`
- Extensions: `.7z` `.bin` `.sms` `.zip` (must be lowercase)
- Bios: `bios_E.sms` (optional), `bios_U.sms` (optional), `bios_J.sms` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sega - SG 1000** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190533057-937cc002-dfe9-44bc-a8e8-7da8fe46fe12.png" align="right" width="240" />


- Emulator: **Gearsystem**
- Rom Folder: `SEGASGONE`
- Extensions: `.sms` `.gg` `.sg` `.mv` `.bin` `.rom` (must be lowercase)
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Sharp X1 ⚠

<table><td><br/>

- Emulator: lr-x1
- Rom Folder: `XONE`
- Extensions: `.dx1` `.zip` `.2d` `.2hd` `.tfd` `.d88` `.88d` `.hdm` `.xdf` `.dup` `.cmd`
- Bios: `IPLROM.X1`, `IPLROM.X1T` (need to be placed in a folder named `xmil` within the `BIOS` folder)

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### Sharp X68000 ⚠

<table><td><br/>

- Emulator: lr-px68k
- Rom Folder: `X68000`
- Extensions: `.dim` `.m3u`
- Bios: `iplrom.dat`, `cgrom.dat`, `iplrom30.dat` (optional), `iplromco.dat` (optional), `iplromxv.dat` (optional) (need to be placed in a folder named `keropi` within the `BIOS` folder)

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Sinclair - ZX Spectrum** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292803-9a311ecd-c000-4bc9-9c1b-8770ef076a32.png" align="right" width="240" />

- Emulator: **lr-fuse**
- Rom Folder: `ZXS`
- Extensions: `.sna` `.szx` `.z80` `.tap` `.tzx` `.gz` `.udi` `.mgt` `.img` `.trd` `.scl` `.dsk`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Sinclair - ZX-81 ⚠

<table><td><br/>

- Emulator: lr-81
- Rom Folder: `ZXEIGHTYONE`
- Extensions: `.p` `.tzx` `.zip`
- Bios: None

> **Notes**
> 
> I was only able to successfully load `.p` based roms. I suggest using `.p` roms and `.zip` files with `.p` roms in them based on my testing.
>
> Many games can be started by pressing <kbd>SELECT</kbd> to bring up the virtual keyboard, press <kbd>R</kbd> then <kbd>RETURN</kbd>. Otherwise, you'll need to search online on how to load these games if you're not familiar with this system.

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **SNK - Neo Geo** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292808-3addb46b-9939-4461-bc17-c7323911900f.png" align="right" width="220" />

- Emulator: **lr_fbalpha2012_neogeo**
- Required ROM Version: `"FBA Arcade Set v0.2.97.29 for FBA 2012 libretro"` (search with quotes for exact match)
- Alternative ROM Pack: `"Neo-Geo Rom Collection By Ghostware"` (search with quotes for exact match)
- Rom Folder: `NEOGEO`
- Extensions: `.zip` (must be lowercase)
- Bios: `neogeo.zip`

> **Notes**
> 
> - UniBIOS can be used but v4.0 can lead to missing or corrupted audio when used with save states. Earlier UniBIOS version are OK. 
> - Because Neo Geo roms can come in different formats (split or non-merged), it's recommended to keep the 'neogeo.zip' bios in both the `/BIOS` folder and the `/Roms/NEOGEO` folder to ensure best compatibility.
> - For best game compatibility, seek out the recommended romset above. Alternative romsets for Final Burn cores may be largely compatible (i.e. v0.2.97.24, v0.2.97.39, v0.2.97.40, v0.2.97.44).
> 
> Check out the [NeoGeo overview video](https://www.youtube.com/watch?v=CGKX6yPG2nE) by *RetroBreeze*.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **SNK - Neo Geo CD** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190532551-7b1c2f3e-9d48-4dd9-8ccb-2b10cfb6ce26.png" align="right" width="240" />

- Emulator: **lr-neocd**
- Rom Folder: `NEOCD`
- Extensions: `.bin/.cue` `.chd` `.m3u`
- Bios: (`000-lo.lo` or `ng-lo.rom`) and (`neocd_f.rom` or `neocd.bin` or `uni-bioscd.rom`) placed in a folder named neocd within the bios folder

> **Notes**  
> UniBIOS can be used but v4.0 can lead to missing or corrupted audio when used with save states. Earlier UniBIOS version are OK.  
> Because Neo Geo roms can come in different formats (split or non-merged), it's recommended to keep the 'neogeo.zip' bios in both the `/BIOS` folder and the `/Roms/NEOGEO` folder to ensure best compatibility.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sony - PlayStation** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292823-4d971dd0-9c8a-4c99-a132-db16416e352a.png" align="right" width="240" />

- Alias: *PS*, *PS1*, *PSX*
- Emulator: **lr-pcsx-rearmed**
- Rom Folder: `PS`
- Extensions: `.chd` `.pbp` `.bin/.cue` `.img` `.mdf` `.toc` `.cbn` `.m3u` `.ccd` (must be lowercase)
- Bios: `PSXONPSP660.bin`, `scph101.bin`, `scph7001.bin`, `scph5501.bin`, `scph1001.bin`
- Recommended Romset: `"files for CHD-PSX-USA"`

> **Important:**  
> Roms in `.bin` format _must_ have accompanying `.cue` files or they will not be displayed.  
> Onion can generate `.cue` files automatically (`Apps` > `Tweaks` > `Tools` > `Generate Cue Files for PSX Games`).  
> Alternatively, you can create `.cue` files using a free online tool such as [cue-maker](https://www.duckstation.org/cue-maker/).  

> All Bios files are 512kb in size and are case sensitive, they must be named _exactly_ as shown above.  
> Using the `PSXONPSP660.bin` bios is recommended for best compatibility.  
> If you experience issues loading games, you can use [md5 checker](http://getmd5checker.com/) to verify your bios files against the md5sum provided in the [official core documentation](https://docs.libretro.com/library/pcsx_rearmed/).

> **Notes**  
> Rewind and Fast Forward capability should be disabled while playing PSX as performance may suffer greatly. 
> PAL/EUR games run at 50fps, NTSC/USA/JP games run at 60fps. This is by design and as per original hardware.   
> Some titles (e.g. Looney Tunes Sheep Rider, Jedi Power Battles, and 2xtreme/espn extreme games) need to have `SMC Checks` disabled or games will eventually slow down and crash. Go to RetroArch Quick Menu (<kbd>MENU</kbd>+<kbd>SELECT</kbd> while in-game), select `Options` and toggle on `(Speed Hack) Disable SMC Checks`.  
>  
> Check out the [PlayStation overview video](https://www.youtube.com/watch?v=5DdSP1KxzSE) by *RetroBreeze*.  


<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Uzebox ⚠

<table><td><br/>

- Emulator: lr-uzem
- Rom Folder: `UZEBOX`
- Extensions: `.uze`
- Bios: None

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


## Handheld Systems

![Handhelds](https://user-images.githubusercontent.com/98862735/177056405-7ccd6fb5-0a49-4ec8-b8da-5dcb280c17f4.png)

### **Atari Lynx** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292836-164db9f8-4685-4366-9f5c-b075cf913abf.png" align="right" width="240" />

- Emulator: **lr-handy**, lr-mednafen\_lynx
- Rom Folder: `LYNX`
- Extensions: `.lnx` `.zip`
- Bios: `lynxboot.img`
> **Notes**  
> Roms must contain headers or they will not function. For an explanation of headers and why they are important, see this [comment](https://github.com/OpenEmu/OpenEmu/issues/4049#issuecomment-560232690)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Bandai - WonderSwan / Color** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190534777-2a54de18-75ac-47c6-8b91-e6659c07fbda.png" align="right" width="160" />

- Emulator: **lr-mednafen-wswan**
- Rom Folder: `WS`
- Extensions: `.ws` `.pc2` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Game & Watch** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190534852-ebac7553-d1a2-46e2-9d43-44a347da82be.png" align="right" width="140" />


- Emulator: **lr-gw**
- Rom Folder: `GW`
- Extensions: `.mgw` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Game Boy** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292860-7a8d651b-77f7-4b91-bc63-528cdf8580e8.png" align="right" width="240" />

- Emulator: **lr-gambatte**, lr-gearboy, lr-tgbdual
- Rom Folder: `GB`
- Extensions: `.gb` `.gbc` `.dmg` `.zip` `.7z`
- Bios: `gb_bios.bin` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Game Boy Advance** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292863-3858a9b9-b26b-418f-9256-7ed41d3dcee6.png" align="right" width="320" />

- Emulator: **lr-gpsp**, lr-mgba, lr-meteor, lr-mednafen-gba, lr-vba\_next
- Rom Folder: `GBA`
- Extensions: `.bin` `.gba` `.zip` `.7z`
- Bios: `gba_bios.bin` (required for lr-gpsp, optional for other cores), `gb_bios.bin` (optional), `gbc_bios.bin` (optional), `sgb_bios.bin` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Game Boy Color** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292875-63b0a423-1268-46c3-87ed-d5829fffe9df.png" align="right" width="320" />

- Emulator: **lr-gambatte**, lr-gearboy, lr-tgbdual
- Rom Folder: `GBC`
- Extensions: `.gb` `.gbc` `.dmg` `.zip` `.7z`
- Bios: `gbc_bios.bin` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Pokemon Mini** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190536785-400df06d-2928-461d-8c90-870b5eebc0d0.png" align="right" width="240" />

- Emulator: **lr-pokemini**
- Rom Folder: `POKE`
- Extensions: `.min` `.zip`
- Bios: `bios.min` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sega - Game Gear** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292895-f3f44133-c028-4bbf-804e-9c2a9383cb68.png" align="right" width="240" />

- Emulator: **lr-picodrive**, lr-genesis\_plus\_gx, lr-gearsystem
- Rom Folder: `GG`
- Extensions: `.bin` `.gg` `.zip` `.7z`
- Bios: `bios.gg` (optional)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **SNK - Neo Geo Pocket / Color** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292915-121919d6-665e-499c-a3e8-451e31d1f54d.png" align="right" width="240" />

- Emulator: **lr-mednafen-ngp**
- Rom Folder: `NGP`
- Extensions: `.ngp` `.ngc` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Welback Holdings - Mega Duck WG-108** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190534953-dbf5704a-1991-4739-9edf-52ae71ece118.png" align="right" width="140" />

- Alias: *Cougar Boy*
- Emulator: **lr-sameduck**
- Rom Folder: `MEGADUCK`
- Extensions: `.bin` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Watara Supervision** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190535299-423aae6f-7af9-4418-ab13-82881b3c2996.png" align="right" width="240" />


- Alias: *QuickShot Supervision*
- Emulator: **lr-potator**
- Rom Folder: `SUPERVISION`
- Extensions: `.sv` `.bin` `.zip` `.7z`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


## Add-ons and Peripherals

![ADD-ONS-PERIPHERALS](https://user-images.githubusercontent.com/98862735/188319678-b0e38782-ecff-4ac0-b9e8-3ed025c59d1d.png)

### **Bandai - SuFami Turbo** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292933-ef785db5-487f-4a04-9f2a-2c411c080947.png" align="right" width="240" />

- Emulator: **lr-snes9x**
- Rom Folder: `SUFAMI`
- Extensions: `.smc` `.zip` `.7z`
- Bios: `STBIOS.bin`

> **Notes**  
> For multi-cart Sufami Turbo games, you must first run each game individually to create sram files for them. Then the multi-link will function correctly. See Libretro’s documentation for more info.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### Dreamcast - VMU ⚠

<table><td><br/>

- Alias: *Visual Memory Unit*, *Visual Memory System* (VMS)
- Emulator: lr-vemulator
- Rom Folder: `VMU`
- Extensions: `.vms` `.bin`
- Bios: None
> **Notes** (Credit: dwmccoy)  
> A pixelated screen displays on launch. Open Retroarch (<kbd>MENU</kbd>+<kbd>SELECT</kbd>) and choose `Close Content`. Now select `History` and load the game file you just closed. Choose Run. The game should now display correctly but plays too fast. To fix this go back into Retroarch and set `Automatic Frame Delay` to ON in the Latency Menu.

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **NEC - TurboGrafx CD** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190537560-2028c37f-3631-49fd-93c4-c83ea088fad7.png" align="right" width="240" />

- Alias: *PC Engine CD*
- Emulator: **lr-mednafen-pce-fast**
- Rom Folder: `PCECD`
- Extensions: `.pce` `.chd` `.ccd` `.iso` `.img` `.bin/.cue`
- Bios: `syscard3.pce`

> **Notes:**  
> This core does not support compressed files (`.zip` or `.7z`), if you find your games are stuck   
> at a `JUST A MOMENT...` loading screen, ensure that you roms (and bios files) are not zipped.   
> For more information relating to this core please review the [official core documentation](https://docs.libretro.com/library/beetle_pce_fast/).   

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Family Computer Disk System** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/98862735/190536969-513ff538-d926-4046-9d79-1754236190f8.png" align="right" width="240" />

- Alias: *Famicom Disk System*
- Emulator: **lr-fceumm**
- Rom Folder: `FDS`
- Extensions: `.nes` `.unif` `.unf` `.fds` `.zip` `.7z`
- Bios: `disksys.rom`
> **Notes**  
> Roms must contain headers or they will not function. For an explanation of headers and why they are important, see this [comment](https://github.com/OpenEmu/OpenEmu/issues/4049#issuecomment-560232690)

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Satellaview** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292961-e4237efb-a26b-45a9-9db0-ea663d86d7a3.png" align="right" width="240" />

- Emulator: **lr-snes9x**
- Rom Folder: `SATELLAVIEW`
- Extensions: `.bs` `.sfc` `.smc` `.zip` `.7z`
- Bios: `BS-X.bin`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Nintendo - Super Game Boy** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292984-db24944a-ea67-45ab-a032-72a7dedd3f08.png" align="right" width="240" />

- Emulator: **lr-mgba**
- Rom Folder: `SGB`
- Extensions: `.gb` `.gbc` `.dmg` `.zip` `.7z`
- Bios: `sgb_bios.bin`

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sega - 32X** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188292997-5f882455-f6b0-448d-9223-330149de4120.png" align="right" width="240" />

- Alias: *Super 32X*, *Genesis 32X*, *Mega 32X*, *Mega Drive 32X*
- Emulator: **lr-picodrive**
- Rom Folder: `THIRTYTWOX`
- Extensions: `.32x` `.7z` `.bin` `.md` `.smd` `.zip`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **Sega CD** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188293014-3e96ba89-328f-4022-bd7e-31c09d613f90.png" align="right" width="240" />

- Alias: *Mega-CD*
- Emulator: **lr-picodrive**, lr-genesis\_plus\_gx
- Rom Folder: `SEGACD`
- Extensions: `.chd` `.bin/.cue` `.iso`
- Bios: `bios_CD_U.bin`, `bios_CD_E.bin`, `bios_CD_J.bin`

> **Notes**  
> You can find a video tutorial by RetroBreeze [here](https://www.youtube.com/watch?v=mSuJHu43LB0)  
> Review the [official core documentation](https://docs.libretro.com/library/picodrive/) for more information about this emulator.  

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


## Miscellaneous

![Miscellaneous](https://user-images.githubusercontent.com/98862735/188297284-06920728-6a26-414f-832c-2a8f196576e4.png)

### **ASCII Corporation / Microsoft - MSX** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188293033-01d4c7e1-cf05-408a-8f2d-a29497c07064.png" align="right" width="240" />

- Alias: *MSX2*
- Emulator: **lr-bluemsx**, lr-fmsx
- Rom Folder: `MSX`
- Extensions: `.cas` `.dsk` `.mx1` `.mx2` `.rom` `.zip` `.7z`

> **Notes**  
> The blueMSX core requires the `Databases` and `Machines` folders from a full installation of blueMSX ([more info here](https://docs.libretro.com/library/bluemsx/#bios)) to be placed inside the `BIOS` folder.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### EasyRPG ⚠

<table><td><br/>

- Emulator: lr-easyrpg
- Rom Folder: `EASYRPG`
- Extensions: `.easyrpg`
- Bios: None
> **Notes**  
> Games must have a `RPG_RT.ini` and `RPG_RT.ldb` inside their respective folders.

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### OpenBOR ⚠

<table><td><br/>

- Emulator: OpenBOR Standalone
- Rom Folder: `OPENBOR`
- Extensions: `.pak`
- Bios: None

> **Notes:**  
> Larger game paks (i.e. Marvel Infinity) may take 2-3 minutes to load, and up to a minute to close, be patient.   
> Save states are not currently supported but in game saving and loading works well.  

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **Pico-8** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188293050-691f7376-544e-4275-a612-bb042576dbe3.png" align="right" width="240" />

- Emulator: **fake-08**
- Rom Folder: `PICO`
- Extensions: `.p8` `.png`
- Bios: None

> **Notes**  
> - Many games will play fine, compatibility is not perfect but is improving.   
> - This emulator does not currently support in-game saves or save states.  
> - For multi-cart games (such as `POOM`), place all 'cart' game files in `Roms/PICO` (you may use subfolders, i.e. `Roms/PICO/POOM`) and launch the game from the first file (i.e. `poom_0.p8`).  
> - To download games from the [Lexaloffle BBS](https://www.lexaloffle.com/bbs/?cat=7#sub=2&mode=carts&orderby=featured), right-click the <img src="https://user-images.githubusercontent.com/98862735/190835494-ec611ceb-5ec1-4b96-924d-7ec969fc83e1.png"  align="center"  width="52" /> 
button in the bottom left corner of the play window and choose ‘save link as’.  
> - Check out the [Pico-8 overview video](https://www.youtube.com/watch?v=ZGd5vmwnAPA) by *RetroBreeze*.

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### **ScummVM** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188293068-a2814bb4-6c1a-4097-98a7-5a3a8e3af279.png" align="right" width="240" />

- Emulator: **lr-scummvm**
- Rom Folder: `SCUMMVM`
- Extensions: `.scummvm`

> **Notes**
> For each ScummVM game folder you need to create a `scummvm` file with the same name (just add the `.scummvm` extension), and place it in the root of the rom folder (`Roms/SCUMMVM`). The file must contain the game's "short name" which can be found here: <https://www.scummvm.org/compatibility/2.7.0/>. (This requires Onion V4 and an updated ScummVM package - done via Package Manager).
>
> **Example**  
> Game folder: `Roms/SCUMMVM/The Secret of Monkey Island/`  
> ScummVM file: `Roms/SCUMMVM/The Secret of Monkey Island.scummvm`, contents: `scumm:monkey`
>
> **Audio Troubleshooting**  
> We recommend sourcing original `.sou` audio files for `Full Throttle`, `The Dig` and `The Curse of Monkey Island` to avoid audio issues (such as missing speech or dropping out).  

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


### SquirrelJME ⚠

<table><td><br/>

- Emulator: squirreljme_libretro
- Note: This core is still under development and not yet capable of running games. You can follow the [developer on reddit](https://www.reddit.com/r/SquirrelJME/)

<img alt="experimental" src="https://user-images.githubusercontent.com/44569252/190520187-500c6659-b99b-497a-b2f3-817f1e6e2669.png" />
</td></table>


### **TIC-80** ✔

<table><td><br/>

<img src="https://user-images.githubusercontent.com/44569252/188293085-b0a1c5a9-ee58-4334-ae39-dfc128d9a9da.png" align="right" width="90" />

- Emulator: **lr-tic80**
- Rom Folder: `TIC`
- Extensions: `.tic`
- Bios: None

<img alt="verified" src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>


## Rom Folders - Quick Reference

![Roms](https://user-images.githubusercontent.com/98862735/177056430-38a57b7d-2293-4ccd-916c-4a43a8809e70.png)

For convenience, a list of Rom folder names for all *fully supported* cores is detailed below. 

**IMPORTANT**
* If they do not already exist on your SD card, Onion will create rom directories as required during first install. 
* All names are case sensitive and may differ from the names on the stock Miyoo SD card. 
* This list does not include expert emulators. 

| System                        | Rom Folder (Case Sensitive) |
| ----------------------------- | --------------------------- |
| Amiga                         | `AMIGA`                     |
| Amstrad CPC                   | `CPC`                       |
| Arcade (Mame 2003+)           | `ARCADE`                    |
| Atari 2600                    | `ATARI`                     |
| Atari 5200                    | `FIFTYTWOHUNDRED`           |
| Atari 7800                    | `SEVENTYEIGHTHUNDRED`       |
| Atari Lynx                    | `LYNX`                      |
| Bandai Sufami Turbo           | `SUFAMI`                    |
| Bandai WonderSwan & Color     | `WS`                        |
| Capcom Play System 1          | `CPS1`                      |
| Capcom Play System 2          | `CPS2`                      |
| Capcom Play System 3          | `CPS3`                      |
| ColecoVision                  | `COLECO`                    |
| Fairchild Channel F           | `FAIRCHILD`                 |
| Famicom Disk System           | `FDS`                       |
| Game & Watch                  | `GW`                        |
| GCE Vectrex                   | `VECTREX`                   |
| Magnavox Odyssey 2            | `ODYSSEY`                   |
| Mattel Intellivision          | `INTELLIVISION`             |
| Mega Duck                     | `MEGADUCK`                  |
| MS-DOS                        | `DOS`                       |
| MSX - MSX2                    | `MSX`                       |
| NEC SuperGrafx                | `SGFX`                      |
| NEC TurboGrafx   CD           | `PCECD`                     |
| NEC TurboGrafx-16             | `PCE`                       |
| Nintendo Entertainment System | `FC`                        |
| Nintendo Game Boy             | `GB`                        |
| Nintendo Game Boy Advance     | `GBA`                       |
| Nintendo Game Boy Color       | `GBC`                       |
| Nintendo Pokemini             | `POKE`                      |
| Nintendo Satellaview          | `SATELLAVIEW`               |
| Nintendo Super Game Boy       | `SGB`                       |
| Nintendo Super Nintendo       | `SFC`                       |
| Nintendo Virtual Boy          | `VB`                        |
| PICO-8                        | `PICO`                      |
| Ports collection              | `PORTS`                     |
| ScummVM                       | `SCUMMVM`                   |
| Sega 32X                      | `THIRTYTWOX`                |
| Sega CD                       | `SEGACD`                    |
| Sega Game Gear                | `GG`                        |
| Sega Genesis                  | `MD`                        |
| Sega Master System            | `MS`                        |
| Sega SG-1000                  | `SEGASGONE`                 |
| Sinclair ZX Spectrum          | `ZXS`                       |
| SNK NeoGeo                    | `NEOGEO`                    |
| SNK NeoGeo CD                 | `NEOCD`                     |
| SNK NeoGeo Pocket & Color     | `NGP`                       |
| Sony Playstation              | `PS`                        |
| TIC-80                        | `TIC`                       |
| VideoPac                      | `VIDEOPAC`                  |
| Watara Supervision            | `SUPERVISION`               |
