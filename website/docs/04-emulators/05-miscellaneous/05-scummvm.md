---
slug: /emulators/scummvm
---

# ScummVM ✔

<img src="https://user-images.githubusercontent.com/44569252/188293068-a2814bb4-6c1a-4097-98a7-5a3a8e3af279.png" align="right" width="240" />

- Emulator: **lr-scummvm**
- Rom Folder: `SCUMMVM`
- Extensions: `.target`

---

### Adding games

No need to create your shortcuts manually anymore, Onion will import your games automatically ! 
Just put your games folders in `Roms/SCUMMVM/[Game folder]/` and then run `~Import games` in ScummVM section. 

The import script will ask you if you want to use ScummVM database names or the names specified on your own folders. 
As ScummVM games names are often long (with details about version and language as `Full Throttle (Version A, English)`) you can also choose to remove parens.

### Audio troubleshooting

We recommend sourcing original `.sou` audio files for `Full Throttle`, `The Dig` and `The Curse of Monkey Island` to avoid audio issues (such as missing speech or dropping out).  


### Speed troubleshooting

On demanding games like Grim Fandango, you can adjust "Target FPS for stutter reduction" in core options to 25 or 15 (baphomet 2.5 works fine with 25 for example).

Do not hesitate to test the standalone version of ScummVM in "expert" section of Package Manager, this one offers impressive performances (Grim, Monkey Island 4 or Blade Runner works perfectly but the standalone version will not be compatible with the Game Switcher and auto save states).
