---
slug: /emulators/amiga
---

# Commodore - Amiga & Amiga CD32  ✔

<img src="https://user-images.githubusercontent.com/44569252/188292584-cdc85cef-c384-4134-91c0-1e87d3360d76.png" align="right" width="240" />

- Alias: Amiga CD32
- Emulator: **lr-puae**, lr-uae4arm, lr-PUAE 2021 extreme
- Rom Folder: `AMIGA`
- Extensions: `.adf` `.hdf` `.lha` `.zip` (For CD32: `.bin/.cue` `.iso`)
- Bios: `kick33180.A500`, `kick34005.A500`, `kick34005.CDTV`, `kick37175.A500`, `kick37350.A600`, `kick39106.A1200`, `kick39106.A4000`, `kick40060.CD32`, `kick40060.CD32.ext`, `kick40063.A600`, `kick40068.A1200`, and `kick40068.A4000`

For maximum compatibility add ALL above ‘kickstart roms’ to the `/BIOS` folder and ensure they are named exactly as detailed (lowercase).

See [this link](https://docs.libretro.com/library/puae/) for more information.

- <kbd>SELECT</kbd> toggles the onscreen keyboard, <kbd>L</kbd> & <kbd>R</kbd> are mapped to the mouse buttons.
- This emulator will play Amiga CD32 games though some perform better than others.
- Setting CPU speed to -700 (negative 700) in the Retroarch menu may improve A1200/CD32 performance.
- Other Core Settings that might be useful for some games are frameskip set to 1, or setting off the blitter wait in Video options.
- Some games will have stuttering audio or behave very slowly regardless of the CPU speed, frameskip etc. Many of these games will work fine if you find another format for them. .hdf (hard disk files) are the most problematic. .adf or .lha games usually work better although you might have to open RA options to switch or add floppy disks when required. If you find a game that won't run correctly no matter the options you set for it, simply try to find another version in another format.

:::note
"PUAE 2021 extreme" is a core made by KMFD. It includes some optimisations which could be useful for some demanding games. This alternative core is available in Expert section of Package Manager.
:::