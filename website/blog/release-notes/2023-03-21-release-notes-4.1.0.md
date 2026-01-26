---
slug: release-notes/4.1.0
authors: [aemiii91, schmurtz]
tags: [release-notes, stable]
image: ./assets/banner-release-4.1.png
---

# Release notes: Onion V4.1

<img src={assets.image} />

*Official release notes for Onion V4.1.0*

This release features a new menu, *GLO*, with game and list specific options (for example, you can now change core per game and load without save state); automatic icon switching and customization, a whole new ports collection, random game app, on-screen overlays, AdvanceMENU (an alternative frontend), and so much more!

<Truncate />

## Features


### Game List Options (GLO)
<sup>

by @Aemiii91

</sup>

*Press Y in a game list to get specific options for the selected game and the entire list*

<table><thead>
<th colspan="3"><b>Game List Options</b></th>
</thead><tr>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/226447277-a9452ecc-92ad-407f-a629-307491a652b4.png"/></td>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/226447388-8c9c95f1-536a-4946-8b0f-f4b8ead0e97e.png"/></td>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/226447466-71d4f5c1-7675-4355-9b60-20dbd1a03eae.png"/></td>
</tr><tr>
<td align="center" valign="top"><p><i>When in a game list, press <kbd>Y</kbd> to open GLO (this action can be reassigned in Tweaks)</i></p></td>
<td align="center" valign="top"><p><i>GLO - specific options for the selected game or the entire list</i></p></td>
<td align="center" valign="top"><p><i>Use the "Game core" option to change core per game</i></p></td>
</tr></table>

- **Reset game** - *load game without save state*
- **Game core** - *set custom core per game*
- **Filter list** - *use a keyword to filter the list*
- **Refresh roms** - *refresh the list’s game cache*
- *Custom scripts:*
  - **Random game** - *added by Random Game app - launches a random game from the list*
  - **Set time** - *added by Clock app - quick access to setting date and time*


### Icon switching
<sup>

by @Aemiii91

</sup>

- ThemeSwitcher now supports themes with icon packs - *remember to visit our [themes repository](https://github.com/OnionUI/Themes/blob/main/README.md) for additional themes and icon packs!*
- Change icon pack and edit individual icons in Tweaks

<table align="center"><thead>
<th colspan="2">Icon Switching</th>
</thead><tr>
<td width="65%" align="center" rowspan="2"><img src="https://user-images.githubusercontent.com/44569252/226450380-2d6ec505-c65d-4b67-bbc5-7104e39cd7ee.gif"/></td>
<td><img src="https://user-images.githubusercontent.com/44569252/226451178-3d11e9f4-0c7f-4213-af24-ad1209b36350.png"/></td>
</tr><tr>
<td><img src="https://user-images.githubusercontent.com/44569252/226451190-2529bb42-f5b3-4a9e-acfd-6cf65e2d4d33.png"/></td>
</tr><tr>
<td align="center"><p><i>Showcase: Applying themes with icon packs</i></p></td>
<td align="center"><p><i>Use Tweaks to change icon pack or edit individual icons</i></p></td>
</tr></table>


### Ports Collection
<sup>

by @schmurtzm

</sup>

Onion's Ports Collection has been completely redone and now includes 61 game ports!  
All the necessary files (except licensed game files) are now hosted on the:  

<p align="center"><a href="https://github.com/OnionUI/Ports-Collection">Ports Collection repository</a></p>

<table align="center"><thead>
<th>Ports Collection</th>
</thead><tr>
<td align="center"><a href="https://user-images.githubusercontent.com/44569252/226474430-0123c521-e96f-4603-bdf3-725ad8d2f3bc.png"><img width="65%" src="https://user-images.githubusercontent.com/44569252/226474515-7750fb79-9bda-4339-ac08-457dc63da5c6.png"/></a></td>
</tr><tr>
<td align="center" valign="top"><p><i>Click the image to view it in full size</i></p></td>
</tr></table>

- 24 free game ports are offered as complete packs
- 37 game ports are available through the collection of game engines, and you are only required to supply the licensed data files from the original game
- Pre-configured config files are included - which gives you the best settings and uniform key mapping for FPS games (*credit: axcelon*)
- Box art is also included


### New apps

- **Random Game** - *launch a random game from any cached system - credit: @marchiore*
- **Video Player** (FFplay) - *watch your favorite 4:3 or 16:9 video content - credit: Steward-Fu, @bobotrax, @schmurtzm* 
- **PDF Reader** (Green) - *ideal for looking up clues in game manuals  - credit: Steward-Fu*
- **Ebook Reader** (Pixel Reader) - *well suited for reading walkthroughs on the go  - credit: [ealang](https://github.com/ealang/pixel-reader)*

<p align="center"><em>
<img title="Random Game" src="https://user-images.githubusercontent.com/44569252/226454336-c2425e57-e507-4b32-aa30-8e1b3884f4ef.png"/>
<img title="Video Player" src="https://user-images.githubusercontent.com/44569252/226454472-7e4e6769-a86c-469f-b735-e85473094b0e.png"/>
<img title="PDF Reader" src="https://user-images.githubusercontent.com/44569252/226454495-bd086c74-61fe-4709-9e97-bebb3febbe75.png"/>
<img title="Ebook Reader" src="https://user-images.githubusercontent.com/44569252/226454509-1815c543-8001-4b7d-b7d9-2d3be5795140.png"/>
</em></p>


### Other exciting features

- **Automatic import of ScummVM games**: games are now automatically imported in Onion thanks to a new script created - no more manual shortcuts to create! (@schmurtzm)
- A new core option has been added to ScummVM: "Target FPS for stutter reduction" allows to run some heavy games with less audio stuttering - unique to Onion! (@schmurtzm)
- Improved **color palette organization** for GB: an organization by palette style (Essentials, Subtle, Single_Color, Multicolor, Hardware, Nintendo_Official, Extras, Others) unique to Onion ! (Jeltron & @schmurtzm)
- mGBA **[one-key fast forward button](https://github.com/schmurtzm/mgba/commit/99387c04ae2879692ee9ff199dc68b6c162f0a8a)** - <kbd>R2</kbd> by default (@schmurtzm)
- Added support for **On-Screen Overlays** in RetroArch (Eggs)
- **Hotkey for video scaling:** <kbd>MENU</kbd>+<kbd>START</kbd> (Eggs) (*remember to save overrides to make it stick*) 
- File explorer for PDF Reader and Video Player apps  (@schmurtzm & @Aemiii91)
- **Tutorials:** video tutorial for arcade saves, video tutorial for Video Player shortcuts, pdf tutorial for PDF Reader  (@schmurtzm)
- Package Manager UI update (@Aemiii91)
- **AdvanceMAME with AdvanceMENU** - browse your arcade games with video previews! (@schmurtzm)
- *Onion easter egg* (コナミコマンド) (@Sichroteph)


<table align="center"><thead>
<th><i>Highlight:</i> Package Manager (UI update)</th>
<th><i>Highlight:</i> On-Screen Overlays</th>
<th><i>Highlight:</i> AdvanceMENU</th>
</thead><tr>
<td align="center" width="33%"><img src="https://user-images.githubusercontent.com/44569252/226452451-cd05258a-8da7-4f1e-bec1-bc41844b1dd4.gif"/></td>
<td align="center" width="33%"><img src="https://user-images.githubusercontent.com/44569252/226492100-f51306ee-9178-47b0-ae87-07f6de6eb7e5.png"/></td>
<td align="center" width="33%"><img src="https://user-images.githubusercontent.com/44569252/226492306-03c1e2f4-902f-4ba2-8d85-13044882ad0d.png"/></td>
</tr></table>


## Core updates
<sup>

by @schmurtzm

</sup>

- [**GBA**] Default core changed: `gpSP` -> `mGBA`
- [**Scumm**] Updated: `ScummVM 2.7`
- [**PS**] Updated: `PCSX-ReARMed`
- [**GB/GBC**] Updated: `Gambatte`
- [**Commodore 64**] Updated: `vice x64`
- [**ZX Spectrum**] Updated: `fuse`
- [**Arcade**] Updated: `MAME 2003-Plus`
- [**Virtual Boy**] Updated: `Beetle VB v1.31.0`
- [**Ports**] Updated: `ecwolf (Wolfenstein 3D)`
- [Expert/**PICO-8**] Updated: `fake-08 standalone`
- [**SNES**] Updated: `Beetle Supafaust` (*~10% increase in FPS*)
- [Expert/**SNES**] Updated: `Snes9x`
- [Expert/**SNES**] Updated: `Snes9x 2005` and `2005 Plus`
- [Expert/**SNES**] Updated: `Snes9x 2010`


### New core additions
<sup>

by @schmurtzm

</sup>

- [**Amiga**] Added: `puae 2021 v2.6.1`
- [**PICO-8**] Added `fake-08 libretro core` (*supports save states*)
- [Expert/**OpenBOR**] Added Steward-Fu's `OpenBOR`
- [Expert/**DOS**] Added: `DOSBox Pure 0.9.7` (the old 0.21 is still default)
- [Expert/**PS**] Added: `PCSX-ReARMed standalone` emulator (*no GameSwitcher integration, but much improved performance - allows for using enhanced resolution*)
- [Expert/**Arcade**] Added: `M.B.A-mini` (*M.B.A = MAME's skeleton + FBA's romsets*)
- [Expert/**Arcade**] Added: `AdvanceMenu`, `AdvanceMame`, and `AdvanceMess`
- [Expert/**NEC PC-98**] Added: `Neko Project II Kai`
- [Expert/**Music**] Added: `Game Music Emu (GME)`


## Added languages

- **Português do Brasil** (Brazilian Portuguese) - *credit: @anibaldeboni*
- **Nederlands** (Dutch) - *credit: @ronvandegraaf*
- **Svenska** (Swedish) - *credit: @Megamannen*
- **Turkce** (Turkish) - *credit: @tcgumus*
- **Українська** (Ukrainian) - *credit: @semioys*
- **Беларуская** (Belarusian) - *credit: @introkun*
- **Tiếng Việt** (Vietnamese) - *credit: Viên Vĩ Khang*
- **한국어** (Korean) - *updated by: @DDinghoya*


## Breaking changes

- The new V4.1 *Ports Collection* is not compatible with files from earlier versions (V4.0.4 and below) - *download the new ports files from our [Ports Collection repository](https://github.com/OnionUI/Ports-Collection), and add the necessary game files in `Roms/PORTS/Games`.*
- PS roms of type `.bin` now requires accompanying `.cue` files (you can use [this tool](https://www.duckstation.org/cue-maker/) to generate them) - *this improves game compatibility, fixes some audio issues, and allows for indexing games consisting of multiple `.bin` files!*


---

<p align="center">
<a href="https://github.com/OnionUI/Onion/releases/tag/v4.1.4" class="button button--primary button--lg">Download Onion V4.1.4</a><br/>
<small><i>you'll find the download at the bottom of the page</i></small>
</p>
