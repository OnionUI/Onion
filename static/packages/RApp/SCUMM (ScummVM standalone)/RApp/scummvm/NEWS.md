For a more comprehensive changelog of the latest experimental code, see:
        https://github.com/scummvm/scummvm/commits/

#### 2.8.0 (XXXX-XX-XX)

 New games:
   - Added support for macOS versions of Syberia and Syberia II.
   - Added support for The Vampire Diaries and Nancy Drew: Secrets Can Kill.
   - Added support for Reah: Face the Unknown and Schizm: Mysterious Journey.
   - Added support for Might and Magic Book One

 New platforms:
   - Added libretro new shiny port.
   - Added Atari port.

 General:
  - Added optional dependency for libvpx.
  - Added optional dependency for libmikmod.
  - Added support for PC98 font ROM.

 Asylum:
   - Added support for Chinese Sanitarium.

 Dragons:
   - Implemented localizations for the copyright screen and main menu.

 GrimE:
   - Added support for Chinese Grim Fandango.
   - Added support for Russian Grim Fandango translations from Fargus,
     ENPY and 7Wolf.

 Kyra:
   - Added support for Kyra1 Amiga French release and DOS Czech fan translation.
   - Improved game controls menu accuracy for all platforms.
   - Added support for Chinese EOB2.

 SCUMM:
   - Added support for unpacked RuSCUMM patches for Russian localizations.

 Sherlock:
   - Added support for Chinese Serrated Scalpel.

 Sword2:
   - Added support for Chinese translation.

 TwinE:
   - Added support for Japanese translation.

 Ultima:
   - Improve Ultima VI responsiveness.
   - Fix crash loading Ultima VI savegames when dialogs are open.
   - Fix ordering of generated items in Ultima VI.
   - Fix auto-readying throwing weapons in Ultima VI.
   - Fix Ultima VI crash opening new-style spellbook gump.
   - Fix Ultima VI actor doll and gump colors.
   - Fix Ultima VI crash using look action on walls.
   - Fix several other miscellaneous crashes.
   - Fix spawners and projectiles to more closely match the original.

 Voyeur:
   - Added support for German fan translation.

 Xeen:
   - Renamed 'xeen' engine to 'mm' for Might & Magic.


#### 2.7.1 (2023-XX-XX)

 General:
  - Fixed playback speed for Theora videos.
  - Added scummvm.log file viewer.
  - Exposed ScummVM debug level in the GUI (Misc tab).

 AGI:
   - Restored possibility to pick arbitrary render mode in Game Options.

 HDB:
   - Fixed bug with inability to pick up red envelope with keyboard.

 SAGA2:
   - Fixed crash when hovering over certain spells.

 Sword1:
   - Added detection for Czech and Polish versions with DXA cutscenes.

 Sword2:
   - Added support for GOG.com version.
   - Added detection for alternate Polish version.

 Touche:
   - Added support for Russian translation by old-games.ru.

 iOS port:
   - Fix default theme selection on modern devices.
   - Better support for autostart configurations.

 macOS port:
   - Better support for autostart configurations.

 RISC OS port:
   - Fixed crash on RISC OS 5 with games that require lots of RAM.


#### 2.7.0 "The Real Slim Shader" (2023-02-26)

 New games:
   - Added support for Soldier Boyz.
   - Added support for C64 and ZX Spectrum versions of GLK Scott Adams
     Interactive Fiction games.
   - Added support for GLK Scott Adams adventures 1-12 in the TI99/4A format.
   - Added support for Obsidian.
   - Added support for Pink Panther: Passport to Peril.
   - Added support for Pink Panther: Hokus Pokus Pink.
   - Added support for Adibou 2 "Environment", "Read/Count 4 & 5" and "Read/Count 6 & 7".
   - Added support for Driller/Space Station Oblivion (DOS/EGA/CGA, Amiga, AtariST, ZX Spectrum and Amstrad CPC versions).
   - Added support for Halls of the Dead: Faery Tale Adventure II.
   - Added support for Chop Suey, Eastern Mind, and 16 other Director 3 and Director 4 titles.

 New platforms:
   - Added support for the RetroMini RS90 under OpenDingux beta.
   - Added support for the 1st generation Miyoo (New BittBoy, Pocket Go and PowKiddy Q90-V90-Q20) under TriForceX MiyooCFW.
   - Added support for the Miyoo Mini.
   - Added support for KolibriOS.

 General:
   - Reduced amount of false positives in Mass Add.
   - Updated the Roland MT-32 emulation code to Munt mt32emu 2.7.0.
   - Added support for shader-based scalers.
   - Added option for mono sound output (via --output-channels=CHANNELS command
     line option).
   - Improved cursor scaling in OpenGL mode.
   - Fix crash when browsing folders containing files with \1 in the names.
   - Added possibility to specify RNG seed via GUI or command line option.
   - Added possibility to run ScummVM in autodetection mode by renaming the
     executable starting with 'scummvm-auto' or by providing an empty file
     named 'scummvm-autorun' next to the ScummVM executable.
   - Added possibility to supply command line parameters which will be picked
     up automatically. Put them one per line in a file named 'scummvm-autorun'.
   - Added possibility to customize the default settings by specifying an initial
     configuration file to load if no configuration file exists in the usual
     location (via --initial-cfg=FILE or -i command line option).
   - Added support for loading game resources which are bigger than 2GB on more
     platforms.

 AGI:
   - Improved support for French translations.

 AGOS:
   - Added option to disable the fade-out effects on room transition for
     Simon1 and 2.

 AGS:
   - Added support for the original installer files for Maniac Mansion Deluxe and
     The New Adventures of Zak McKracken.

 Director:
   - Support for Pippin version of L-Zone.
   - Fix a bug caused by use of slash in filename.

 Dreamweb:
   - Support playing from the original installer floppies.

 Hadesch:
   - Added support for a 1997 release.

 Kyra:
   - Added support for the Korean version of Legend of Kyrandia 1.
   - Support multi-floppy mac kyra1 both as installer floppies and as installed directory.
   - Added support for the Hebrew version of Legend of Kyrandia 3.

 MADS:
   - Added support for original floppy installer file layout for Rex Nebular.

 Neverhood:
   - Added support for Japanese version of Neverhood.
   - Support localizations by -=CHE@TER=- & Rigel.

 Plumbers:
   - Fixed crash with windows version.

 Private:
   - Allow playing mac version directly from installer CD.
   - Added support for Japanese/Mac version

 Queen:
   - Added option for using a better font in Hebrew version.

 SAGA:
   - Added support for Chinese Inherit the Earth.
   - Added support for Chinese I Have no Mouth and I Must Scream.
   - Added support for Korean I Have no Mouth and I Must Scream.
   - Added support for playing directly from floppy installer for ITE.
   - Support for Amiga (AGA/ECS, Retail/Demo, English/German) Inherit the Earth.

 SCI:
   - Improved text rendering for Macintosh titles.
   - Added support for Casio MT-540, CT-460 and CSM-1 MIDI devices for the SCI0
     games that originally supported it.

 SCUMM:
   - Added support for CGA, CGA Composite, CGA black & white and Hercules modes
     for SCUMM 1 versions of Zak McKracken and Maniac Mansion.
   - Improved accuracy of CGA and Hercules modes for SCUMM 2 (enhanced) versions of
     Zak McKracken and Maniac Mansion.
   - Improved accuracy of CGA and Hercules modes for Monkey Island 1 (EGA version
     only - the VGA version does not have CGA and Hercules modes).
   - Fixed some minor glitches for the CGA mode of Loom.
   - Added EGA dithering mode for VGA versions of Loom, Monkey Island 1 and 2 and
     Indiana Jones 4.
   - Fixed a possible dead-end in the Ultimate Talkie Edition of Monkey Island 2,
     if one doesn't pick up a required item before Captain Dread brings Guybrush
     back to Scabb Island, at the end of Part II.
   - Fixed various original game bugs, oversights and continuity errors (only
     when using the "Enable game-specific enhancements" setting).
   - Improved the decoding of some Western European game strings when they're
     displayed through ScummVM's interface, such as when pausing a game.
   - Fixed the navigator head not pointing to some directions in Monkey
     Island 1, in the original releases without the enhanced verb interface.
   - Fixed slightly inaccurate text position in v4 games and in Loom v3.
   - For Sam & Max, it is now possible to shoot down the text lines of the
     final credits with the crosshair mouse cursor, just like in the original.
   - Fixed lipsync issues in the final scene of Freddi Fish 4.
   - Fixed The Dig and COMI loading cursors not being shown when they should.
   - Improved the accuracy of some audio drivers, which was notably impacting
     the pitch bending effect in the DOTT intro music.
   - In COMI, only let Guybrush read the clock of Puerto Pollo in the English,
     Italian and (fan-made) Russian versions of the game, matching the behavior
     of the original interpreters (probably because of the poor results in the
     other languages).
   - Improved support for Hebrew HE game localizations.
   - Fixed Roland MT-32 support in Sam & Max.
   - Implemented original GUI and save menus for LucasArts games (DOS, Windows,
     Amiga, Macintosh, FM-Towns, SegaCD, Atari ST, NES and Commodore 64 versions).
     Also activate the general "Ask for confirmation on exit" option for a more
     authentic '90s experience!
   - Fixed minor timing issues for the SMUSH video engine, mostly affecting
     Full Throttle.
   - Added a low latency audio mode to Full Throttle, The Dig and The Curse of
     Monkey Island; this can improve audio performance expecially in non-desktop
     devices, but it is also a little less accurate than the original.
   - Implemented reloading CD audio for Monkey Island 1 and Loom (CD versions),
     when reloading a save state.

 Sherlock:
   - Added support for Chinese Rose Tattoo.

 Sky:
   - Added support for Chinese Beneath a Steel Sky.

 Sword1:
   - Support Novy Disk Russian translation.
   - Fixed launching various demos.
   - Switched detection to md5-based. Submit your unrecognized versions!

 Sword2:
   - Switched detection to md5-based. Submit your unrecognized versions!

 Tinsel:
   - Fixed Discworld II subtitle colors on big-endian ports.

 Toon:
   - Made game menus behave like in the original.

 TwinE:
   - Fixed rendering issue with doors.
   - Fixed invalid music pause in behaviour and inventory menu.
   - Fixed giving kashes instead of hearts as fallback.
   - Fixed item flashing when they drop.
   - Fixed meca penguin angle at spawn.
   - Fixed background redraw when watching cutscenes at the television.
   - Fixed recenter the screen on activating an inventory item.
   - Fixed pressing W to talk to people also makes you jump.

 Ultima8:
   - Added support for saving and resizing of minimap.
   - Adjust cursor to behave closer to the original.
   - Adjust item quantity slider to behave closer to the original.

 Xeen:
   - Several crash fixes for Clouds of Xeen.
   - Wait until farewell finishes before leaving shops.
   - Don't reload map after leaving character creation.
   - Implement correct clouds falling logic for Swords of Xeen.
   - Fix GateMaster monster in Underworld map.

 3DS port:
   - Update relocation parser to support PREL31 that are emitted by new compiler.
   - Do more autoconfiguration in ./configure.

 Android port:
   - Added Storage Access Framework support.
   - Improved support for game controllers.

 iOS port:
   - Added pointer device support.
   - Improved support for touchpad mode.
   - Added support for games that use 32 bit pixel formats.

 Nintendo DS port:
   - Added a splash screen to the top screen when the launcher is active.

 OpenDingux port:
   - Added support for dynamic plugins.

 PS Vita port:
   - Added support for dynamic plugins.

 PSP port:
   - Improved support for games that use 32 bit pixel formats.

 RISC OS port:
   - Added support for 26-bit versions of RISC OS.

#### 2.6.1 "Incremental Escapism" (2022-10-31)

 General:
   - Various improvements to the icon-based grid view in the ScummVM launcher.
   - Fixed possible graphics corruptions when using the OpenGL renderer.

 AGS:
   - Fixed a crash in AGDI's KQ1 remake (and some other games), on big-endian systems.

 AGOS:
   - Fixed various bugs that lead to crashes in the demo versions of The Feeble Files
     and Personal Nightmare and in the full version of Waxworks.
   - Fixed pirate dialogue freeze in Simon the Sorcerer 2.

 Asylum:
   - Fixed the Keyboard Config screen.
   - Added support for the keymapper feature to the dialog screen.
   - The in-game menu is now accessible via a keyboard shortcut.
   - Fixed a bug that caused the controls to be reset when leaving the Hive puzzle.
   - Fixed animation of floating bodies in Chapter 4.

 Ultima:
   - Fixed rendering issue when moving the minimap off the screen.
   - Fixed possible crash when using the autosave function.

 SCI:
   - Fixed various bugs and script issues in KQ4, KQ5, LB2, LONGBOW, PQ1, PQ3, QFG2, QFG3, QFG4 and SQ5.
   - Numerous improvements to various parts of the engine code.

 SCUMM:
   - Fixed potential lockups in games using iMuse.
   - Prevented crashes caused by missing .SOU/.BUN files.
   - Fixed crashes in v7-v8 games on strict-alignment platforms.
   - Fixed speech lipsync for COMI on big-endian ports (such as PS3 or Wii).

 Tinsel:
   - Fixed an issue in Discworld that prevented some items from being placed in the inventory.

 Toltecs:
   - Fixed crashes on some strict-alignment platforms.

 Toon:
   - Fixed music and SFX being muted by default when adding the Toonstruck game.

 Android port:
   - Properly convert mouse coordinates between screen coordinates and virtual ones.

 Windows port:
   - Clearing the icons cache created a duplicate ScummVM folder in the APPDATA directory. Fixed.

#### 2.6.0 "Insane Escapism" (2022-08-01)

 New games:
   - Added support for Sanitarium.
   - Added support for Hades Challenge.
   - Added support for Marvel Comics Spider-Man: The Sinister Six.
   - Added support for The 11th Hour.
   - Added support for Clandestiny.
   - Added support for Tender Loving Care (CD-ROM Editions).
   - Added support for Uncle Henry's Playhouse.
   - Added support for Wetlands.
   - Added support for Chewy: Esc from F5.

 General:
   - The project license has been upgraded to GPLv3+.
   - Now ScummVM requires C++11 for building.
   - Removed support for VS2008, as it doesn't support C++11.
   - Implemented enhanced filtering in the Search box. See "Understanding
     the search box" in the documentation for details.
   - Implemented Icon view in GUI (GSoC task).
   - Added support for the RetroWave OPL3 sound card.
   - Added OpenDingux beta port.
   - Removed Symbian port.
   - Added the create_engine tool to aid when creating new engines.
   - Fixed mouse capture in HiDPI mode.
   - The GUI Options dialog now marks settings overridden via command lines in red.
   - In GUI launcher it is now possible to group games by different categories.
   - GUI launcher has new game icons grid look.

 AGI:
   - Added support for Macintosh versions of Manhunter 1-2.

 AGS:
   - Synced changes from upstream AGS.

 AGOS:
   - Elvira 1: Added support for Casio MT-540/CT-460/CSM-1 and CMS/GameBlaster.
   - Elvira 1 & 2, Waxworks, Simon the Sorcerer: Added AdLib OPL3 mode.
     Depending on the game, this will prevent cut-off notes, add extra notes or
     instruments and/or add stereo.
   - Elvira 2, Waxworks: Added support for AdLib and MT-32 sound effects.
   - Elvira 2, Waxworks, Simon the Sorcerer floppy: Added Mixed MIDI support
     (MT-32 music with AdLib sound effects).
   - Simon the Sorcerer floppy: Improved AdLib sound effects accuracy.
   - Simon the Sorcerer: DOS version music tempos are now accurate. Both DOS
     and Windows versions now offer the choice of the DOS music tempos or the
     faster Windows tempos.
   - Simon the Sorcerer 2: Improved AdLib and GM support.
   - Simon the Sorcerer 2: Added workaround for the missing MT-32 tracks in the
     intro.

 BBVS:
   - Fixed the size of the main menu buttons being incorrect in some cases.
   - Fixed crash at the end of the Hock-A-Loogie mini game.

 Buried:
   - Added support for skipping synchronous audio and video.
   - The mouse pointer is now hidden during cutscenes.
   - Implemented game pausing via Control-P.
   - After saving, the player returns back to the game instead of the Biochip
     menu.
   - The game is now always paused when the Biochip menu or the save/restore
     dialogs are open.
   - Added metadata to saved games, including thumbnails, creation date and
     play time.
   - Saved games are now sorted by slot, like in other engines, instead of
     being sorted alphabetically.
   - The currently selected item is now stored in saved games.
   - Comments from Arthur that play in the background can now be stopped with
     the space key (the same key that replays Arthur's last comment).
   - The agent evaluation (current points) can now be shown with Control-D.
   - Fixed global flag corruption in death screens.

 Dreamweb:
   - Added text to speech for dialogs and object descriptions.

 Glk:
   - Added support for ZX Spectrum games with graphics in the Scott sub-engine.

 Kyra:
   - Added support for the Traditional Chinese versions of Legend of Kyrandia 1 - 3.
   - Added sound support for the Macintosh version of Legend of Kyrandia.
   - Added support for playing the Macintosh non-talkie version of Legend
     of Kyrandia 1 directly from the files on the CD. This means you no longer
     have to run the installer to extract the data files.

 NGI:
   - Fixed the rolling bridge state in scene 13.
   - Fixed getting stuck when teleporting to the foot in scene 30.
   - Fixed inconsistent cactus state.

 Private:
   - Refactored code to allow rendering using the original 256 color palette.
   - Fixed endianness issues.
   - Added support for the Korean release.

 Supernova:
   - Added text to speech for dialogs and object descriptions.

 SCI:
   - Added support for Text To Speech in SCI floppy games.
   - Allow saving from the ScummVM Global Game Menu in the following games:
     BRAIN1, BRAIN2, ECOQUEST1, ECOQUEST2, FAIRYTALES, PHARKAS, GK1, GK2, ICEMAN,
     KQ1, KQ4, KQ5, KQ6, KQ7, LB1, LB2, LIGHTHOUSE, LONGBOW, LSL1, LSL2, LSL3,
     LSL5, LSL6, LSL6HIRES, LSL7, PEPPER, PHANT2, PQ1, PQ2, PQ3, PQ4, PQSWAT,
     QFG1, QFG1VGA, QFG2, QFG3, QFG4, SHIVERS, SQ1, SQ3, SQ4, SQ5, SQ6, TORIN.
   - Fixed many script bugs in KQ6, KQ7, GK2, QFG3, QFG4, Hoyle4.
   - Fixed loading autosaves in Shivers and Phantasmagoria 2.
   - Added support for Korean fan translations from the scummkor project:
     EcoQuest 2 and Gabriel Knight 2.

 SCUMM:
   - New Digital iMUSE engine. Support for re-compressed audio files dropped in
     Full Throttle, The Dig and The Curse of Monkey Island.
   - Rewrote music player for Amiga versions of Indy3 and Loom in accordance
     to the original code.
   - Fix missing cursor in the 16-color Macintosh versions of Loom and Indiana
     Jones and the Last Crusade after loading a savegame.
   - It is now possible to replace the music in the floppy versions of Loom
     with audio tracks. The ScummVM Wiki has a list of which parts of the Swan
     Lake ballet the game uses: <https://wiki.scummvm.org/index.php/Loom>.
   - Fixed some MIDI music looping when it shouldn't in EGA/VGA floppy versions
     of The Secret of Monkey Island.
   - Fixed the lava flowing in the wrong direction in the VGA floppy version
     of The Secret of Monkey Island.
   - Fixed Full Throttle distorted graphics when Ben runs past the Corley
     Motors entrance.
   - Fixed the dissolve effect, and Bobbin's palette when leaving the darkened
     tent in the TurboGrafx-16 version of Loom, to match the original behavior.
   - Fixed incorrect dark rooms colors in MM NES on strict-alignment ports such
     as Dreamcast, Apple silicon and various handheld devices.
   - Rewrote text rendering routines for Full Throttle, The Dig and The Curse
     of Monkey Island in accordance to the original interpreters.
   - Rewrote timer handling routines to better approximate both the original
     hardware behavior and the intepreters' quirks.
   - Fix lip syncing in Backyard Baseball 2003.
   - Fixed various original game bugs and oversights in most of the LucasArts
     titles: <https://wiki.scummvm.org/index.php?title=SCUMM/Game_Enhancements>.
     Most of these enhancements can now be disabled in the game's settings if
     one prefers playing with the original behavior.
   - Added sliders for tweaking the CD audio playback in the MI1 CD intro, as
     well as VGA CD Loom in general. Loom is particularly sensitive to the
     amount of silence at the start of the track, and the CD version of MI1
     never synced the music as well to the intro as previous versions. See the
     Wiki for more details.
   - Detect and reject the EGA floppy version of Monkey Island 1 that Limited
     Run Games sold in their Monkey Island 30th Anniversary Anthology, if using
     the default DISK4 image, which is corrupted. It's possible to recover a
     working image from the KryoFlux dumps they also provided.
   - Fixed random number generation which fixes throwing in Backyard Baseball.
   - Marked a workaround in Monkey Island 2 (FM-Towns version) as an
     enhancement; this workaround originally restored a portion of the
     map chasing puzzle in Booty Island which had been cut in the
     FM-Towns version of the game.
   - Made the sentence line in Maniac Mansion work like the manual says, i.e.
     you can click on it to execute the command.

 Sherlock:
   - Fixed slowdown in Serrated Scalpel intro when playing the game from a small
     installation.
   - Fixed UI glitches in Serrated Scalpel.

 Titanic:
   - Fixed not being able to see House in Starfield puzzle.

 TwinE:
   - Fixed a bug in the collision code that made the game unfinishable due to the
     tank not moving any further in scene 63.
   - Fixed light angle calculation which produced rendering artifacts in a few
     scenes.
   - Fixed polygon rendering method for the boat windows.
   - Fixed wrong shooting direction for some actors.
   - Fixed door movement in some situations.

 Android port:
   - Added hardware acceleration for 3D graphics.
   - Improved touch controls.

 macOS port:
   - Added support for displaying OSD messages on the Touch Bar.

 Windows port:
   - Added "Portable Mode" in which the executable's directory is used to store
     application files if a scummvm.ini file is present, instead of the user's
     profile directory.
   - Fixed detection of the Application Data path on Windows 95/98/ME.

 RISC OS port:
   - Added support for dynamic plugins.
   - Added a native MIDI driver.

 Nintendo DS port:
   - Fixed screen scrolling when using the Load and Save dialogs.

#### 2.5.1 (2022-01-02)

 General:
   - Ported ResidualVM GUI theme to remastered version.
   - Fixed edge case for Punycode.
   - Fixed checking for savegame overwrite in autosave slot.
   - Fixed moving savegame to new slot for most engines.
   - Scalers are now supported with the OpenGL graphics mode.

 AGOS:
   - Fixed old Waxworks AdLib music regression.

 AGS:
   - Detection list updates.

 Grim:
   - Fixed default "Talk Speed" option value.
   - Fixed black screen while entering save game name.
   - OpenGL without shaders is preferred as default for Grim Fandango.

 Kyra:
   - Fixed graphical glitch in Legend of Kyrandia 3.

 SAGA:
   - Fixed digitized music not looping in Inherit the Earth.

 SCUMM:
   - Improved support for the high-resolution text in the 16-color Macintosh
     versions of Loom and Indiana Jones and the Last Crusade.
   - Improved OPL3 sound emulation for Sam&Max.
   - Fixed music not looping in The Dig.
   - Fixed crash when loading savegames from Humongous Entertainment games.

 Sherlock:
   - Fixed crash using matches on lab table.
   - Fixed character animation in Rose Tattoo.
   - Fixed glitch opening map in Rose Tattoo.
   - Fixed bell pull and fog horn sounds in Serrated Scalpel.
   - Fixed inventory not updating in Serrated Scalpel after examining watch.
   - Fixed fog overlay at Cleopatra's Needle in Rose Tattoo.
   - Fixed graphic issues in Serrated Scalpel looking at items whilst inventory is open.
   - Made Serrated Scalpel darts closer in speed to the original.
   - Fixed crash when moving cursor past end of string in Rose Tattoo save dialog.
   - Process pending key presses in the order they were made, not the most
     recent first.
   - Fixed crash when using Delete key in Rose Tattoo save dialog.
   - Fixed rare conversation bug in Serrated Scalpel that would happen when Lord
     Brumwell started talking to you while the inventory window was open.
   - Resume animations in Serrated Scalpel after conversations. This fixes the
     bug where Jock Mahoney or Nobby Charleton would get stuck indefinitely,
     refusing to talk to you.
   - Fixed various user interface glitches in both games.

 Stark:
   - Added OpenGL renderer (without shaders).
   - Added TinyGL renderer.
   - Fixed autosave handling.

 TwinE:
   - Numerous bugfixes and stability improvements.

 Xeen:
   - Fixed crash on startup loading constants from xeen.ccs.
   - Fixed spell selection aborting when characters were switched.
   - Fixed some bad memory accesses.
   - Various sound fixes.
   - Fixed the monster item drop probabilities.

#### 2.5.0 "Twenty years ago today..." (2021-10-09)

 New games:
   - Added support for Grim Fandango.
   - Added support for The Longest Journey.
   - Added support for Myst 3: Exile.
   - Added support for Little Big Adventure.
   - Added support for Red Comrades 1: Save the Galaxy.
   - Added support for Red Comrades 2: For the Great Justice.
   - Added support for Transylvania.
   - Added support for Crimson Crown.
   - Added support for OO-Topos.
   - Added support for Glulx interactive fiction games.
   - Added support for Lure of the Temptress Konami release.
   - Added support for Private Eye.
   - Added support for Spanish Blue Force.
   - Added support for Spanish Ringworld.
   - Added support for Spanish Amazon: Guardians of Eden.
   - Added support for AGS Games versions 2.5+.
   - Added support for Nightlong: Union City Conspiracy.
   - Added support for The Journeyman Project 2: Buried in Time.
   - Added support for Crusader: No Remorse.
   - Added support for L-ZONE.
   - Added support for Spaceship Warlock.

 New ports:
   - The Nintendo DS port got a major rewrite.

 General:
   - Switched ScummVM GUI output to UTF-32.
   - Updated the Roland MT-32 emulation code to the Munt project's mt32emu 2.5.1.
   - Updated Dropbox Cloud Storage to use the new Dropbox OAuth workflow.
   - Major extension to the number of supported graphics scalers.
   - Display path to scummvm configuration file in GUI -> Options -> Paths.
   - Added new optional dependency, giflib >= 5.0.0. Used by some version of LBA.
   - Added HiDPI support to the ScummVM GUI.
   - Added command line option --window-size for specifying ScummVM window size,
     applicable only to the OpenGL renderer.
   - Fixed switching to the default graphics mode. This was sometimes not applied
     until restarting ScummVM or starting a game.
   - ScummVM GUI has been fully translated into Korean and Japanese.
   - Added GUI option for enabling and disabling the Discord RPC integration.

 ADL:
   - Added support for Mystery House French translation.
   - Added support for several game variants.

 AGI:
   - Added support for Russian versions. Input now works.

 AGOS:
   - Added support for the Japanese PC-98 version of Elvira 1.

 CGE:
   - Added option to use Text To Speech for Soltys.

 CGE2:
   - Added option to use Text To Speech for Sfinx.

 Cine:
   - Added detection for Future Wars CD version with French translation patch.
   - Added detection for Italian Amiga Operation Stealth.
   - Fixed crash before entering secret base.
   - Fixed space missing in verb line.
   - Fixed vertically overflowing message boxes.

 Dreamweb:
   - Rendering fixes for Russian fan translation.

 Glk:
   - Fixed savegame issues with several subengines.
   - Fixed memory overrun in Level9 game detector.
   - Added detections for 2020 IF Comp games.
   - Enabled Glulx sub-engine.

 Gob:
   - Added support for Bargon Attack Russian translation.
   - Added support for Woodruff Russian translation.

 Griffon:
   - Fixed Return to Launcher from The Griffon Legend.
   - Added option to use Text To Speech in The Griffon Legend

 Grim:
   - Added support for Brazillian Portuguese Grim Fandango.

 Kyra:
   - Added support for the Japanese Sega-CD version of Eye of the Beholder.
   - Added support for the Hebrew fan translation of Legend of Kyrandia.
   - Added support for the Hebrew fan translation of Legend of Kyrandia 2.
   - Added support for the Simplified Chinese version of Legend of Kyrandia 3.
   - Added support for the playable demo of Lands of Lore.

 Lure:
   - Fixed fire animation in first room when loading saves.
   - Fixed AdLib support.
   - Fixed MT-32 support.

 Pegasus:
   - Added support for DVD/GOG.com release.

 Queen:
   - Added support for German Amiga floppy release.

 SAGA:
   - Added support for ITE GOG Mac CD v1.1.
   - Added support for ITE PC-98 Japanese.
   - Fixed digitized music not looping in Inherit the Earth.

 SCI:
   - Added RGB rendering mode (16/32bpp) for SCI0 - SCI1.1 games, which addresses palette
     issues in screen transitions and avoids mode changes when playing Mac QuickTime videos.
   - Added custom palette mods for SQ3 and LSL2 from the FreeSCI project. When enabled, the mods improve the visuals
     in these two games.
   - Added support for Macintosh version of Gabriel Knight 1.
   - Added support for CD-Audio version of Mixed-Up Mother Goose.
   - Added support for Korean fan translations from the scummkor project: Castle of Dr. Brain,
     EcoQuest 1, Gabriel Knight 1, King's Quest 1, 5, and 6, Laura Bow 2, and Space Quest 4.
   - Added support for Space Quest 4 Update 1.3 by New Rising Sun.
   - Added support for French LSL1VGA.
   - Added support for Hebrew QFG1VGA.
   - Added support for Hebrew GK1.
   - Added support for Russian Longbow.
   - Added support for Russian LSL6.
   - Added support for alternate Russian LSL7.
   - Added support for alternate Polish LSL7 including files packaged with InstallShield.
   - Added support for alternate Russian SQ5.
   - Added support for alternate Russian Torin's Passage.
   - Added support for fan game Soulshade Asylum.
   - SCI1.1 views are now scaled accurately.
   - Fixed sounds not stopping or restarting correctly.
   - Fixed sound issues when restoring SCI0 games.
   - Fixed corruption when "Prefer digital sound effects" was disabled in SCI1 games.
   - Space Quest 4 CD sound effects now match the selected platform.
   - Added option to enable Windows cursors in CD versions of King's Quest 5 and Space Quest 4.
   - Fixed "Girl In The Tower" song not playing at the end of King's Quest 6 CD.
   - Fixed King's Quest 5 Amiga beach error that prevented completing the game.
   - Fixed over 30 script bugs in GK1, HOYLE4, KQ1DEMO, KQ5, KQ6, KQ7, LB1,
     LSL3, PEPPER, PQ3, QFG3, SQ1, SQ4, and SQ6.
   - Unlocked JANE easter egg in Gabriel Knight 1.

 SCUMM:
   - Fixed Chinese, Japanese and Korean text display for The Dig and for The Curse of Monkey Island. These fixes
     also include some improvements to the common text display (mainly the formatting of wrapped texts).
   - Fixed display of Chinese, Japanese and Korean pause and restart dialogs.
   - Added support for numerous Korean translations from scummkor project.
   - Added support for Russobit-M versions of Pajama2 and SpyOzone.
   - Fixed speech playback in Akella version of COMI.
   - Added support for Discord and Humble Bundle versions of Indiana Jones and the Fate of Atlantis.
   - Added smooth scrolling for FM-TOWNS versions of games.
   - Added optional trimming to 200 pixels for some FM-TOWNS games, so aspect-ratio correction is possible.
   - Fixed audio distortion in Loom for PC-Engine.
   - Added support for the high resolution font and cursor in the 16-color Macintosh version of Loom.
   - Added support for Japanese Mac version of The Dig.
   - Added partial support for the high resolution fonts and cursor in the
     16-color Macintosh version of Indiana Jones and the Last Crusade.
   - Fixed missing instruments in the m68k Mac versions of Monkey Island 2
     and Indiana Jones and the Fate of Atlantis.
   - Added "Macintosh b/w" render mode for the 16-color Macintosh versions of
     Loom and Indiana Jones and the Last Crusade.
   - Enabled difficulty selection in the version of Monkey Island 2 that was
     included on the LucasArts Mac CD Game Pack II compilation. (It had been
     disabled along with the copy protection.)
   - Repaired clumsy crack in Maniac Mansion (enhanced)'s keypad script.
     This means that the GOG and Steam versions will no longer accept incorrect
     numbers, e.g. for Edna's phone number. (Why are they selling a cracked
     version anyway?!)
   - Added support for Low quality music in Macintosh version of Loom.
   - Improved Digital iMUSE accuracy for Full Throttle and The Curse of Monkey Island. These improvements also fix
     several audio related bugs for both games.
   - Fixed a very old regression in the walk code for Full Throttle which softlocked the game.
   - Improved the accuracy of the walk code for The Dig and The Curse of Monkey Island.
   - Fixed a bug in The Curse of Monkey Island which prevented, during the cannon minigame in Part 1, the destruction
     of one of the three destroyable turrets in the fort.
   - Added animated cigar smoke to the close-up of captain Smirk in the CD
     version of Monkey Island 1. It was present in earlier versions.
   - Restored some missing Lemonhead lines in the English, Italian, German and Spanish CD
     versions as well as the English Macintosh, FM-Towns and Sega CD versions
     of Monkey Island 1.
   - Made the clock tower in Monkey Island 1 behave the same in the CD version
     as in earlier versions, i.e. after examining it you have to leave the
     room and come back again for its description to change.

 Tinsel:
   - Enabled the Return to Launcher feature.

 Titanic:
   - Fixed crashes when asking bots what I should do.

 TsAGE:
   - Added support for uninstalled floppy version.

 Stark:
   - Added support for Hungarian translation.

 Supernova:
   - Added Italian translation for part 1.

 Sword25:
   - Made the extracted version working.
   - Fixed crash when selecting Croatian language.

 Ultima:
   - Ultima 4: Added several debugger commands.
   - Ultima 4: Allow Enter key to exit ZStats display.
   - Ultima 8: Fixed several animation bugs for events and objects.
   - Ultima 8: Restored original text for the Spell of Resurrection book.

 Xeen:
   - Fixed occasional border corruption during fights.
   - Improvements to cutscenes to better match the original games.
   - Fixes for character selection, deselection, and dismissing to better match original.
   - Added support for Russian version.

 AmigaOS port:
   - Added native system file browser feature.
   - Re-activated nuked OPL Adlib driver.

 Big-endian ports:
   - Fixed crashes or rendering issues with the Blazing Dragons, Duckman and
     Full Pipe games.

 iOS port:
   - Fixed using arrow keys on physical keyboard in iOS 15.
   - Fixed rotating the device while ScummVM is inactive.
   - Added support for upside down portrait orientation.

 macOS port:
   - Added support for Dark Mode.
   - Use OpenGL renderer by default, providing better support for HiDPI displays.

 MorphOS port:
   - Added native system file browser feature.
   - Added Cloud feature.
   - Re-activate nuked OPL Adlib driver.
   - Added CAMD MIDI driver support.

  Windows port:
   - Use OpenGL renderer by default, providing better support for HiDPI displays.

#### 2.2.0 "Interactive Fantasy" (2020-09-27)

 New games:
   - Added support for Blazing Dragons.
   - Added support for Griffon Legend.
   - Added support for Interactive Fiction games based on the following engines:
     ADRIFT (except for version 5), AdvSys, AGT, Alan 2 & 3,
     Archetype (newly reimplemented for Glk from the original Pascal sources),
     Hugo, JACL, Level 9, Magnetic Scrolls, Quest, Scott Adams,
     ZCode (all ZCode games except the Infocom graphical version 6 games).
     Currently, more than 1600 games are detected and supported.
   - Added support for Operation Stealth.
   - Added support for Police Quest: SWAT.
   - Added support for English translation of Prince and the Coward.
   - Added support for Ultima IV - Quest of the Avatar.
   - Added support for Ultima VI - The False Prophet.
   - Added support for Ultima VIII - Pagan.

 New ports:
   - MorphOS port got a major rewrite.

 General:
   - Autosaves are now supported for all the engines.
   - Errors are more likely to open the debugger, and be displayed, than just crash ScummVM.
   - Games are sorted in GUI ignoring the articles.
   - Now Hebrew is displayed correctly in GUI (requires FriBiDi library).
   - Updated the Roland MT-32 emulation code to Munt 2.4.0.
   - Added option to select the default voice for ports that support Text-to-Speech.
   - Added support for Discord Rich Presence integration on supported platforms (Windows Vista+, macOS 10.9+ 64 Bit).
   - Major improvements to the keymapper.
   - Games are now recognised by engineid:gameid combination.

 BBVS:
   - Added support for the demo, available from our website.
   - Added support for the Loogie minigame demo.

 Dreamweb:
   - Added support for Russian fan-translation.
   - Fixed animation speed.

 Illusions:
   - Fixed subtitle speed (set it to max for good speed).
   - Added support for Russian Duckman.

 Kyra:
   - Added support for the SegaCD version of Eye of the Beholder I (with CD-Audio, animated
     cutscenes and map function).
   - Added support for the PC-98 version of Eye of the Beholder I.
   - Added support for the Spanish versions of Eye of the Beholder I and II, Legend of
     Kyrandia 1 (CD-ROM fan translation) and Legend of Kyrandia 2 (floppy version and
     CD-ROM fan translation). Fix Spanish Lands of Lore support (floppy version and
     CD-ROM fan translation).

 Lab:
   - Fixed sound looping in some rooms.

 Neverhood:
   - Added support for bigger demo, available from our website.

 Prince:
   - Fixed inventory item descriptions display.
   - Added English translation.
   - Fixed bug with infinite loop when looking at some objects.

 Queen:
   - Fixed loading a save game from the launcher.
   - Fixed random long delays when starting a game.

 SCI:
   - Major improvements to Amiga and Mac sound drivers.
   - Improved MIDI playback and fixed many audio issues.
   - Fixed 30 original script bugs in ECO2, GK1, KQ4, KQ5, KQ6, KQ7, LB1, LONGBOW,
     PHANT2, QFG1, QFG3, QFG4, SQ5 and SQ6.
   - Fixed a script bug responsible for rare and random lockups in most Sierra
     games between 1992-1996.
   - Added support for Inside the Chest / Behind the Developer's Shield.
   - Added support for German LSL6-Floppy.
   - Added support for Hebrew Torin's Passage.
   - Added support for Italian Lighthouse.
   - Added support for Polish KQ5, LSL2, LSL3, LSL5 and LSL6-Floppy.
   - Fixed Russian LSL1 error when hailing a taxi.
   - Fixed Phantasmagoria 2 error when attempting easter eggs.
   - Fixed QFG3 auto-saves.
   - Fixed QFG4 and Shivers save game thumbnails being obscured by control panels.
   - Fixed a random crash in the Windows version when exiting a game.
   - Added support for Roland D-110 sound driver.
   - The "Prefer digital sound effects" checkbox works correctly now for SCI01/SCI1 games.

 SCUMM:
   - Fixed palette issues leading to incorrect colors in MM NES intro and dark rooms.
   - Fixed the rendering of the flashlight in MM NES to match the original version.
   - Replaced the existing NES palette to a more accurate NTSC palette based on Mesen.
   - Added a new GUI option to switch to an alternative NES palette based on the NES Classic.
   - Improved colors in Apple //gs Maniac Mansion.
   - Fixed crash when entering garage in Apple //gs Maniac Mansion.
   - Added support from Classic Full Throttle from Remastered release.

 Supernova:
   - Improved English translation.

 Sky:
   - Fixed syncing of music volume between native settings panel and ScummVM configuration.

 Sword1:
   - Added support for localized menus in Novy Disk Russian Trilogy release.
   - Added support for Spanish playable Demo available from our website.

 Sword2:
   - Added support for Spanish playable Demo available from our website.

 Titanic:
   - Fixed Barbot crash after asking what else he needs.

 Wintermute:
   - Added subsystem for tracking achievements, implemented for 10+ games.

 Xeen:
   - Add missing sprite drawer for enemies hit by Energy Blast.
   - Fixed freeze due to bad mob data at the top of Witches Tower.
   - Fix crash loading some savegames directly from the launcher.
   - Fix curing the well in Nightshadow.
   - Fix loading of wall items from savegames.
   - Fix U/D keys not working on Quests dialog.
   - Fix incorrect mirror destination for Dragon Tower.
   - Fix crash reading book in Great Pyramid safe.
   - Prevent attributes from going negative.
   - Fix border faces animation during Clairvoyance.

 ZVision:
   - Fixed regression in the safe puzzle in Zork Nemesis: The Forbidden Lands.
   - Fixed getting perfect score in Zork: Grand Inquisitor.

 Android port:
   - Added support for immersive fullscreen mode.
   - Improved keyboard and mouse input.

 iOS port:
   - Fixed unsupported graphic mode for some games (such as SCI games with high
     quality video scaling enabled).
   - Removed Quit button to follow the iOS design guidelines.
   - Removed virtual keyboard input assistant bar. In particular this means that we
     no longer see a bar at the bottom of the screen when using an external keyboard.
   - Added save of current game state if possible when switching to a different task
     and restore game state when returning to the ScummVM task.

 Linux port:
   - Added option to use the system file browser instead of the ScummVM file browser.

 MacOS X port:
   - Fixed blurry on Retina screens. Unfortunately this required dropping support
     for the dark theme for window decorations.
   - Fixed Taskbar icon display when running a game (this was broken since ScummVM
     1.9.0).

 RISC OS port:
   - Added a VFP optimized build for newer hardware.

 Windows port:
   - Added support for using additional OneCore voices for text to speech.
   - Active support for Windows Vista and lower is now discontinued. While we still provide
     builds for those systems in the foreseeable future, some newer features might be missing.

#### 2.1.2 "Fixing Powerful Windows" (2020-03-31)

 Windows port:
   - Fixed an issue with the Sparkle updater which lead to an infinite update loop.

 MacOS X port:
   - Fixed application freeze on start on Mac OS X 10.5 and older.
   - Fixed application icon on Mac OS X 10.5 and older.


#### 2.1.1 ":More sheep:" (2020-01-31)

 General:
   - Fixed crash when switching certain languages in GUI.
   - Fixed ESC erroneously saving the changes in the options dialog.
   - Improvements in FM-TOWNS/PC-98 audio.
   - Improved Greek language support in the GUI.

 Networking:
   - Improved error handling.
   - Only download saves when necessary.

 Bladerunner:
   - Fixed buggy savestate in some scenes.
   - Added engine checkbox to target 120fps.
   - Fixed thumbnail portability.
   - Use virtual keyboard on save screen on relevant platforms.

 Kyra:
   - Fixed EOB1-Amiga ending sequence (which would play only if you achieved all bonus quests).
   - Fixed monster random item drop chance in EOB1.
   - Added handling for the secret potion in the Legend of Kyrandia 2.
   - Fixed sound issues in the Legend of Kyrandia 2.
   - Fixed graphics glitches in Legend of Kyrandia 1 and 3, EOB1 and EOB2-FM-TOWNS.

 Mohawk:
   - Persist changes in game options across sessions.

 Queen:
   - Fixed regression with the display of the bellboy dialogue.

 SCI:
   - Numerous game script fixes in CAMELOT, ECO2, GK1, GK2,
     KQ7, PHANT1, PQ1VGA, QFG3, QFG4, SQ5 and SQ6.
   - Implement horizontal and FM-TOWNS type screen shake.
   - Added support for Phantasmagoria 1 censored mode.
   - Added support for Polish LSL7.
   - Added support for Italian GK2.
   - Added support for Portuguese GK2.
   - Added support for Russian KQ7.
   - Added support for Russian SQ1VGA.
   - Added support for GK2 fan-made subtitle patches.

 SCUMM:
   - Added support for Pajama2 (UK release).

 Supernova:
   - Hooked F5 to the Main Menu.

 Toltecs:
   - Added Czech version support.
   - Fixed exiting from game menus when returning to the launcher.

 Wintermute:
   - Added several missing game variants and demos to the detection tables.
   - Fixed regression with stack handling.
   - Fixed the behavior of edit boxes.
   - Improved support for Chinese language game variants.

 Xeen:
   - Fixed display of gold and gem amounts on the Switch.
   - Fixed tavern exit locations in Swords of Xeen.
   - Fixed crash loading Deep Mine Alpha in World of Xeen CD.

 GUI:
   - MIDI setting tabs are no longer shown if a game has no music at all.

 All ports:
   - Fixed screen filling in non-paletted screen modes.

 3DS port:
   - Major improvements.

 AmigaOS4 port:
   - Minor tweaks (stack cookie, build automation).

 Android port:
   - Improved filesystem navigation.
   - Proper handling of HiDPI displays.
   - Improved keyboard support.

 iOS port:
   - The home indicator is now automatically hidden on iPhone X and later models.

 MacOS X port:
   - Follow the OS dark theme for window decorations.

 RISC OS port:
   - Fixed crash when accessing an unavailable drive.
   - Reduced the required DigitalRenderer version.

 Switch port:
   - Added cloud integration.

 OpenPandora port:
   - Minor improvements.

 Windows:
   - Added Text-to-Speech support.
   - Fix screenshots with Unicode paths.


#### 2.1.0 "Electric Sheep" (2019-10-11)

 New games:
   - Added support for Blade Runner.
   - Added support for Duckman: The Graphic Adventures of a Private Dick.
   - Added support for Hoyle Bridge.
   - Added support for Hoyle Children's Collection.
   - Added support for Hoyle Classic Games.
   - Added support for Hoyle Solitaire.
   - Added support for Hyperspace Delivery Boy!
   - Added support for Might and Magic IV - Clouds of Xeen.
   - Added support for Might and Magic V - Darkside of Xeen.
   - Added support for Might and Magic - World of Xeen.
   - Added support for Might and Magic - World of Xeen 2 CD Talkie.
   - Added support for Might and Magic - Swords of Xeen.
   - Added support for Mission Supernova Part 1.
   - Added support for Mission Supernova Part 2.
   - Added support for Quest for Glory: Shadows of Darkness.
   - Added support for The Prince and the Coward.
   - Added support for Versailles 1685.

 New ports:
   - Added Nintendo Switch port.

 General:
   - Improved GUI rendering and overall GUI performance.
   - Added stretch mode option to control how the display is stretched to the
     window or screen area.
   - Fixed incorrect cursor movement when it's controlled using the keyboard.
   - Updated the Roland MT-32 emulation code to Munt 2.3.0.
   - Improved unknown game variants reporting.
   - Enabled cloud support.
   - Added Text to Speech capabilities for better accessibility on some platforms.

 ADL:
   - Improved color accuracy.
   - Added a TV emulation mode.
   - Added support for the WOZ disk image format.

 Drascula:
   - Fixed loading game from launcher when the game had been saved in chapter 1.

 Full Pipe:
   - Fixed playtime not being restored when loading a savegame.
   - Fixed a bug that leads to enormous memory consumption in scene 22.

 Kyra:
   - Added support for the Amiga version of Eye of the Beholder I + II.
   - Added support for the FM-Towns version of Eye of the Beholder II.
   - Several bug fixes.

 MOHAWK:
   - Added a main menu for the 25th anniversary release of Myst ME.
   - Repurposed the landing menu as a main menu for the 25th anniversary
     release of Riven.
   - Added autosave to slot 0 to Myst and Riven.
   - Added keyboard shortcuts for loading and saving as documented in the game
     manual to Myst and Riven.
   - Fixed a crash caused by the observatory viewer random position going out
     of bounds in Myst.
   - Fixed a crash caused by Jungle Island flies going out of bounds in Riven.
   - Fixed missing end credits for the Polish version in Riven.
   - Improved usability for some puzzles in Myst and Riven.
   - Fixed various crashes, graphics glitches, and sound imperfections in Myst
     and Riven.

 Mortevielle:
   - Added speech synthesis on some platforms.

 SCI:
   - Added LarryScale, a high quality cartoon scaler for Leisure Suit Larry 7.
   - Fixed over 100 original game script bugs in CAMELOT, ECO1, ECO2,
     FREDDYPHARKAS, GK1, HOYLE5, ICEMAN, KQ6, LB1, LB2, LONGBOW, LSL6,
     MOTHERGOOSE256, PQ3, PQ4, QFG1VGA, QFG4, and SQ4.
   - Fixed a bug in version 2.0.0 that prevented the Macintosh versions of
     Freddy Pharkas, King's Quest 6, and Quest for Glory 1 (VGA) from loading.
   - Fixed a crash in the Macintosh version of Freddy Pharkas when picking up
     the shovel which makes the game completable.
   - Fixed loading autosave games.

 SCUMM:
   - Implemented lipsync for v6 and v7+ games.
   - Improved Audio quality in Humongous Entertainment games by using the Miles AdLib driver.
   - Fixed possible stack overflows in The Dig and Full Throttle.
   - Fixed original speech glitch on submarine in Indiana Jones and the Fate of Atlantis.
     Users need to recompress their monster.sou using an up-to-date version of scummvm-tools
     for this to take effect when using compressed audio.
   - Fixed an issue in the wig maker room in the German version of SPY Fox 3: Operation Ozone
     which makes the game completable.
   - Added sound driver for the Amiga versions of Monkey Island 2 and Indiana Jones
     and the Fate of Atlantis.

 Sherlock:
   - Fixed crash in Spanish version talking to lady in Tailor shop.

 SKY:
   - Added workaround for original game bug to improve intro and not cut off images which exist
     as fullscreen (320x200px) in the game data files.

 Tinsel:
   - Fix loading Discworld 1 savegames from the launcher where Rincewind had a held item.
   - Script patch for hang in Discworld 1 GRA using items on Temple big hammer.
   - In Discworld 1, Held items being released that were never in the Luggage or Rincewind's inventory
     will now be automatically dropped into the Luggage rather than being lost.

 Titanic:
   - Fixed bug in entering floor numbers numerically that could crash the game.
   - Fixed parser not getting properly reset across sentences in a conversation.
   - Fixed endless busy cursor on Titania closeup when brain slots are incorrectly inserted.
   - Fixed loading saves in front of Barbot could cause him to go into an infinite animation loop.
   - Fixed crash asking Parrot who sabotaged the ship.

 Tucker:
   - Fixed multiple graphic issues in Bud Tucker in Double Trouble.
   - Fixed multiple issues with font and subtitle rendering.
   - Fixed dentist music in mall being played incorrectly.
   - Fixed wrong sound effects being played.
   - Fixed a bug that made the bubbles in the Plugs Shop not always visible.
   - Fixed a missing animation when Ego and Billie are on the boat.
   - Fixed a bug that caused a dead end in the second museum scene.
   - Bud is no longer able to walk outside the walkable area when visiting the club.
   - Bud is also no longer able to walk through closed doors.
   - Added mouse wheel support for inventory scrolling.
   - Allow skipping of speech.
   - Improved savegame handling and added support for autosaves.

 ZVISION:
   - Fixed graphical glitch in Zork: Grand Inquisitor.
   - Packaged the required fonts with ScummVM.
   - Enabled hires movies in the DVD version of Zork: Grand Inquisitor (requires
     libmpeg2 and libac52).

 Android port:
   - Rewrote to make use of the OpenGL Graphics Manager.
   - Added a button to show the virtual keyboard.
   - Implemented clipboard support.
   - Use the dedicated GUI option for enabling the touchpad mode.
   - Added code for searching accessible external media.

 iOS port:
   - Added support for Smart Keyboard.
   - Added three-fingers swipe gestures to simulate arrow keys.
   - Added pinch in and out gestures to show and hide the keyboard.
   - Added scrollable accessory bar above the keyboard with keys not present on the keyboard.

 macOS port:
   - Added option to use the ScummVM file browser instead of the system file browser.
   - Added access to documentation from the Help menu.

 PS Vita port:
   - Implemented front touch and optional rear touch controls.

 PSP port:
   - Implemented aspect ratio correction.
   - Improved smoothness of mouse pointer motion.
   - Added mouse pointer speed and analog nub deadzone settings.

 SDL ports (including Windows, Linux, macOS):
   - Added support for game controllers.
   - Added support for adding games via Drag and Drop.

 Windows port:
   - Added option to use the system file browser instead of the ScummVM file browser.

 RISC OS port:
   - Added a StrongHelp manual.
   - Added error reporting using !Reporter.


#### 2.0.0 (2017-12-17)

 New Games:
   - Added support for Full Pipe.
   - Added support for Hi-Res Adventure #3: Cranston Manor.
   - Added support for Hi-Res Adventure #4: Ulysses and the Golden Fleece.
   - Added support for Hi-Res Adventure #5: Time Zone.
   - Added support for Hi-Res Adventure #6: The Dark Crystal.
   - Added support for Riven.
   - Added support for Starship Titanic English & German.

 New Games (Sierra SCI2 - SCI3):
   - Added support for Gabriel Knight.
   - Added support for Gabriel Knight 2.
   - Added support for King's Quest VII.
   - Added support for King's Questions.
   - Added support for Leisure Suit Larry 6 (hires).
   - Added support for Leisure Suit Larry 7.
   - Added support for Lighthouse.
   - Added support for Mixed-Up Mother Goose Deluxe.
   - Added support for Phantasmagoria.
   - Added support for Phantasmagoria 2.
   - Added support for Police Quest 4.
   - Added support for RAMA.
   - Added support for Shivers.
   - Added support for Space Quest 6.
   - Added support for Torin's Passage.

 New ports:
   - Added PSP Vita port.
   - Added RISC OS port.

 General:
   - Added bilinear filtering option for SDL2 fullscreen mode.
   - Fixed a bug that caused a crash in the options dialog of the GUI.
   - Added a command-line option to automatically scan for supported games in
     the current or a specified directory.
   - Added possibility to apply changes in the options dialog without closing
     the dialog.
   - Added support for on-the-fly GUI language switching.
   - Updated Munt MT-32 emulation code to version 2.0.3.
   - Improved handling of joysticks.
   - Improved audio latency.
   - Improved management of the ScummVM window in games that switch display
     modes.
   - Fixed list view drawing over text above it (for example in the save dialog).
   - Changed location where screenshot are saved. This fixes issues when scummvm
     is installed in a read-only directory. Also added setting to allow changing
     this location.
   - Changed screenshot format to png.
   - Fixed multithreading issue that could cause a crash in games using MP3 audio.

 ADL:
   - Fixed application freeze when reading sign in rocket in Mission Asteroid.

 AGI:
   - Fixed game script blocking forever after loading a savegame that was saved
     while music was playing (this could happen for example in Police Quest 1
     poker back room.
   - Fixed cursor behaviour in Manhunter.
   - Fixed nightclub arcade sequence speed for Manhunter Apple IIgs version.
   - Reduced fastest game speed to a maximum of 40 FPS to ensure the games do
     not run too fast.

 AGOS:
   - Fixed subtitle speed setting in the Hebrew version of Simon the Sorcerer 1.

 Composer:
   - Added save/load from General Main Menu.
   - Fixed the detection for the French Gregory.
   - Added detection for German Baba Yaga.

 Cruise:
   - Fixed font rendering.

 Drascula:
   - Fixed bug that made it impossible to talk to the drunkard more than once in the inn.
   - Added handling of the master volume and fix volume synchronization between
     the game and ScummVM options.
   - Added possibility to load and save games using GMM.

 Dreamweb:
   - Fixed crash when collecting last stones under church.
   - Fixed detection of Italian CD release.

 Kyra:
   - Fixed a buffer overflow in Lands of Lore.
   - Fixed crash due to missing palette data for Legend of Kyrandia floppy version.

 MADE:
   - Fixed badly distorted sound (bug #9753).

 MADS:
   - Fixed a bug that caused a crash after starting Rex Nebular and the Cosmic Gender Bender.
   - Fix rare crash that can happen when Rex is first locked up

 MOHAWK:
   - Added patch to the original data files to correct the vault access
     instructions in Myst ME.
   - Fixed situations where Myst could appear to be unresponsive.
   - Reworked sound handling in Myst to be more accurate.
   - Fixed crash in Myst piano puzzle.

 Neverhood:
   - Fixed crash in musical hut in Russian DR version.
   - Fixed late game notes crash in Russian DR version.

 Pegasus:
   - Fixed loading a game from the launcher after returning to the launcher.
   - Ignored events occuring while the GUI is visible. This for example fixed an
     issue where closing the GMM using Escape would also opens the game's own
     menu.
   - Fixed several crashes when toggling the shared screen space.
   - Improved performances when fading screen.

 SAGA:
   - Fixed crash when using the give verb on an actor in IHNM.
   - Fixed Gorrister invisible and stuck when reloading at mooring ring in IHNM.
   - Fixed the conversation panel background color in IHNM.
   - Added support French Fan Translation of Inherit the Earth.

 SCI:
   - Fixed a script bug in Laura Bow 2: Dagger of Amon Ra that made it impossible
     to exit the party room with the large golden head inside the museum (room 350).
     This bug is also present, when using the original interpreter.
   - Improved startup speed when using the MT-32 emulator.
   - Improved handling of MT-32 reverb in SCI0 games.
   - Improved selection of synthesized sound effects in SCI0 games.
   - Improved selection of digital audio in SQ4.
   - Improved resource bounds checking.
   - Improved error handling of corrupt MIDI data.
   - Fixed slow leak of small amounts of data into save games over time.
   - Fixed broken day/night cycle in QFG3.
   - Fixed a script bug in Police Quest 3 to now grant 10 points when giving the
     locket to Marie. Now it's possible to beat the game with a perfect score.
     This bug is also present when using the original interpreter.
   - Fixed various other script bugs.
   - Improved audio volume and settings synchronization.

 SCUMM:
   - Fixed crash in amiga games.
   - Fixed two soundtracks playing at once in Monkey Island 2.
   - Fixed Caponians dont disguise after using blue crystal in Zak McKracken.
   - Fixed Dr. Fred facing wrong way in lab cutscene in Maniac Mansion.
   - Fixed actors being drawn one line too high in V0 and V1 games.
   - Fixed Purple Tentacle appears in Lab Entry after being chased out in maniac Mansion.
   - Fixed power not turning back on in Maniac Mansion when entering the lab
     while Dr. Fred has the power off.
   - Fixed actors skipping between certain walk-boxes in Maniac Mansion.

 Sherlock:
   - Fixed detection for Italian fan translation of Serrated Scalpel.

 Sky:
   - Fixed collision detection.

 Sword1:
   - Added thumbnail when saving from in-game dialog.
   - Fixed audio and subtitles settings being changed when open the load/save
     in-game dialog.

 Tinsel:
   - Fixed some Discworld 2 text/voice not displaying & playing all the way through.
   - Fix crash in in-game save menu when all slots are used with long names

 TsAGE:
   - Fixed regression preventing animations in Return to Ringworld from playing.
   - Fixed display issues in Return to Ringworld Demo.
   - Fixed loading Return to Ringworld savegames with unreferenced dynamic objects.
   - Fixed deadlock in audio code.
   - Fixed crash on Return to Launcher.

 Voyeur:
   - Fixed backgrounds not showing for static rooms.
   - Fixed playback of audio events on VCR.
   - Fixed exiting game from the VCR screen.
   - Added workaround for original game bug using invalid hotspot Ids

 macOS port:
   - Added support for selecting any connected MIDI devices instead of automatically
     using the first one.
   - Improved behaviour of the file browser.


#### 1.9.0 "Myst-ery U.F.O.s release" (2016-10-17)

 New Games:
   - Added support for Myst.
   - Added support for Myst: Masterpiece Edition.
   - Added support for U.F.O.s.
   - Added support for Hi-Res Adventure #0: Mission Asteroid.
   - Added support for Hi-Res Adventure #1: Mystery House.
   - Added support for Hi-Res Adventure #2: The Wizard and the Princess.

 General:
   - Fixed audio corruption in the MS ADPCM decoder.
   - Fixed audio pitch in the CMS/GameBlaster emulation.
   - Switched SDL backend to SDL2 by default. SDL1 is still a fallback.

 AGI:
   - Added support for Hercules rendering. Both green and amber modes are
     supported.
   - Added support for the Hercules high resolution font. The font is also
     usable outside of Hercules rendering.
   - Added optional "pause, when entering commands" feature, that was only
     available in the original interpreter for Hercules rendering.

 Beneath a Steel Sky:
   - Fixed a bug that could possibly make the game unfinishable due to a
     wrong animation for Officer Blunt that makes further interaction with
     this character impossible.

 Gob:
   - Fixed graphical issues in Gobliiins (EGA version).

 Kyra:
   - Updated Italian EOB1 translation.
   - Fixed a bug that caused a crash in Lands of Lore.

 SCI:
   - Fixed a missing dialog line in QfG3 which awards the player with 3 additional
     points. This is a bug in the original game interpreter. Due to this bug,
     it was not possible to get all points in the original game.
   - Fixed a bug in Space Quest 1 that caused issues with the spider droid.
   - Fixed a bug in Laura Bow: The Colonel's Bequest that could cause a lock-up when
     interacting with the armor in room 37 (main house, downstairs). This bug is also
     present in the original game.
   - Fixed auto-saving in the fan-made Cascade Quest.
   - Fixed a game bug in the Conquests of Longbow scripts that could cause crashes in Sherwood Forest.
   - Added support for the ImagiNation Network (INN) demo.

 SCUMM:
   - Fixed missing translations in the in-game quit and restart dialogs in Pajama Sam 1.
   - Fixed visual glitches in DOTT that occurred after loading a savegame with the stereo
     in Green Tentacle's room turned on.
   - Improved timing and pathfinding in Maniac Mansion (C64 and Apple II versions).
   - Added support for the Dutch demo of Let's Explore the Airport with Buzzy.

 Sherlock:
   - Fixed a bug that could cause a crash in The Case of the Serrated Scalpel.
   - Fixed an issue with item hotspots in The Case of the Serrated Scalpel.
   - Fixed a bug that caused game lockups in the inventory of The Case of the Rose Tattoo.

 Amiga port:
   - Added support for AmiUpdate autoupdates.

 Linux port:
   - Added basic support for the snap packaging system.

 Windows port:
   - Fixed taskbar support on Windows 10 onwards.
   - Fixed keymapping for non-QWERTY keyboards.
   - Added support for WinSparkle updater.


#### 1.8.1 "Where Is Your Android?" (2016-05-25)

 New ports:
   - Added Nintendo 3DS port.
   - Added Android SDL port.

 General:
   - Removed TESTING flag from several supported games.
   - Added Chinese Pinyin translation.
   - Fixed cursor stuttering in the launcher that occurred on some systems.

 BBVS:
   - Fixed game restart.

 CinE:
   - Fixed sound effect loading.

 Drascula:
   - Fixed text alignment to be faithful to the original.
   - Fixed character walking off screen.
   - Fixed loading savegames in the Pendulum scene.
   - Fixed wrong background for inventory items during chapter 6 in the
     Spanish version.
   - Fixed animations speed (they were running two times slower than in the
     original engine).
   - Fixed noise at start and/or end of speech. This was most noticeable
     with the Spanish speech.
   - Fixed delay when interacting with the verb menu and the inventory.
   - Fixed possibility to pick up the axe in the castle multiple times.

 Gob:
   - Fixed lock up for some games during sound initialization.

 KYRA:
   - Fixed potential crash when using swamp snake potion on the rat in Hand
     of Fate. (NOTE: This fix was included in version 1.8.0, but it was not
     added to the NEWS file).
   - Fixed missing voice reactions when hitting enemies in CD version of
     Lands of Lore.

 Lab:
   - Fixed lock-up during ending sequence.
   - Improved internal game controls.
   - Fixed lock-up during some in-game animations.

 SAGA:
   - Fixed user interface colors in the French and German versions of I Have No
     Mouth and I Must Scream.

 SCI:
   - Make cursor workarounds work properly on OpenPandora (and other devices, that
     support touch screen and analog sticks/mouse at the same time).
   - Script patch to fix broken ending battle in multilingual King's Quest 5
     (French, German + Spanish versions are all broken).
   - Fixed invalid memory access, when loading broken King's Quest 5 credit music track.
   - Fixed lowres/hires issues in King's Quest 6 when saving, changing the lowres/hires
     setting and restoring the saved game afterwards.

 SCUMM:
   - Fixed detection of Maniac Mansion from Day of the Tentacle in the Windows
     version of ScummVM.
   - Fixed a sound effect not stopping in Loom EGA with AdLib.

 Broken Sword 2.5:
   - Added option to use English speech instead of German one when no speech is
     available for the selected language.
   - Fixed resource releasing on game exit.
   - Fixed game restart after language change in-game.
   - Fixed flickering in main Menu.
   - Fixed long save time on Windows.

 Windows port:
   - Fixed bug in MIDI device listing affecting cases where MIDI devices were
     not usable.

 Mac OS X port:
   - Dock menu for ScummVM now lists recently played games when ScummVM is
     not running and allows starting those games.
   - Enabled Sparkle application updater.

 GCW0 port:
   - Improved support for built-in ScummVM documentation.


#### 1.8.0 "Lost with Sherlock" (2016-03-04)

 New Games:
   - Added support for Rex Nebular and the Cosmic Gender Bender.
   - Added support for Sfinx.
   - Added support for Zork Nemesis: The Forbidden Lands.
   - Added support for Zork: Grand Inquisitor.
   - Added support for The Lost Files of Sherlock Holmes: The Case of the
     Serrated Scalpel.
   - Added support for The Lost Files of Sherlock Holmes: The Case of the Rose
     Tattoo.
   - Added support for Beavis and Butthead in Virtual Stupidity.
   - Added support for Amazon: Guardians of Eden.
   - Added support for Broken Sword 2.5: The Return of the Templars.
   - Added support for The Labyrinth of Time.

 New Ports:
   - Added Raspberry Pi port.
   - Added GCW0 port.

 General:
   - Updated Munt MT-32 emulation code to version 1.5.0.

 SDL:
   - Alt-x no longer quits ScummVM. Use Cmd-q/Ctrl-q/Ctrl-z instead; see README.
   - On POSIX systems we now follow the XDG Base Directory Specification for
     placement of files for users. This effectively results in new locations
     for our configuration file, our log file, and our default savegame path.
     We still support our previous locations. As long as they are present, we
     continue to use them. Please refer to the README for the new locations.
     File locations on Mac OS X are not affected by this change.

 3 Skulls of the Toltecs:
   - Improved AdLib music support.

 AGI:
   - It is now possible to disable mouse support (except for Amiga versions
     and fanmade games, that require a mouse).
   - Fixed PCjr sound volumes.
   - Major rewrite of graphics subsystem.
   - Support for Apple IIgs, Amiga + Atari ST transitions, fonts and mouse
     cursors. The Atari ST 8x8 system font is not included with ScummVM.
   - Added ability to make for example a PC version look like an Apple IIgs
     version. This includes palette, cursor, transition and even font. Just
     set corresponding render mode.
   - Fixed Apple IIgs game versions running too fast.
   - Added support for automatic saving/restoring used by Mixed Up Mother Goose.
   - Removed forced two second delay on room changes; replaced with heuristic.
   - Fixed certain key bindings breaking after saving/reloading.

 AGOS:
   - Fixed arpeggio effect used in music of Amiga version of Elvira 1.
   - Fixed loading and saving progress in the PC version of Waxworks.
   - Fixed verb area been removed in Amiga versions of Simon the Sorcerer 1.
   - Added Accolade AdLib & MT-32 music drivers for the games:
     Elvira 1, Elvira 2, Waxworks and Simon the Sorcerer 1 demo.
   - Added Simon the Sorcerer 1 AdLib output. This vastly improves the AdLib
     output and makes it closer to the original.

 Broken Sword 1:
   - Fixed Macintosh version speech when running on big endian systems.
   - Fixed loading from Main Menu in bull's head scene, and maybe other scenes.

 CinE:
   - Added support for music in CD version of Future Wars.

 MADE:
   - Improved AdLib music support in Return to Zork.

 SAGA:
   - Improved AdLib music support.

 SCI:
   - Handling of music priority has been greatly improved.
   - A lot of fixes for original game script bugs that also occurred when
     using the original interpreter. This affects the following games:
     KQ6 (Dual Mode), LSL5, PQ1, QfG1 (EGA), QfG1 (VGA), QfG2, QfG3, SQ1,
     SQ4 (CD).
   - Restoring from the ScummVM in-game menu should now work all the time.
   - Improved support for Japanese PC-9801 games.
   - Default to hi res version of KQ6, changeable using engine option.

 SCUMM:
   - Major improvements to Korean versions text rendering.
   - Implemented original Maniac Mansion v0-v1 walking code.
   - It is now possible to play Maniac Mansion from within Day of the
     Tentacle, with a few caveats. See README for details.
   - Alt-x can now be used to quit SCUMM games on all platforms.
   - Improved lip sync animation in later HE games.

 Tinsel:
   - Improved AdLib music support in Discworld 1.


#### 1.7.0 "The Neverrelease" (2014-07-21)

 New Games:
   - Added support for Chivalry is Not Dead.
   - Added support for Return to Ringworld.
   - Added support for The Neverhood.
   - Added support for Mortville Manor.
   - Added support for Voyeur.

 General:
   - Updated Munt MT-32 emulation code to version 1.3.0.
   - Switched from our custom JPEG and PNG decoders to libjpeg(-turbo) and
     libpng, which are faster and can handle more images.
     (NOTE: The change to libpng was done in version 1.6.0, but it was not
     added to the NEWS file).
   - Added generic OpenGL (ES) output (based on GSoC Task).
   - The GUI can now be rendered in 32-bits.
   - The build system has been changed to be more modular and easier to add new
     engines.

 SDL:
   - Added OpenGL graphics mode based on our generic OpenGL output. This
     allows for arbitrary output sizes. However, it does not support special
     filters like AdvMAME, HQ, etc.

 AGOS:
   - Added mouse wheel support for inventory and save game lists.
   - Enabled verb name display in Simon the Sorcerer 2.
   - Fixed the Feeble Files loyalty rating in the English 4CD version. (This
     was apparently a bug in the original game. It is currently unknown if
     other versions still have the same problem.)

 Broken Sword 1:
   - Added back support for MPEG-2 videos.

 Broken Sword 2:
   - Added back support for MPEG-2 videos.

 CGE:
   - Added an option to enable "Color Blind Mode" to the ScummVM GUI.

 Gob:
   - Improved video quality in Urban Runner.

 Hopkins:
   - Added an option to toggle "Gore Mode" from the ScummVM GUI.
   - Fixed bug that could cause the music to stop prematurely.

 Pegasus:
   - Fixed several rare crashes and glitches.
   - Fixed multiple bugs carried over from the original binary.

 SCI:
   - Added support for the more detailed RAVE lip syncing data in the Windows
     version of King's Quest 6. Portraits should now be much more expressive
     when talking.
   - Added support for simultaneous speech and subtitles in the CD versions
     of Laura Bow 2 and King's Quest 6 (toggled either in-game with the new
     "Dual" audio state, or via the ScummVM audio options).
   - Fixed music fading.
   - Fixed several script bugs in Camelot, Crazy Nick's, Hoyle 3, QFG1VGA, KQ5,
     KQ6, LB2, LSL2, LSL5, Pharkas, PQ1VGA, SQ4, SQ5.
   - Improved the MIDI parser so that music event processing is done more
     properly.

 SCUMM:
   - Changed the saved game naming scheme of HE games to always contain
     the target name.
   - Fixed having multiple coaches in Backyard Football.
   - Improved AdLib support for Loom and Indiana Jones and the Last Crusade.
     This makes sound effects like, for example, the typewriter and waterfall
     in Indiana Jones and the Last Crusade sound like in the original.
   - Added support for the Steam versions of Indiana Jones and the Last
     Crusade, Indiana Jones and the Fate of Atlantis, Loom and The Dig. Both
     the Windows and the Macintosh versions are supported.

 TONY:
   - Savegames in Tony Tough now work on big-endian systems.

 Tinsel:
   - Discworld 1 and 2 no longer crash on big-endian systems.

 Android port:
   - Added experimental support for the OUYA console.

 PS2 port:
   - Added configurable TV modes: NTSC and PAL.
   - Added configurable graphics modes: SDTV progressive, SDTV interlaced, EDTV
     progressive and VESA.
   - Added a configuration option for the HDD partition used.
   - Added a configuration option for the IP address used.
   - Added a configuration option to toggle USB mass storage.

 Tizen port:
   - The BADA port has been merged/updated into Tizen.


#### 1.6.0 "+4 to engines" (2013-05-31)

 New Games:
   - Added support for 3 Skulls of the Toltecs.
   - Added support for Eye of the Beholder.
   - Added support for Eye of the Beholder II: The Legend of Darkmoon.
   - Added support for Hopkins FBI.
   - Added support for Tony Tough and the Night of Roasted Moths.
   - Added support for The Journeyman Project: Pegasus Prime.
   - Added support for the Macintosh version of Discworld 1.

 General:
   - Added a new save/load chooser based on a grid of thumbnails. This is only
     supported for resolutions bigger than 640x400. The old chooser is still
     available and used for games without thumbnail support. It is possible to
     select the old one as default too.
   - Rewrote VideoDecoder subsystem.
   - Added Galician translation.
   - Added Finnish translation.
   - Added Belarusian translation.
   - Using the mouse wheel on a slider widget now changes the value by the
     smallest possible amount. This is more predictable than the old behaviour,
     which was to change the value by "one pixel" which would sometimes not
     change it at all.
   - Updated MT-32 emulation code to latest munt project snapshot.
   - Added FluidSynth settings dialog, mainly for reverb and chorus settings.
   - Fixed crash on certain Smacker movies.

 Cine:
   - Improved audio support for Amiga and AtariST versions of Future Wars.
     Now music fades out slowly instead of stopping immediately. Sound
     effects are now properly panned, when requested by the game.

 CGE:
   - Soltys contains a puzzle requiring the ALT key to be pressed while clicking
     on an object. This puzzle has been disabled on devices not using this key.

 Drascula:
   - Resolved multiple UI issues with the original save/load screen.
   - Added advanced savegame functionality, including savegame timestamps and
     thumbnails and the ability to load and delete savegames from the launcher.
     It's now possible to use the ScummvM save/load dialogs.
   - The F7 key (previously unmapped) now always shows the ScummVM load screen.
     The F10 key displays either the original save/load screen, or the ScummVM
     save screen, if the user has selected to use the ScummVM save/load
     dialogs.

 Dreamweb:
   - Now that the game is freeware, there is a small extra help text showing
     the available commands in the in-game terminals when the player uses the
     'help' command. Previously, players needed to consult the manual for the
     available commands. Since this reference to the manual is a form of copy
     protection, this extra line can be toggled by the ScummVM copy protection
     command line option.

 Groovie:
   - Simplified the movie speed options, and added a custom option for The 7th
     Guest. Movie options are now "normal" and "fast", with the latter changing
     the movie speed in T7G to match the faster movie speed of the iOS version.
     The game entry might need to be readded in the launcher for the new setting
     to appear.

 SAGA:
   - Added music support for the Macintosh version of I Have No Mouth and, I
     Must Scream.

 SCUMM:
   - Implemented Monkey Island 2 Macintosh's audio driver. Now we properly
     support its sample based audio output. The same output is also used for
     the m68k Macintosh version of Indiana Jones and the Fate of Atlantis.
   - Improved music support for the Macintosh version of Monkey Island 1. It
     now uses the original instruments, rather than approximating them with
     General MIDI instruments, and should sound a lot closer to the original.
   - Added sound and music support for the Macintosh version of Loom.
   - Handle double-clicking in the Macintosh version of Loom.
   - Major bugfixes in INSANE (the Full Throttle bike fights).

 TOUCHE:
   - Added support for Enhanced Music by James Woodcock
     (http://www.jameswoodcock.co.uk/category/scummvm-music-enhancement-project/).


#### 1.5.0 "Picnic Basket" (2012-07-27)

 New Games:
   - Added support for Backyard Baseball 2003.
   - Added support for Blue Force.
   - Added support for Darby the Dragon.
   - Added support for Dreamweb.
   - Added support for Geisha.
   - Added support for Gregory and the Hot Air Balloon.
   - Added support for Magic Tales: Liam Finds a Story.
   - Added support for Once Upon A Time: Little Red Riding Hood.
   - Added support for Sleeping Cub's Test of Courage.
   - Added support for Soltys.
   - Added support for The Princess and the Crab.

 General:
   - Updated MT-32 emulation code to latest munt project snapshot. The emulation
     improved dramatically.
   - Implemented support for TrueType fonts via FreeType2 in our GUI. Along
     with it GNU FreeFont was also added to our modern theme. Note that not all
     ports take advantage of this.
   - Added Basque translation.
   - Added custom game and engine options in the AGI, DREAMWEB, KYRA, QUEEN,
     SKY and SCI engines. It is now possible to toggle these options via the
     Engine tab when adding or editing a configuration for a game. In most
     cases, you will have to run each game once or readd them all in ScummVM's
     launcher in order to get the custom options tab.
   - Improved predictive dialog look.
   - Various GUI improvements.

 Broken Sword 1:
   - Fixed incorrect sound effects in the DOS/Windows demo.
   - Added support for PlayStation videos.
   - Fixed missing subtitles in the demo.

 Broken Sword 2:
   - Added support for PlayStation videos.

 Cine:
   - Implemented Roland MT-32 output driver.

 Drascula:
   - Added Spanish subtitles in the Von Braun cutscene (#5372: no subtitles
     in scene with "von Braun").

 Gob:
   - Fixed a crash in Lost in Time.
   - Rewrote the AdLib player. Enabled the now working MDY player in
     Fascination and Geisha.

 SCUMM:
   - Added support for the Macintosh version of SPY Fox in Hold the Mustard.
   - Added a difficulty selection dialog for Loom FM-TOWNS.
   - Fixed graphical glitches in HE98 version of Pajama Sam's Lost & Found.

 iPhone port:
   - Changed "F5 (menu)" gesture to open up the global main menu instead.
   - Added support for custom cursor palettes, this makes the moderm theme use
     the red pointer cursor for example.
   - Added aspect ratio correction feature.
   - Implemented 16 bits per pixel support for games.

 Maemo port:
   - Added support for Nokia 770 running OS2008 HE.
   - Added configurable keymap.

 Windows port:
   - Changed default savegames location for Windows NT4/2000/XP/Vista/7.
     (The migration batch file can be used to copy savegames from the old
      default location, to the new default location).


#### 1.4.1 "Subwoofer Release" (2012-01-27)

 AGOS:
   - Fixed loading videos directly from InstallShield cabinets in the Windows
     version of the The Feeble Files.

 BASS:
   - Added support for Enhanced Music by James Woodcock
     (http://www.jameswoodcock.co.uk/?p=7695).

 Broken Sword 2:
   - Slight graphics improvement for PSX version.

 KYRA:
   - Fixed bug in the original Lands of Lore GUI which made ScummVM error out
     in the case the user did not have a contiguous save slot usage.
   - Add support for original DOS Lands of Lore save files (also applies to save
     files made with the GOG release).

 SCI:
   - Fixed race condition in SCI1.1 palette changes. This fixes an error in
     QFG1VGA, when sleeping at Erana's place.
   - The option to toggle sound effect types between digitized and synthesized
     has been disabled until a more user-friendly GUI option is possible.
     Digital sound effects are always preferred for now.
   - Fixed a case where starting a new song didn't fully reset its channels,
     thus some notes sounded wrong.


#### 1.4.0 "10th Anniversary" (2011-11-11)

 New Games:
   - Added support for Lands of Lore: The Throne of Chaos.
   - Added support for Blue's Birthday Adventure.
   - Added support for Ringworld: Revenge Of The Patriarch.
   - Added support for the Amiga version of Conquests of the Longbow.

 New Ports:
   - Added PlayStation 3 port.

 General:
   - Fixed the ARM assembly routine for reverse stereo audio.
   - Added support for building with MacPorts out of the box.

 AGI:
   - Implemented sound support for the DOS version of Winnie the Pooh in the
     Hundred Acre Wood.

 AGOS:
   - Implemented support for loading data directly from InstallShield
     cabinets in The Feeble Files and Simon the Sorcerer's Puzzle Pack.
   - Fixed loading and saving in the PC version of Waxworks.
   - Fixed music in the PC versions of Elvira 1/2 and Waxworks.

 Groovie:
   - Added support for the iOS version of The 7th Guest.

 Lure:
   - Fixed crash when trying to talk and ask something at the same time.

 SCI:
   - Added better handling of digital vs. synthesized sound effects. If the
     "Mixed AdLib / MIDI mode" checkbox is checked, the engine will prefer
     digital sound effects, otherwise their synthesized counterparts will be
     preferred instead, if both versions of the same effect exist.

 SCUMM:
   - Implemented PC Speaker support for SCUMM v5 games.
   - Fixed priority bug in iMuse. As a result the AdLib music should sound
     better, since important notes are not interrupted anymore.
   - Implemented CMS support for Loom, The Secret of Monkey Island and
     Indiana Jones and the Last Crusade.
   - Improved palette handling for the Amiga version of Indiana Jones and the
     Fate of Atlantis.

 Broken Sword 1:
   - Fix freeze in Windows demo.
   - Fix crash when using cutscene subtitles pack with the Macintosh version.

 Tinsel:
   - Fixed deleting saved games from the list of saved games (from the launcher
     and the in-game ScummVM menu).
   - The US version of Discworld II now shows the correct title screen and
     language flag.

 Android port:
   - Fixed plugins on Android 3.x.
   - Moved the default saved game location to the SD card.


#### 1.3.1 "All Your Pitches" (2011-07-12)

 General:
   - Improved audio device detection and fallback.
     There should be no more silent errors due to invalid audio devices.
     Instead ScummVM should pick up a suitable alternative device.

 Mohawk:
   - Added detection entries for more variants of some Living Books
     games.

 Tinsel:
   - Fixed a regression that made Discworld uncompletable.

 SAGA:
   - Fixed a regression in Inherit the Earth's dragon walk code which
     was causing crashes there.
   - Fixed a regression causing various crashes in I Have No Mouth and
     I Must Scream.

 SCI:
   - Added detection entries for some Macintosh game versions.
   - Audio settings are now stored correctly for the CD version of EcoQuest 1.

 SCUMM:
   - Fixed graphics bug in FM-TOWNS versions of games on ARM devices
     (Android, iPhone, etc.).


#### 1.3.0 "Runner" (2011-05-28)

 New Games:
   - Added support for Backyard Baseball.
   - Added support for Backyard Baseball 2001.
   - Added support for Urban Runner.
   - Added support for Playtoons: Bambou le Sauveur de la Jungle.
   - Added support for Toonstruck.
   - Added support for Living Books v1 and v2 games.
   - Added support for Hugo's House of Horrors, Hugo 2: Whodunit?
     and Hugo 3: Jungle of Doom.
   - Added support for Amiga SCI games (except Conquests of the Longbow).
   - Added support for Macintosh SCI1 games.

 New Ports:
   - Added WebOS port.

 General:
   - Added support for loadable modules on platforms without a dynamic
     loader (GSoC Task).
   - Added Danish translation.
   - Added Norwegian Bokmaal translation.
   - Added Norwegian Nynorsk translation.
   - Added Swedish translation.
   - Added Debug Console to Cine, Draci, Gob, MADE, Sword1, Touche and
     Tucker Engines.
   - Closed significant memory leaks. RTL should now be more usable.

 AGOS:
   - Closed memory leaks in Simon 2 and Feeble Files.

 Cine:
   - Corrected memory leaks and invalid memory accesses.
     Future Wars should be more stable.
   - Made Operation Stealth completable, though significant graphical
     glitches remain so not official supported.

 Drascula:
   - Added German and French subtitles in the Von Braun cutscene (#5372:
     no subtitles in scene with "von Braun").
   - Improved French translation of the game.
   - Added support for "Return To Launcher".

 Gob:
   - Fixed "Goblin Stuck On Reload" bugs affecting Gobliiins.

 Kyra:
   - Closed memory leaks.

 Parallaction:
   - Corrected issue which could cause crash at engine exit.
   - Closed memory leaks in Nippon Safes Amiga.

 SCI:
   - Added a CMS music driver for SCI1 - SCI1.1 games.
   - Added an option to toggle undithering from the ScummVM GUI.
   - Added several previously missing parts of the game state in saved games,
     such as game played time, script created windows, the script string heap
     and information related to the text parser in old EGA games.
   - Added support for SCI1.1 magnifier cursors.
   - Added support for the keypad +/- keys.
   - Added support for the alternative General MIDI tracks in the Windows CD
     versions of Eco Quest, Jones in the Fast Lane, King's Quest 5 and Space
     Quest 4.
   - Added support for the alternative Windows cursors in the Windows version
     of King's Quest 6.
   - Added support for simultaneous speech and subtitles in the CD versions of
     Space Quest 4 and Freddy Pharkas.
   - Corrected resource loading leaks.
   - Corrected several problems and issues in the Skate-O-Rama rooms in Space
     Quest 4.
   - Corrected several issues in Hoyle Classic Card Games.
   - Fixed several graphical glitches (like, for example, parts of the screen
     that weren't erased correctly under some rare circumstances).
   - Fixed several script bugs.
   - Fixed several pathfinding related issues and lockups (like, for example,
     a lockup in the shower scene of Laura bow 1 and pathfinding in some
     screens during the chase sequence in Laura Bow 2).
   - Fixed several music related glitches and possible lockups (like, for
     example, a rare music lockup that occurred when loading a saved game
     outside the palace in Quest for Glory 3).
   - Fixed possible problems and lockups in the character import screens of
     Quest for Glory 2 and 3.
   - Fixed a bug that caused a lockup in the SCI1 CD version of Mixed Up Mother
     Goose, after Tommy Tucker's song.
   - Fixed a script bug in the CD version of King's Quest 5, which caused a
     lockup under certain circumstances when going outside the witch's house
     in the dark forest.
   - Function keys now work correctly when the num lock key is on.
   - Improved support for fanmade game scripts.
   - Improved support for non-English versions of games.
   - Made several enhancements and fixes related to MT-32 music (e.g. reverb).
   - Music is no longer out of tune when loading saved games.

 SCUMM:
   - Improved support for FM-TOWNS versions of games.

 Sky:
   - Fixed crashes on sequences for several ports (Android, OpenGL, ...).

 Teenagent:
   - Closed memory leaks.

 Tinsel:
   - Closed memory leaks in Coroutines.
   - Added enhanced music support for the German CD "Neon Edition" re-release
     of Discworld 1.

 Touche:
   - Corrected memory leaks and minor issues.

 Tucker:
   - Added workarounds for several issues present in the original game.

 SDL ports:
   - Closed memory leaks in Mouse Surfaces.

 Android port:
   - Switched to the official NDK toolchain for building.
   - Fixed GFX output for various devices.
   - Fixed various crashes.
   - Switched to the native screen resolution to improve text readability.
   - Added support for pause/resume.
   - Added support for games using 16bit graphics.
   - Increased the performance significantly.
   - Added support for the "Fullscreen mode" option. Unchecking this keeps the
     game's aspect ratio.
   - Added a new graphics mode for linear filtering.
   - Overhauled the input system (see README.Android).
   - Added a MIDI driver based on SONiVOX's Embedded Audio Synthesis (EAS).

 Nintendo DS port:
   - Added support for loadable modules.

 PSP port:
   - Added support for loadable modules.
   - Added image viewer.

 PS2 port:
   - Added support for loadable modules.

 Wii/GameCube port:
   - Added support for loadable modules.
   - Fixed 16bit mouse cursors on HE games.


#### 1.2.1 "Bork Bork Bork" (2010-12-19)

 General:
   - Added Hungarian translation.
   - Added Brazilian Portuguese translation.

 Cruise:
   - Fixed a problem with Raoul appearing when examining the Book.

 Groovie:
   - Fixed a regression that made the Russian version of T7G crash.

 Lure:
   - Fixed several NPC movement bugs.


#### 1.2.0 "FaSCInating release" (2010-10-15)

 New Games:
   - Added support for Fascination.

 New Games (Sierra SCI0 - SCI1.1):
   - Added support for Castle of Dr. Brain (EGA and VGA).
   - Added support for Codename: ICEMAN.
   - Added support for Conquests of Camelot.
   - Added support for Conquests of the Longbow (EGA and VGA).
   - Added support for EcoQuest: The Search for Cetus.
   - Added support for EcoQuest 2: Lost Secret of the Rainforest.
   - Added support for Freddy Pharkas: Frontier Pharmacist.
   - Added support for Hoyle's Book of Games 1.
   - Added support for Hoyle's Book of Games 2.
   - Added support for Hoyle's Book of Games 3 (EGA and VGA).
   - Added support for Hoyle Classic Card Games.
   - Added support for Jones in the Fast Lane.
   - Added support for King's Quest I (SCI remake).
   - Added support for King's Quest IV (SCI version).
   - Added support for King's Quest V (EGA and VGA).
   - Added support for King's Quest VI (low and hi res).
   - Added support for Laura Bow: The Colonel's Bequest.
   - Added support for Laura Bow 2: The Dagger of Amon Ra.
   - Added support for Leisure Suit Larry 1 (SCI remake) (EGA and VGA).
   - Added support for Leisure Suit Larry 2.
   - Added support for Leisure Suit Larry 3.
   - Added support for Leisure Suit Larry 5 (EGA and VGA).
   - Added support for Leisure Suit Larry 6 (low res).
   - Added support for Mixed-up Fairy Tales.
   - Added support for Mixed-up Mother Goose.
   - Added support for Pepper's Adventures in Time.
   - Added support for Police Quest I (SCI remake).
   - Added support for Police Quest II.
   - Added support for Police Quest III (EGA and VGA).
   - Added support for Quest for Glory I/Hero's Quest.
   - Added support for Quest for Glory I VGA remake.
   - Added support for Quest for Glory II.
   - Added support for Quest for Glory III.
   - Added support for Slater & Charlie go camping.
   - Added support for Space Quest I (SCI remake) (EGA and VGA).
   - Added support for Space Quest III.
   - Added support for Space Quest IV (EGA and VGA).
   - Added support for Space Quest V.
   - Added support for The Island of Dr. Brain.

 New Ports:
   - Added Android port.
   - Added Dingux port.
   - Added Caanoo port (based on the GP2XWiz port).
   - Added OpenPandora port.

 General:
   - Removed the outdated PalmOS port.
   - Switched to the "fast" DOSBox OPL emulator.
   - Fixed a crash in the rjp1 player code affecting the FOTAQ Amiga version.
   - Added support for more original media layouts.
   - Added support for GUI localization.
   - Improved GUI by adding tooltips and radiobuttons.
   - Improved GUI usability by hiding more irrelevant options not supported by
     specific games.

 AGI:
   - Fixed number of GFX glitches.
   - Made PIC drawing code picture perfect.
   - Added support of MIDI devices.
   - Added support for accurate Tandy sound emulation. Switched to it as default.

 Broken Sword 2:
   - Fixed missing speech in some cutscenes.
   - Fixed a memory leak that would eventually cause the game to hang.
     (#4828 - BS2: Game lockup in British Museum)

 Drascula:
   - Fixed number of GFX glitches.
   - Made many cutscenes smoother.
   - Changed behavior of items menu. Now it shows up on mouse up.

 Groovie:
   - Added support for the Macintosh version of The 7th Guest.
   - Added support for custom MT-32 instruments.

 KYRA:
   - Fixed some minor graphical glitches.
   - Implemented formerly missing recreation of some in game items.
   - Added support for playing Kyrandia 3 with the original CD file layout.

 LURE:
   - Fixed bug where Goewin could get stuck in the Weregate.
   - Fixed issue with Ratpouch repeatedly moving between two rooms.
   - Fix for Goewin losing her schedule after Were-cave.
   - Fix for player getting stuck in sewer exit room.

 Parallaction:
   - Made part one of The Big Red Adventure completable.

 SAGA:
   - Fixed graphics glitches in several scenes.

 SCUMM:
   - Several improvements in Maniac Mansion NES.

 PSP port:
   - New backend design: fixed minor graphical issues and enabled 16-bit support.
   - Enabled playback of MP3 files using the Media Engine. This means that
     the port is optimized for MP3 files (as opposed to OGG).
   - Many optimizations. Everything should run faster.

 Wii port:
   - Added support for USB2 mass storage devices (requires The Homebrew Channel
     >= v1.0.8 with IOS58).

 GameCube port:
   - Added support for DVDs with the ISO9660 file system.

 GP2X port:
   - Added support for dynamic engine plugins (experimental).
   - Reworked control system and better touchscreen support.

 GP2XWiz/Caanoo port:
   - Improved downscale code to minimise 'tearing' corruption.
   - Reworked control system and better touchscreen support.
   - Renamed backend from GP2XWIZ to GPH to better reflect
     the supported devices.


#### 1.1.1 "Better version" (2010-05-02)

 New Ports:
   - Added Nintendo 64 port. (Actually added in 1.1.0, but forgot to mention it. oops)

 General:
   - Fixed several minor bugs here and there.

 Drascula:
   - Fixed regression that caused some texts to always be in English, even when
     using another language. (#4819 - DRASCULA: missing german translation)

 KYRA:
   - Fixed a bug which caused the DOS versions to crash before the credits when
     AdLib music is selected.

 LURE:
   - Fixed several memory leaks.
   - Corrected problems in the handling of followers when blocked from performing
     actions by closed doors between rooms.
   - Solved issues with Goewin not always correctly following the player out of the caves.

 Tinsel:
   - Fix video playback regression in Discworld 2.

 Parallaction:
   - Fix several crashes and other regressions in Nippon Safes, including
     bugs 2969211, 2969232, 2969234, 2969257, 2970141.

 Wii/GameCube port: (Also forgot to mention in 1.1.0)
   - Added support for games using 16bit graphics.
   - Complete GFX overhaul (new video modes, better performance,
     picture smoothing, fine grained overscan settings).
   - Added a new options dialog for Wii/Gamecube specific settings.
   - Fixed a GFX glitch on savegame thumbnails.
   - Added support for SMB mountpoints (Samba/Windows shares).
   - Added an on-screen console, which is shown when ScummVM exits abnormally.
   - Fixed a couple of crashes when using Ogg/Vorbis encoded sound files.
   See the bundled READMII.txt file for more information.

 PSP port: (Also forgot to mention in 1.1.0)
   - Added plugin support which allows the PSP Phat to run every game.
   - Added a new virtual keyboard optimized for rapid D-Pad input.


#### 1.1.0 "Beta quadrant" (2010-04-04)

 New Games:
   - Added support for Blue's Art Time Activities.
   - Added support for Blue's Reading Time Activities.
   - Added support for Freddi Fish 5: The Case of the Creature of Coral Cove.
   - Added support for Pajama Sam: Games to Play on Any Day.
   - Added support for SPY Fox 3: Operation Ozone.
   - Added support for Dragon History.
   - Added support for TeenAgent.

 General:
   - Added support for a custom SJIS font for FM-TOWNS and PC98 games.
   - Added support for 16bit graphics. (GSoC Task)
   - Removed QuickTime MIDI backend on Mac OS X; it was buggy and did not
     compile on modern systems.

 CinE:
   - Added support for Amiga style menus for Amiga versions of Future Wars.

 KYRA:
   - Added support for the Amiga version of The Legend of Kyrandia.
     (sound support was done as a GSoC Task)
   - Adapted KYRA to support the custom SJIS font.

 SCUMM:
   - Added support for the PC-Engine version of Loom.
   - Added support for music and sound effects in the the Amiga version of
     The Secret of Monkey Island. (GSoC Task)
   - Fixed some other bugs related to game versions for the Amiga.
   - Added support for original save/load dialog in MM NES.
   - Added support for savepoint passcodes for Sega CD MI1 via debugger command 'passcode'.
   - Added support for Kanji rendering in Japanese version of Monkey Island Sega CD.


#### 1.0.0 "Shiny Logo" (2009-11-15)

 New Ports:
   - Added MotoEZX and MotoMAGX ports.

 General:
   - Fixed several bugs in GUI.
   - Updated the project logo and icons.
   - Clarified licenses for several PS2 port files.

 AGI:
   - Fixed crash on game exit.
   - Fixed crash at detection of some games.

 AGOS:
   - Fixed load/save code for PC version of Waxworks.
   - Fixed undo in Puzzle Pack games.

 Broken Sword 1:
   - Fixed missing background sound effects in some rooms.

 CinE:
   - Fixed crashes with Future Wars and Operation Stealth demos.

 Cruise:
   - Fix freeze on game pause.

 Gob:
   - Fixed gfx glitch in Lost in Time.
   - Fixed hotspot-related regressions in Gob2.
   - Fixed several regressions in Gob3.
   - Fixed crash in Bargon Attack intro.
   - Fixed animations in Win3.1 version of Gob3.

 Groovie:
   - Fixed video performance on PSP.
   - Fixed menu blanking in some instances.

 Kyra:
   - Fix possible corruption of restart game save in Kyrandia 1.
   - Fix GFX glitch at the broken bridge.
   - Fix for brandon turning invisible in some situations in Kyrandia 1.

 MADE:
   - Fixed performance on NDS.
   - Fixed crash in Return to Zork demo.

 Parallaction:
   - Fixed several regressions in Nippon Safes.
   - Fixed music in sushi bar in Nippon Safes.

 SCUMM:
   - Fixed crash on Macintosh versions of Putt-Putt Joins the Parade and Fatty
     Bear's Birthday Surprise.
   - Fixed game save in Macintosh versions of HE games.
   - Fixed default save game path in later HE games.
   - Fixed palette in NES version of Maniac Mansion.
   - Fixed palette in Amiga version of Secret of Monkey Island.
   - Fixed cutscenes not stopping music after pressing ESC in DIG.
   - Fixed lip-sync in Fatty Bear.
   - Fixed crash in FT when entering inventory.

 Tinsel:
   - Fixed crash in palace.
   - Fixed crash when talking to the old lady.
   - Fixed partially off-screen text in DW1.

 NDS port:
   - Added mouse pad undeneath onscreen keyboard.
   - Added ability to scroll when cursor reaches edge of screen (in mouse pad
     mode).
   - Made cd audio read from track01.wav as well as track1.wav.
   - Fixed bug with switching modes with keyboard active.

 PSP port:
   - Fixed video flickering and stretching in some situations.
   - Improved suspend/resume support.

 WinCE port:
   - Improved compatibility with VGA devices.


#### 1.0.0rc1 "Grog XD" (2009-08-31)

 New Games:
   - Added support for Discworld.
   - Added support for Discworld 2 - Missing Presumed ...!?.
   - Added support for Return to Zork.
   - Added support for Leather Goddesses of Phobos 2.
   - Added support for The Manhole.
   - Added support for Rodney's Funscreen.
   - Added support for Cruise for a Corpse.

 General:
   - Added experimental AdLib emulator from DOSBox.
   - Added quick search to Launcher.
   - Improved modern GUI theme look.
   - Added per-game GUI options.
   - Improved Mass Add dialog.

 New Ports:
   - Added GP2X Wiz port.

 AGI:
   - Increased compatibility for Sierra games.
   - Implemented all 'unknown' commands.

 Beneath a Steel Sky:
   - Changed the game speed to match the original game (previously
     it ran too fast).

 Broken Sword 1:
   - Added support for the original cutscenes.
   - Dropped support for the now obsolete MPEG2 cutscenes.
   - Added support for the PlayStation version.

 Broken Sword 2:
   - Added support for the original cutscenes.
   - Dropped support for the now obsolete MPEG2 cutscenes.
   - Dropped support for playing cutscene sound without the video.
   - Added support for the PlayStation version.

 Gob:
   - Introduced a new savegame format to fix a fatal flaw of the old one,
     breaking compatibility with old savegames made on big-endian systems.

 Groovie:
   - Increased microscope puzzle difficulty to match original.

 KYRA:
   - Added support for PC Speaker based music and sound effects.
   - Added support for 16 color dithering in Kyrandia PC-9801.

 PSP port:
   - Added support for sleep mode (suspend/resume).

 WinCE port:
   - Speed optimized versions of low-res Smartphone and 2x scalers.
   - New aspect correction scaler for VGA (or higher) devices.
   - Dropped support for MPEG-2 and FLAC.


#### 0.13.1 "SAGA returns" (2009-04-27)

 AGOS:
   - Fixed crash after OmniTV video is played in The Feeble Files.
   - Fixed crashes when exploring Jack the Ripper scene in the PC version of
     the Waxworks.
   - Fixed palette glitches in the AtariST version of Elvira 2.
   - Fixed noise that can occur when sound effects are played, when exploring
     Pyramid scene in the Waxworks.

 Gob:
   - Fixed a crash in the Italian version of Woodruff.

 Groovie:
   - Fixed some issues with music in The 7th Guest.

 Parallaction:
   - Fixed the sarcophagus puzzle in Nippon Safes.

 SAGA:
   - Fixed a crash in Inherit the Earth.
   - Fixed glitches in the save/load dialog.

 Sword2:
   - Fixed random sound corruption when using the original sound files.

 Game launcher:
   - Fixed a case where memory could be corrupted.
   - Fixed the small cursor in the modern theme.
   - Fixed a bug in the theme engine, which could cause crashes.
   - Made the file browser bigger in 1x mode.

 iPhone port:
   - Fixed backspace handling on the iPhone soft keyboard.

 DS port:
   - Added support for the Global Main Menu feature.

 PS2 port:
   - Switched to the new GUI and theme code.
   - All possible devices are supported to store, play and save games
     (CD, HD, USB, MC and remote).
   - Optimized cache/read-ahead for every media.
   - Added support for the Return to Launcher feature.

 Symbian port:
   - Added Bluetooth mouse support.
   - Added support for the Return to Launcher feature.

 WinCE port:
   - Fixed an issue which could cause random crashes with VGA devices.


#### 0.13.0 "More Guests" (2009-02-28)

 General:
   - Added MIDI driver for Atari ST / FreeMint.
   - Added a 'Load' button to the Launcher (not supported by all engines).
     (GSoC Task)
   - Added a new global main menu (GMM) dialog usable from all engines.
     (GSoC Task)
   - Added the ability to return to the launcher from running games (via
     the GMM). (GSoC Task)
   - Rewrote GUI renderer to use an vector based approach. (GSoC Task)
   - Rewrote GUI configuration to use XML. (GSoC Task)

 New Games:
   - Added support for Blue's 123 Time Activities.
   - Added support for Blue's ABC Time Activities.
   - Added support for Bud Tucker in Double Trouble.
   - Added support for The 7th Guest.

 AGOS:
   - Added support for the original cutscenes of The Feeble Files.
   - Added support for text compression in the AtariST version of Elvira 1.
   - Fixed combining items in Waxworks.
   - Fixed display of spell descriptions in Elvira 2.

 KYRA:
   - Added support for Auto-save feature.
   - Added support for MIDI music.

 Parallaction:
   - Credits of the Nippon Safes Amiga demo are now shown correctly.

 SCUMM:
   - Implemented radio-chatter effect in The DIG.


#### 0.12.0 "&nbsp;" (2008-08-31)

 New Games:
   - Added support for The Legend of Kyrandia: Book Two: Hand of Fate.
   - Added support for The Legend of Kyrandia: Book Three: Malcolm's Revenge.
   - Added support for Lost in Time.
   - Added support for The Bizarre Adventures of Woodruff and the Schnibble.
   - Added support for the PC version of Waxworks.
   - Added support for the Macintosh version of I Have no Mouth, and I
     must Scream.
   - Added support for Drascula: The Vampire Strikes Back.

 General:
   - Added CAMD MIDI driver for AmigaOS4.
   - Revived the PS2 port (was already in 0.11.1 but was forgotten in the
     release notes).
   - Plugged numerous memory leaks in all engines (part of GSoC'08 task).
   - Added audio double buffering to the SDL backend, which fixes the
     problems with the MT-32 emulator on Mac OS X (for now only enabled
     on Mac OS X).

 AGOS:
   - Fixed crashes during certain music in Amiga versions of Elvira 1 and
     Simon the Sorcerer 1.
   - Fixed palette issues in Amiga versions of Simon the Sorcerer 1.

 Queen:
   - Speech is played at the correct sample rate. (It used to be pitched a bit
     too low.)

 SCUMM:
   - Rewrote parts of Digital iMUSE, fixing some bugs.
   - Rewrote the internal timer code, fixing some speed issues in e.g. COMI.
   - Improved support for sound effects in Amiga version of Zak McKracken.
   - Added support for mixed AdLib/MIDI mode in Monkey Island 1 (Floppy).


#### 0.11.1 "Fixed exist()nce" (2008-02-29)

 SCUMM:
   - Improvements for Digital iMUSE subsystem. This fixes several glitches in
     The Curse of Monkey Island.
   - Fixes for cursors in HE games.

 AGI:
   - Fix for zombies in King's Quest 4.
   - Fix for changing palettes in fanmade games using AGIPAL.

 Lure:
   - Fixed some conversation crashes in the German version.
   - Fixed operation of the optional copy protection dialog in the German
     version.
   - Added saving of conversation flags as to whether a particular conversation
     option had been previously selected or not.
   - Fixed glitch that could cause transformation sparkle to happen a second
     time.
   - Fixed behavior of Goewin when you rejoin her after meeting the dragon.

 SAGA:
   - Fix for rat maze bug in Inherit the Earth which made game not completable.
   - Fixes for Inherit the Earth and I Have no Mouth game startup on a number
     of platforms.
   - Reduced the number of simultaneous open files in I Have no Mouth, to allow
     it to run on platforms that can keep a limited amount of files open (e.g.
     on the PSP).
   - Fixed graphics glitch in Inherit the Earth with simultaneous speech.
   - Fixed palette glitch in Inherit the Earth when looking at the map while at
     the docks.


#### 0.11.0 "Your Palindrome" (2008-01-15)

 New Games:
   - Added support for Elvira: Mistress of the Dark.
   - Added support for Elvira 2: The Jaws of Cerberus.
   - Added support for I Have no Mouth, and I Must Scream (demo and full game).
   - Added support for preAGI game Mickey's Space Adventure.
   - Added support for preAGI game Troll's Tale.
   - Added support for preAGI game Winnie the Pooh in the Hundred Acre Wood.
   - Added support for Amiga version of Waxworks.
   - Added support for Lure of the Temptress.

 New Ports:
   - Added iPhone port.
   - Added Maemo port for Nokia Internet tablets.

 General:
   - Added ARM assembly routines for code in the sound mixer and SCUMM
     video playback, resulting in some nice speedups on several ports.
   - Improved the way keyboard input is handled internally, resolving
     some odd quirks in some game / port combinations.
   - Added optional 'confirm exit' dialog to the SDL backend.
   - Added support for TiMidity++ MIDI server.
   - Added DMedia MIDI driver for IRIX.
   - Improved detection of new game variants and localized versions.
   - Completely reworked internal access to files. (GSoC Task)
   - Added option to delete games from the list with Del key.
   - Added support for "~/" prefix being substituted by $HOME in paths
     on POSIX systems (Linux, Mac OS X etc.).

 AGI:
   - Added support for AGI256 and AGI256-2 hacks. (GSoC Task)
   - Added support for Amiga menus and palettes. (GSoC Task)
   - Better support for early Sierra AGI titles.

 AGOS:
   - Fixed crashes related to OmniTV playback in The Feeble Files.
   - Improved work on 64-bit systems.

 Broken Sword 1:
   - Added support for FLAC encoded music.

 Kyrandia:
   - Added support for Macintosh version.

 Parallaction:
   - Added support for Amiga version of Nippon Safes, Inc.
   - Many bugfixes.

 Queen:
   - Added support for AdLib music.
   - Added missing music patterns playback in Amiga version.

 SCUMM:
   - Added subtitle skipping (via '.' key) in older games which didn't have
     this feature so far (e.g. Zak, MM, Indy3, Loom).
   - Added support for Chinese COMI.
   - Better support for eastern versions of games.
   - Various fixes for COMI and other games.
   - Added support for original save menus (browse only). Use Alt-F5 to
     activate.
   - Added support for Spanish version of NES Maniac Mansion.
   - Better support for German version of C64 Maniac Mansion.
   - Fixed bug with cursors in Windows versions of Humongous Entertainment
     games.

 SAGA:
   - Added support for compressed sound effects, voices and music.

 Touche:
   - Added workarounds for some glitches/issues present in the original game.

 WinCE Port:
   - Switched compilers again. Now using cegcc (http://cegcc.sourceforge.net/).
   - Plugins now supported for WinCE (but not used in this release).
   - Redesigned 'Free Look' action, mainly for the lure engine's requirements.
   - Smaller optimization setting to counteract the growth of the executable.
   - Various bug fixes.

 GP2X Port:
   - Support F200 Touchscreen.
   - Various fixes to input code.


#### 0.X.0 "Tic-tac-toe edition" (2007-06-20)

 New Games:
   - Added Cinematique evo 1 engine. Currently only Future Wars is supported.
   - Added Touché: The Adventures of the Fifth Musketeer engine.
   - Added support for Gobliins 2.
   - Added support for Simon the Sorcerer's Puzzle Pack.
   - Added support for Ween: The Prophecy.
   - Added support for Bargon Attack.
   - Added Sierra AGI engine.
   - Added support for Goblins 3.
   - Added Parallaction engine. Currently only Nippon Safes Inc. is supported.

 General:
   - Added dialog which allows the user to select the GUI theme on runtime.
   - Added 'Mass Add' feature to the Launcher, which allows you to scan for
     all games in all subdirectories of a given directory (to use it, press
     shift then click on "Add Game").
   - Improved the way the auto detector generates target names, it now takes
     a game's variant, language and platform into account.
   - Improved compression for DXA movies.
   - Keyboard repeat is now handled in a centralized way, rather than on a
     case-by-case basis. (Simply put, all engines now have keyboard repeat.)

 Broken Sword 1:
   - Added support for DXA cutscenes.
   - Added support for Macintosh version.

 Broken Sword 2:
   - Added support for DXA cutscenes.
   - Added "fast mode" (use Ctrl-f to toggle).

 Queen:
   - Added support for Amiga versions.
   - Fixed some sound glitches.

 SCUMM:
   - Added support for non-interactive demos of HE games (CUP).
   - Improved A/V syncing in SMUSH videos (affects Dig, FT, COMI).
   - Improved speed of the NES sound code.
   - Fixed many (sometimes serious) actor walking issues, specifically
     in Zak McKracken and Maniac Mansion, by rewriting the walking code
     for these games.
   - Fixed several other issues.
   - Added support for DXA movies playback in HE games.

 Simon:
   - Renamed Simon engine to AGOS.

 Kyrandia:
   - Added support for FM-TOWNS version (both English and Japanese).

 BASS:
   - Fixed long-standing font bug. We were using the control panel font for
     LINC space and terminals, and the LINC font in the control panel. This
     caused many character glitches (some of which we used to work around) in
     LINC space and terminals, particularly in non-English languages.

 Nintendo DS Port:
   - New engines supported: AGI, CINE, and SAGA.
   - Option to show the mouse cursor.
   - Word completion on keyboard for AGI games.
   - Plenty of optimisations.

 Symbian Port:
   - Added support for MP3 to S60v3 and UIQ3 versions.
   - Switched to SDL 1.2.11 for bug fixes and improvements.
   - Improved performance for S60v3 and UIQ3 using ARM target.
   - Limited support for pre Symbian OS9 devices due to compiler issues.
   - Updated key mapping handling.

 WinCE Port:
   - Switched to using a GCC toolchain for building.
   - Major update to the SDL lib. Better, faster, more compatible. :-)
     The included fixes are too numerous to mention here.
     Most of the updates in this version have concentrated on infrastructure.
     This leads to faster execution, greatly increased compatibility and
     OS friendliness - especially for keyboard/mouse input and display
     handling (f.ex. no more popups during gameplay).

 Windows Port:
   - The default location of the config file changed, to support multi-user
     systems.

 PalmOS Port:
   - Now using PalmOS Porting SDK which enables use of the C standard library.


#### 0.9.1 "PalmOS revived" (2006-10-29)

 New Ports:
   - Added Nintendo DS port.
   - Added GP2X port.
   - Added GP32 port.

 General:
   - Fixed potential garbage and/or crash in debug console.
   - Removed restriction on 27 games per game id when added via launcher.

 SCUMM:
   - Improved support for international versions of HE games.
   - Fixed rare actor drawing glitches.
   - Fixed path finding during smart star challendge in Big Thinkers 1st Grade.
   - Fixed graphical glitches in stomach location of Pajama Sam 3.
   - Fixed graphical glitches in HE80 version of Putt-Putt Travels Through
     Time.
   - Fixed FM-TOWNS version of Indy3 failing on Amiga port.
   - Fixed crash in MM NES when clicking on top screen area.
   - Now it is possible to turn on or off subtitles during SMUSH movies.

 Simon:
   - Improved support for international versions of the Feeble Files.
   - Fixed undefined behaviour when loading music.
   - Fixed crash when displaying some subtitles in the Feeble Files.
   - Fixed crackling sound in Mac version of Feeble Files.

 BASS:
   - Fixed character spacing in LINC terminals in floppy version v0.0303.
   - Fixed a regression which caused incorrect AdLib music emulation.

 Broken Sword 1:
   - Fixed speech-related crashes.

 Broken Sword 2:
   - More robust handling of the optional startup.inf file.

 Kyrandia:
   - Scrolling in the Kyrandia intro is less CPU intensive, at the cost of
     not being as smooth as before.
   - Fixed a tiny graphics glitch in the Kyrandia intro.
   - Improved screen update handling, speeds up drawing on small devices.
   - Improved resource loading, faster startup.

 PSP Port:
   - Fixed crashes during scrolling scenes in certain SCUMM games.
   - Added saving of thumbnail in SCUMM savegames.

 PS2 Port:
   - Overlay uses higher resolution now.
   - Can boot from USB, HDD etc. as well.

 WinCE Port:
   - Several bugfixes.
   - Re-add support for 2002 based devices.


#### 0.9.0 "The OmniBrain loves you" (2006-06-25)

 New Games:
   - Added kyra engine (for the Kyrandia series). Currently only the first
     part of the series is supported.
   - Added support for The Feeble Files.

 General:
   - Switched from CVS to Subversion.
   - Restructured our source tree partially.
   - Fixed a bug that prevented you from overriding the scaler from the command
     line for 640x480 games.
   - Added subtitle configuration controls to the Launcher options dialogs.
   - GUI was completely redesigned and is now themeable.

 SCUMM:
   - Rewrote the detection code, for improved accuracy and better support of
     fan translations. It should now work much better on games with unknown
     MD5.
   - Added subtitle configuration controls to the options dialog.
   - Fixed graphical glitches in several HE games.
   - Fixed palette glitches in Big Thinkers 1st Grade.
   - Fixed songs in the kitchen of Pajama Sam 1.

 SAGA:
   - Fixed sound distortion in the Inherit the Earth demo.

 Simon:
   - Improved Hebrew support.
   - Lots of long-overdue cleanups and re-structuring were made to accommodate
     for The Feeble Files.
   - Fixed a rare MIDI bug that would cause a channel to change volume without
     adjusting it to the master volume.
   - Fixed delay after introduction of Simon the Sorcerer 1 demo (with speech).
   - Fixed music tempo in DOS versions of Simon the Sorcerer 1.

 Broken Sword 1:
   - Added support for the --save-slot option.

 Broken Sword 2:
   - Major rewrite of how game resources are accessed in memory. This
     should fix alignment issues reported on some platforms.
   - Missing data files are handled more gracefully.

 WinCE Port:
   - Added: PocketPC: Vertical oversampling scaler 320x200=>320x240 when panel
     not shown. (n0p)
   - Added: PocketPC: Right click by double-tapping. (n0p)
   - Fixed: All: Clipping issues in some cases.
   - Added: PocketPC: Mouse emulation using keys.
   - Added: Smartphones: Virtual keyboard popup.
   - Fixed: Smartphones: Incorrect screen blit in SDL port.
   - Added: All: Mouse cursor visible on panel if using emulated mouse.
   - Added: All: Inverse landscape mode.
   - Fixed: PocketPC: Dialogs cut-off/not redrawn.


#### 0.8.2 "Broken Broken Sword 2" (2006-02-08)

 General:
   - Fixed OS X bundle building when using GCC 3.3.

 SCUMM:
   - Added support for rotating and scaling of sprites in HE games.

 Sword2:
   - Fixed last-minute crash when playing sound effects.

 WinCE Port:
   - Should fix weird crashes with DOTT stamp and in FOA. (#2439 #2430)
   - Fixed Monkey Island blocking keyboard panel on Smartphones. (thks Knakos)
   - Fixed QVGA Smartphone detection (mean it this time :p).
   - Fixed Smartphone double right click issue.


#### 0.8.1 "Codename: missing" (2006-01-31)

 General:
   - Fixed compiling with GCC 2.95.
   - Fixed LaTeX documentation.
   - Switched to new improved logo which matches new site design.
   - More descriptive game titles in all engines.
   - Fixed crash when trying to apply aspect-ratio correction to games that
     cannot use it.
   - Fixed potential security vulnerability with oversized PATH environment
     variables.
   - Lowered the default gain for the FluidSynth music driver and made it
     configurable.

 SCUMM:
   - Scrolling fixes in COMI, so it is less CPU-hungry.
   - Added support for Maniac Mansion NES German version.
   - Fixed mouse button states in COMI.
   - Fixed overflow when using control panel for robot in the Dig.
   - Added support for sound code, used by songs in HE games.
   - Improved shadows in later HE games.
   - Fixed subtitles glitches in HE games.
   - Improved music/sound for HE games.
   - Improved support for international versions of HE games.
   - Improved support for Macintosh versions of games.
   - Fixed several minor bugs.

 BASS:
   - Fix crash when speed/volume sliders are clicked and then dragged out
     of the scummvm window.

 Gob:
   - Fixed disappearing cursor when level password is typed in.
   - Warn user if he tries to run CD version directly from CD under Windows.

 Queen:
   - Fixed charset for Spanish version.

 SAGA:
   - Fixed digital music playback under BE systems.

 Simon:
   - Implemented more precise MD5-based game detection.
   - Added Polish support for Simon the Sorcerer 2.
   - Fixed fades during ride to goblins camp in Simon the Sorcerer 2.
   - Fixed palette delay at the end of Simon the Sorcerer 1.
   - Fixed sound looping in Windows version of Simon the Sorcerer 2.

 Sword1:
   - Fixed a bug where looping sounds would keep playing during cutscenes or
     when displaying any form of control panel dialog.
   - The save game dialog would erroneously claim an I/O error occurred if the
     savegame list had unused slots, and savegames were compressed.
   - Fixed a scrolling bug which caused the finale sequence to be displayed
     incorrectly.

 Sword2:
   - Fixes and cleanups to the end credits. The German credits work now.
   - Fixed missing speech/music in the second half of the game, reported to
     happen in some versions of the game.

 PS2 Port:
   - Completely reworked and now really goes official.

 PSP Port:
   - Fixed a bug that caused Broken Sword 1, and games that use ripped CDDA
     tracks (most notably the CD version of Monkey Island 1), to stop
     functioning properly after a while.

 WinCE Port:
   - Check backends/wince/README-WinCE for the latest news.
   - Fixed disappearing panel when opening a list widget in GUI.
   - Knakos patches (QVGA smartphones fix, easier key binding and panel
     switching).


#### 0.8.0 (2005-10-30)

 New Games:
   - Added SAGA engine (for the games "I Have No Mouth and I Must Scream"
     and "Inherit the Earth").
   - Added Gob engine (for the Goblins series). Currently, only the
     first of the Goblins games is supported.

 New Ports:
   - Added PlayStation 2 port.
   - Added PlayStation Portable (PSP) port.
   - Added AmigaOS 4 port.
   - Added EPOC/SymbianOS port.
   - Added fixes for OS/2 port.

 General:
   - Reworked cursor handling in SDL backend. Now cursors can have
     their own palette and scaling. This is used by Humongous
     Entertainment games now.
   - Added FluidSynth MIDI driver.
   - Added GUI for the "soundfont" setting. (Currently only used by the
     CoreAudio and FluidSynth MIDI drivers.)
   - The MPEG player could hang if the sound ended prematurely.
   - Improved autoscaling GUI, which takes full advantage of your screen.
   - Fixes for GCC 4.

 SCUMM:
   - Added support for Mac Humongous Entertainment titles.
   - Added support for multiple filenames/versions using a single target.
   - Implemented CGA and Hercules render modes in early LEC titles.
   - Added dialogs which are shown when you modify the talkspeed or
     music volume using hotkeys.
   - Added support for NES version of Maniac Mansion.
   - Added thumbnail support for savegames.
   - Broke compatibility with HE savegame (HE v71 and upwards only).
   - Added possibility to disable building of HE and SCUMM v7 & v8 games
     support.
   - Fixed the last few known music glitches in Sam & Max. (There are
     still some - probably - minor missing features though.)
   - Added support for Commodore64 version of Zak McKracken.
   - Eliminated all demos targets and platform-specific targets. Config
     file is autoupdated.

 Sword2:
   - Made the resource manager expire resources more intelligently.
   - Improved performance when playing the game from CD instead of hard
     disk.
   - Simplified sound effects handling. Again.
   - Code cleanups and restructuring.
   - Fixed long-standing bug in decompressing sounds from the
     speech/music CLU files. It was generating one sample too many,
     which could be heard as a very slight popping noise at the end of
     some sounds. Files that have been compressed with older versions
     of compress_sword2 will, of course, still have the same error. You
     may want to regenerate them.


#### 0.7.1 (2005-03-27)

 General:
   - Added a MT-32 emulator. (It was actually added in 0.7.0 but we
     forgot to put it into the NEWS file :-).
   - Less memory-hungry MPEG movie playback for the Broken Sword games.

 SCUMM:
   - Fixed wrong actor animation in Full Throttle INSANE.

 Windows Mobile port (PocketPC / Smartphone):
   - Fixed FOTAQ crash on all platforms when leaving the hotel AGAIN.
   - Better low quality AdLib emulation for FOTAQ.
   - Fix randomly broken Hide Toolbar option. (thanks iKi)
   - Fix first hardware key mapping (was not displayed before).
   - Fix BASS & Simon hangs on Smartphone when using the Skip hotkey.
   - Fix Zone key action on Smartphone (now mapped on key 9).
   - Experimental third party VGA mode (SE-VGA) fix.
   - Add Key Mapping option in the launcher (Options / Misc / Keys).
   - Remove AYGSHELL.DLL dependency to work on CE .Net platforms.
   - Fix key mapping issues introduced in 0.7.0.
   - Full Throttle interactive action sequences should be more playable.
   - New key mapping option "FT Cheat" to win a Full Throttle action sequence.
   - Quit Simon game with 'Action key' on Smartphones.

 Sword2:
   - Fixed crash caused by attempting to play music from CD1 and CD2 at the
     same time.
   - Fixed crash in the cutscene player if the speech file was missing.

 BASS:
   - Fixed crash when talking to the gardener using spanish text with the
     floppy version.


#### 0.7.0 (2004-12-24)

 New Games:
   - Added 26 Humongous Entertainment titles, only a few are completable.

 General:
   - Added support for FLAC (lossless) encoded audio files.
   - Added an 'On Screen Display' to the SDL backend.
   - Partially rewrote the backend API.
   - Comments and the order of section in config files are preserved now.
   - Updated AdvMame scalers based on scale2x 2.0 - AdvMame3x looks nicer now,
     and AdvMame2x is MMX accelerated.
   - Added MMX i386 assembler versions of the HQ2x and HQ3x scalers.
   - Added 'Extra Path' option allows for a searching an additional datafile
     location (for reencoded cutscenes and the like).
   - Disabled Alt-x and Ctrl-z quit keys in favor of Ctrl-q on unix like
     operating systems, like Linux (exception: Mac OS X still uses Cmd-q).
   - Separate smaller font for the console, allowing for more visible
     information, for example in the SCUMM debugger.
   - Added support for setting output sample rate at run-time, although there
     is currently no GUI for it.
   - The save directory now has a default rather than the current directory on
     some platforms:
        - Mac OS X:     $HOME/Documents/ScummVM Savegames/
        - Other unices: $HOME/.scummvm/
   - Added a new about dialog with scrolling credits.

 SCUMM:
   - Removed the old zak256 target, use zakTowns instead.
   - Added native support for Macintosh versions using a special container
     file. This removes the need for using the 'RESCUMM' program.
   - Added smooth horizontal scrolling for The Dig, Full Throttle and COMI
     (matching the original engine).
   - Partially rewrote the text engine, fixing various bugs, especially in
     newer games (The Dig, COMI).
   - Fixed actor drawing glitches in V1 Maniac and Zak.
   - Fixed ship-to-ship graphic glitches in COMI.
   - Fixed palette glitches in COMI.

 Queen:
   - Fixed some issues with the Dreamcast backend.

 Sword1:
   - Added support for compressed speech and music.
   - Added support for the demo.
   - Better support for the Czech version.
   - Added workarounds for script and subtitle bugs in some game versions.

 Sword2:
   - Simplified memory/resource management.
   - Simplified sound effects handling.
   - Added support for compressed speech and music.
   - Fixed various minor bugs.

 BASS:
   - Added workarounds for some rare scripting bugs that could render the game
     unwinnable.

#### 0.6.1b (2004-08-03)

 General:
   - Fixed copy/paste bug in launcher that may corrupt the Savegame Path.
   - Fixed crashes on 64-bit architectures.

 SCUMM:
   - Fixed VOC crash when playing DOTT Floppy.
   - Fixed palette issues in Amiga version of MI2.

 Simon:
   - Fixed VOC crash.

#### 0.6.1 (2004-07-25)

 General:
   - Fixed sound glitch when streaming two or more Ogg Vorbis sounds from the
     same file handle, e.g. in the Sam & Max intro when using monster.sog.

 SCUMM:
   - As usual: many SCUMM game engine fixes.
   - Added graphics decoders for 3DO Humongous Entertainment games.
   - Numerous Humongous Entertainment games fixes.
   - Fixed bug in Full Throttle, so battle difficulty matches original.
   - Improved Digital iMUSE.

 Sword1:
   - Warn the user if saving fails, instead of crashing.
   - Slightly more user-friendly save/restore dialog.
   - Fixed masking glitch outside Nico's apartment.

 BASS:
   - Warn the user if saving a game doesn't work.

 Simon:
   - Fixed crashes in some international versions.


#### 0.6.0 (2004-03-14)

 New Games:
   - Added Broken Sword 1 engine.
   - Added Broken Sword 2 engine.
   - Added Flight of the Amazon Queen engine.
   - Added support for V1 SCUMM games 'Maniac Mansion' and 'Zak McKracken'.
   - SCUMM game Full Throttle is now supported.

 General:
   - Subtitles now default to disabled. '-n' option now enabled subtitles.
   - Added HQ2x and HQ3x scalers.
   - Rewrote sound code for more flexibility and performance.
   - Improved native MT32 support.
   - AdLib GM emulation table revamped, providing more accurate software MIDI.
   - Default Makefile now uses configure script.
   - Greatly improved the launcher and options dialogs (work-in-progress).
   - Many other "under the hood" improvements, like a new config managment
     and plugin capabilities.

 Simon:
   - Added data files decoder for Amiga disk versions.
   - Added support for inventory graphics in Amiga versions.
   - Fixed various brief freezes.
   - Fixed minor glitches in load/save dialog in non-English versions.
   - Fixed missing inventory arrows in some versions of Simon the Sorcerer 1.

 SCUMM:
   - Many, Many, Many SCUMM game engine fixes. Many of them. And that's a lot.
   - Added INSANE support for Full Throttle 'action sequences'.
   - Added option to choose between AdLib, PCjr and PC Speaker in earlier
     games.
   - Added AdLib support for indy3ega and loom (ega).
   - Added MIDI support for loom (ega), monkeyega and monkeyvga.
   - Added sound effects support for indy3/monkeyega/monkeyvga/pass.
   - Added FM Towns targets for Loom and Indy3.
   - Rewrote in-game menu (F5) to be easier to use.
   - Improved FM Towns SFX support (YM2612 emulation, looping).
   - Classic V1 versions of Maniac Mansion and Zak McKracken are now supported
     and completable.
   - Rewrote Digital iMUSE music system.
   - Several Analog iMUSE music system bugs fixed.
   - Improved music/sound for various Amiga versions.
   - Improved compression of Fate of Atlantis and Simon the Sorcerer 2 sound
     files.
   - Keyboard fighting in Fate of Atlantis now works.
   - Keyboard support for cannon battle in Curse of Monkey Island.
   - Keyboard support for derby scene in Full Throttle.


#### 0.5.1 (2003-08-06)

- Rewrote Beneath a Steel Sky savegame code (see note in READMEs 'Known Bugs').
- Fixed dialog skipping, music volume and several crashes/freezes in Steel Sky.
- Fixed dialog skipping in V7 games.
- Fixed glitch when quitting ScummVM in fullscreen mode on Mac OS X.
- Fixed various COMI bugs related to actor placement/scaling.
- Added complete Hebrew support for Simon the Sorcerer 1 and 2.
- Several MorphOS and DreamCast port fixes.
- DreamCast now runs Simon the Sorcerer 1 & 2.
- Fixed a fullscreen problem on Mac OS X were you couldn't use the mouse in
  the top part of the screen by linking to a bugfixed version of SDL.


#### 0.5.0 (2003-08-02)

- Enhanced versions of Maniac Mansion and Zak McKracken are now supported and
  completable.
- Beneath A Steel Sky is now supported and completable.
- Added support for Amiga version of Monkey Island 1.
- Initial unplayable support for V1 version of Maniac Mansion/Zak McKracken.
- Curse of Monkey Island (COMI) support for playing from CD improved on Mac
  OS X.
- Loading COMI savegames for disk 2 doesn't anymore require disk 1 first.
- Rewritten iMUSE engine, and many Music fixes (exp. Monkey Island 2).
- Support for music in DOS versions of Humongous Entertainment games and
  Simon the Sorcerer 2 (XMIDI format).
- Support for music in floppy demo of Simon the Sorcerer 1 (Proprietary
  format).
- Complete music support for Simon the Sorcerer 2.
- Improved music and sound support in Zak256.
- Added Aspect Ratio option.
- Many other bug fixes, improvements and optimizations.


#### 0.4.1 (2003-05-25)

- Added AdvMame3x filter.
- Fixed crash in Curse of Monkey Island (and possibly other games as well).
- Fixed airport doors in Zak256.
- Fixed crash in SDL backend.
- Fixed various iMUSE bugs.


#### 0.4.0 (2003-05-11)

- Curse of Monkey Island (comi) support (experimental).
- Added support for the EGA versions of Loom, Monkey Island and Indy3.
- Improved music support in Indy3 and the floppy versions of Monkey Island.
- Many Simon the Sorcerer 1 & 2 improvements and fixes.
- Very pre-alpha Beneath a Steel Sky code. Don't expect it to do anything.
- Even more pre-alpha support for V2 SCUMM games (Maniac Mansion and Zak).
- Preliminary support for early Humongous Entertainment titles (very
  experimental).
- New debug console and several GUI/Launcher enhancements.
- New Save/Load code (easier to expand while retaining compatibility).
- DreamCast port now works with new games added for 0.3.0b.
- New official PalmOS port.
- Various minor and not so minor SCUMM game fixes.
- Large memory leak fixed for The Dig/ComI.
- SMUSH code optimised, frame dropping added for slower machines.
- Code cleanups.


#### 0.3.0b (2002-12-08)

- Massive cleanup work for iMUSE. Sam and Max music now plays correctly.
- Many bugfixes for Zak256, + sound and music support.
- Music support for Simon the Sorcerer on any platform with real MIDI.
- Experimental support for Indy3 (VGA) - Indiana Jones + Last Crusade.
- Completed support for Monkey1 VGA Floppy, The Dig.
- Added akos16 implementation for The Dig and Full Throttle costumes.
- Added Digital iMUSE implementation for The Dig and Full Throttle music.
- Loom CD speech+music syncronisation improved greatly.
- Added midi-emulation via AdLib, for platforms without sequencer support.
- Code separation of various engine parts into several libraries.
- Several fixes to prevent Simon the Sorcerer crashing and hanging.
- Hundreds of bugfixes for many other games.
- New SMUSH video engine, for Full Throttle and The Dig.
- New in-game GUI.
- Launcher dialog.


#### 0.2.0 (2002-04-14)

- Core engine rewrite.
- Enhanced ingame GUI, including options/volume settings.
- Auto-save feature.
- Added more command-line options, and configuration file.
- New ports and platforms (MorphOS, Macintosh, Dreamcast, Solaris, IRIX, etc).
- Graphics filtering added (2xSAI, Super2xSAI, SuperEagle, AdvMame2x).
- Support for MAD MP3 compressed audio.
- Support for first non-SCUMM games (Simon the Sorcerer).
- Support for V4 games (Loom CD).
- Enhanced V6 game support (Sam and Max is now completable).
- Experimental support for V7 games (Full Throttle/The Dig).
- Experimental support for V3 games (Zak256/Indy3).


#### 0.1.0 (2002-01-13)

- Loads of changes.


#### 0.0.2 (2001-10-12)

- Bug fixes.
- Save & load support.


#### 0.0.1 (2001-10-08)

- Initial version.
