---
slug: /emulators/neogeocd
---

# SNK - Neo Geo CD ✔

<img src="https://user-images.githubusercontent.com/98862735/190532551-7b1c2f3e-9d48-4dd9-8ccb-2b10cfb6ce26.png" align="right" width="240" />

- Emulator: **lr-neocd**
- Rom Folder: `NEOCD`
- Extensions: `.bin/.cue` `.chd` `.m3u`
- Bios: (`000-lo.lo` or `ng-lo.rom`) and (`neocd_f.rom` or `neocd.bin` or `uni-bioscd.rom`) placed in a folder named neocd within the bios folder

UniBIOS can be used but v4.0 can lead to missing or corrupted audio when used with save states. Earlier UniBIOS version are OK.  

Because Neo Geo roms can come in different formats (split or non-merged), it's recommended to keep the 'neogeo.zip' bios in both the `/BIOS` folder and the `/Roms/NEOGEO` folder to ensure best compatibility.
