---
slug: /advanced/combine-systems
---

![Advanced](https://user-images.githubusercontent.com/98862735/178853223-10d53d46-3d80-4d44-95d6-3cd345a730e1.png)


Below is an example `launch.sh` script for combining different systems or cores inside on Emu/Roms folder. To get started you can copy any of the subfolders inside `Emu`, as long as it contains a `launch.sh` and `config.json` file. To set the name of the emu, edit the label inside `config.json`. You also need to make sure the rom and image paths are pointing to the right folder in `Roms`.

The example is made up of three different core selection methods, you can include one or more of them depending on your needs. To find the correct core names, you can lookup the filename in `RetroArch/.retroarch/cores` and remove the `_libretro.so` part.

Edit the `launch.sh` to contain the following (changing out the core names and keywords to fit your specific use-case):

```sh title='Emu/[SYSTEM]/launch.sh'
#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
homedir=`dirname "$1"`

# Set the default core
core="mednafen_supafaust"
```

The `core` variable is initialized with the default core name, which will be used for unknown file extensions. Remember to either clear the `extlist` in the `config.json` file, or add all the supported extensions (be sure to add the lower- and uppercase versions).

Below is the first method of choosing a core for the rom you want to launch. It's purely based on the file extension of the rom, and will only work if the cores you are choosing between use different file extensions. E.g. here we're creating a games folder that can contain PSX, SNES, and PC-Engine games.

```sh title='Emu/[SYSTEM]/launch.sh (continued)'
# Select core based on file extension
case `echo "$(basename "$1")" | awk -F. '{print tolower($NF)}'` in
    iso|cue|img|mdf|pbp|toc|cbn|m3u|ccd|chd)
        core="pcsx_rearmed"
        ;;
    sfc|smc)
        core="mednafen_supafaust"
        ;;
    pce)
        core="mednafen_pce_fast"
        ;;
    *)
        ;;
esac
```

Another way to select cores is by checking the name of the subfolder it is in. This will solve the problem of different cores accepting the same files. In our example we can know detect which core to use for games located in different subfolders. The PlayStation core will be used for roms located in `Roms/[ROMFOLDER]/PS` or `Roms/[ROMFOLDER]/PSX`, and the SNES core will be used for the `SFC` or `SNES` subfolders, etc.

```sh title='Emu/[SYSTEM]/launch.sh (continued)'
# Select core based on name of containing folder
case `echo "$(basename "$homedir")" | awk '{print toupper($0)}'` in
    PS|PSX)
        core="pcsx_rearmed"
        ;;
    SFC|SNES)
        core="mednafen_supafaust"
        ;;
    PCE|PCECD)
        core="mednafen_pce_fast"
        ;;
    *)
        ;;
esac
```

A third way to select a core, is to check for keywords in the filename. Below I've made a switch case that selects a core based on different abbreviations in parentheses. To add more abbreviations be sure to add `*"([ABBR])"*` with a `|` as separator.

```sh title='Emu/[SYSTEM]/launch.sh (continued)'
# Select core based on parentheses in filename
case `echo "$(basename "$1")" | awk '{print toupper($0)}'` in
    *"(PS)"*|*"(PSX)"*)
        core="pcsx_rearmed"
        ;;
    *"(SFC)"*|*"(SNES)"*)
        core="mednafen_supafaust"
        ;;
    *"(PCE)"*|*"(PCECD)"*)
        core="mednafen_pce_fast"
        ;;
    *)
        ;;
esac
```

Lastly, we need to launch RetroArch with the selected core.

```sh title='Emu/[SYSTEM]/launch.sh (continued)'
cd /mnt/SDCARD/RetroArch/
HOME=/mnt/SDCARD/RetroArch/ ./retroarch -v -L '.retroarch/cores/'"$core"'_libretro.so' "$1"
```

