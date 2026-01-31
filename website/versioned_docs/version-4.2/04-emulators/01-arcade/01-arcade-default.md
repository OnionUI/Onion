---
slug: /emulators/arcade/default
---


# Arcade (Default) ✔

<img src="https://user-images.githubusercontent.com/44569252/188292473-5ce7149e-2d90-441f-9c17-fc259c674387.png" align="right" width="240" />

- Alias: MAME
- Emulator: **lr-mame2003plus** 
- Rom Folder: `ARCADE` 
- Extensions: `.zip`
- Bios: `pgm.zip` (for PGM games only like *Knights of Valour* and *DoDonPachi*)
- Required rom set version: `MAME 2003-Plus Reference: Full Non-Merged Romsets`
- Samples: Audio Samples for Mame2003Plus should be placed in the `/BIOS/mame2003-plus/samples` folder
 
MAME does not play well with save states this is a core issue and cannot be fixed. It is recommended to disable 'auto-loading of save states' in the Retroarch menu (and save core overrides).

An onscreen message saying ‘This Game Will Not Work’ is MAME letting you know that the game is not emulated in the MAME version and you will not find a compatible/playable rom for that game.

Current/modern versions of MAME are not compatible with the Miyoo Minis limited hardware specification which is why MAME 2003Plus has been chosen as the default.  

https://www.youtube.com/watch?v=-0riylHkJis
