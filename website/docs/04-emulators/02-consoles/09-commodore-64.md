---
slug: /emulators/c64
---

# Commodore 64/PET ✔

- Emulator: **lr-vice_x64**, Frodo
- Rom Folder: `COMMODORE`
- Extensions: `.d64` `.zip` `.7z` `.t64` `.crt` `.prg` `.nib` `.tap`
- Bios: None


## Note

`.crt` roms boot fast and not `.d64`.

`.d64` games can take up to 2 minutes to boot. Fortunately after that the save state allows to restart it quickly.

You can convert `.d64` to `.crt` thanks to [disk2easyflash](https://csdb.dk/release/?id=150323).

Command line to convert :
`disk2easyflash.exe -c mygame.d64 mygame.crt`

`.crt` games boot in only few seconds but are only compatible with Vice core and not Frodo core.