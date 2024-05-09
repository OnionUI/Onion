---
slug: /emulators/scummvm
---

# ScummVM ✔

<img src="https://user-images.githubusercontent.com/44569252/188293068-a2814bb4-6c1a-4097-98a7-5a3a8e3af279.png" align="right" width="240" />

- Emulator: **lr-scummvm**, ScummVM Standalone
- Rom Folder: `SCUMMVM`
- Extensions: `.target`

---

Although included in the "Emulators" section of this documentation for practical reasons, ScummVM project is not an emulator but a complete rewrite of the games executables. In other terms each game launched by ScummVM works like a port with its own dedicated engine. This implementation offers a rendering of games that's very close to the original rendering, with higher performance than you'd expect from an emulator. To play to ScummVM in the best conditions it is recommended to not use modified game assets. 

For more details about ScummVM, the [official FAQ](https://docs.scummvm.org/en/latest/help/faq.html) and [the official wiki](https://wiki.scummvm.org) are a good start.

### Adding games

No need to create your shortcuts manually anymore, Onion will import your games automatically ! 
Just put your games folders in `Roms/SCUMMVM/[Game folder]/` and then run `~Import games` in ScummVM section. 

ScummVM documentation offers great ressources to learn how to add games to ScummVM : This article "[Where to get the games](https://wiki.scummvm.org/index.php?title=Where_to_get_the_games)" and the [list of supported games](https://wiki.scummvm.org/index.php?title=Category:Supported_Games) will help you to know which files to copy from your original game to get it working properly on ScummVM.

The import script will ask you if you want to use ScummVM database names or the names specified on your own folders. 
As ScummVM games names are often long (with details about version and language as `Full Throttle (Version A, English)`) you can also choose to remove parens.

### Audio troubleshooting

ScummVM project doesn't support modified game assets so we recommend to use original `.sou` uncompressed audio files for `Full Throttle`, `The Dig` and `The Curse of Monkey Island` to avoid audio issues (such as missing speech or dropping out).  


### Speed troubleshooting

On demanding games like Grim Fandango, you can adjust "Target FPS for stutter reduction" in core options to 25 or 15 (baphomet 2.5 works fine with 25 for example).


:::note
ScummVM Standalone (not a RetroArch core but the native ScummVM reimplementation) can be found in the "Expert" section of [Package Manager](../apps/package-manager). It offers much improved performances (Grim, Monkey Island 4 or Blade Runner works perfectly).
However its integration in Onion is not perfect : no automatic save states, no resume at boot , different shortcuts...
:::
