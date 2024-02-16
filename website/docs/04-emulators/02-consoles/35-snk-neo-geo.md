---
slug: /emulators/neogeo
---

# SNK - Neo Geo ✔

<img src="https://user-images.githubusercontent.com/44569252/188292808-3addb46b-9939-4461-bc17-c7323911900f.png" align="right" width="220" />

- Emulator: **lr_fbalpha2012_neogeo**, GnGeo
- Required ROM Version: `"FBA Arcade Set v0.2.97.29 for FBA 2012 libretro"` (search with quotes for exact match)
- Alternative ROM Pack: `"Neo-Geo Rom Collection By Ghostware"` (search with quotes for exact match)
- Rom Folder: `NEOGEO`
- Extensions: `.zip` (must be lowercase)
- Bios: `neogeo.zip`

UniBIOS can be used but v4.0 can lead to missing or corrupted audio when used with save states. Earlier UniBIOS version are OK. 

Because Neo Geo roms can come in different formats (split or non-merged), it's recommended to keep the `neogeo.zip` bios in both the `/BIOS` folder and the `/Roms/NEOGEO` folder to ensure best compatibility.

For best game compatibility, seek out the recommended romset above. Alternative romsets for Final Burn cores may be largely compatible (i.e. v0.2.97.24, v0.2.97.39, v0.2.97.40, v0.2.97.44).

https://www.youtube.com/watch?v=CGKX6yPG2nE

## Notes on GnGeo

The GnGeo is an AES/MVS Neo Geo emulator (without NG-CD support) based of MAME ROMsets.
This emulator is a standalone emulator (not Retroarch core) which natively supporte GNO files.
GnGeo is interesting because of loading times. Tested a few big games like kof2000 and it starts just a few seconds instead of 45 seconds. The framerate is also very good.

Interesting information about GnGeo [here](https://github.com/TriForceX/MiyooCFW/discussions/369) (with compatibility list).