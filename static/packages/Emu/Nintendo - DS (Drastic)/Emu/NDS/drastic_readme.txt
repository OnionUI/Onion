DraStic Nintendo DS emulator

by Exophase (exophase@gmail.com)

~ desperate times call for drastic measures ~

-- Changelog --
Legend:
  @: optimization
  +: new feature
  #: bug fix
  -: removed feature
  *: Note

r2.4.0.0p:
  +: Added high-resolution 3D mode. Enhances DS 3D graphics so they're 
     rendered at 512x384 resolution instead of 256x192. Has a big performance
     overhead and therefore works best with high end quad-core devices.
  #: Fixed a bug causing garbled graphics when capture is set to use invalid
     VRAM banks. Fixes graphical problems in Doki Doki Majo games.
  #: Fixed multiple bugs with 2D sprites. Fixes glitches in Puyo Pop Fever,
     Wario Ware DIY, Final Fantasy Tactics A2, and probably others.
  #: Fixed an incorrect implementation of VRAM bank mirroring. Fixes missing
     graphics in Daniel X.
  #: Added a workaround that turns off deferred rendering to fix graphics
     glitches in Florist Shop.
  #: Added timing fixes that address problems in Zhu Zhu Babies and 
     Will o' Wisp DS.
  #: Fixed a glitch in the custom BIOS that caused audio pitch and volume to
     be incorrect in some games that used them. Fixes Chrono Trigger and
     probably others.
  #: Fixed implementation of strange viewport values to work like it does on
     a DS. Fixes missing graphics in Americal Girl: Kit's Mystery Challenge.
  #: Fixed switching the 2D engine screens mid-frame. Fixes map scroll in
     Zelda Phantom Hourglass.
  #: Fixed a bug with fog causing green dots to appear in some games like
     Animal Crossing and Zelda Spirit Tracks.
  #: Fixed a savestate issue with audio. Prevents the sound from becoming
     extremely bad on savestate loads in Golden Sun Dark Dawn.
  #: Fixed bug causing graphics problems in some games on x86 devices. Fixes
     Solatarobo.
  #: Fixed implementation of toon highlight shading to modulate against R
     instead of RGB. Fixes color of tile highlighting in Advance Wars DS.
  #: Added more robust check for mapped memory, fixes crashes on some devices.
  @: Optimized screen darkening in 16-bit mode and brightening in all modes.
  @: Changed in-game save generation code to only update the parts of the file
     that were modified. Makes an especially large difference for games with
     very big save files, eg Wario Ware DIY (32MB NAND)

r2.3.0.2p:
  #: Fixed slowdown in Pokemon Mystery Dungeon: Explorers of Sky.

r2.3.0.1p:
  #: Fixed regression with graphical glitches caused during fog rendering.

r2.3.0.0p:
  +: Emulator crashes will now save an input log if possible, to facilitate
     reproducing the crash.
  +: Savestate is now saved to a temporary file first, so if a crash happens
     while the state is being saved it won't overwrite the old one.
  +: Savestates made using BIOS files not currently in use will now switch to
     those files if they're in the system directory.
  +: Homebrew games now pass a NitroFS path. Needed to run Minecraft DS.
  +: Added DeSmuME footer to .dsv save files, allowing them to load in
     DeSmuME 0.9.10.
  +: Implemented custom cheat files. See the cheats section for more
     information.
  +: Added slot 2 (GBA cart) loading support, which can be used to access bonus
     content in some games. See the slot 2 section for more information.
  @: Improved performance of indirect branches.
  @: Special cased IRQ and SWI block lookups, slight performance improvement
     possible.
  @: Optimized perspective division in geometry code.
  @: Minor optimizations to 2D + 3D compositing code.
  @: Optimized 3D polygons where W is constant.
  @: Optimized 3D polygons where no pixels fail occlusion tests.
  @: Optimized 3D to remove zero-length rows early.
  @: Saving savestates now happens in the background so it doesn't necessarily
     stall the emulator completely.
  @: Icons loaded in the menu are now cached to files, for people with slow 
     SD cards.
  @#: Fixed some issues with division and sqrt operations, which also made them
     faster.
  #: Fixed a bug where sleep mode was not properly exited from (fixes Tangled)
  #: Fixed an error in block load operations containing the base register in the
     register list.
  #: Rewrote 2D color effects handlers to better handle some edge cases.
  #: Implemented 3D fog effect.
  #: Implemented 3D edge marking effect (can be disabled).
  #: Fixed Z/W depth selection latch delay. Fixes graphical glitches in Marvel
     Super Hero Squad.
  #: Added several games to the game database, should fix save errors for many
     more recent releases.
  #: Increased translation recursive buffer size. Fixes Minecraft DS and
     prevents some random crashes in Pokemon White and maybe others.
  #: Corrected chip ID designation, allowing some previously broken games to
     work.
  #: Fixed read value of slot 2 region when cart is not inserted. Fixes hangs in
     some Digimon World games.
  #: Fixed a bug causing graphical problems when the same texture is used with
     different repeat modes. Fixes glitches in SaGa 3.
  #: Fixed a VRAM mapping problem. Fixes GTA.
  #: Implemented huge BGs in 2D, fixes Spyro.
  #: Fixed invalid 2D layers showing garbage.
  #: Added support for mosaic in 2D background layers.
  #: Added a workaround for hang in Spider-Man Shattered Dimensions.
  #: Added a workaround for hang in Puppy Palace.
  #: Added a workaround for hang in Ore ga Omae o Mamoru.
  #: Added a workaround for hang in Legend of Kay.
  #: Fixed a bug with mode switching load/store block memory instructions.
  #: Implemented CPU deep sleep.
  #: Added geometry stalls, fixing graphical problems in American Girl games.
  #: Fixed depth equality comparison. Fixes graphical glitches in Zelda Phantom
     Hourglass.
  #: Fixed geometry Z precision. Fixes some menu bugs in Final Fantasy 4.
  #: Fixed a texture caching bug. Fixes crashes in Final Fantasy 4.
  #: Fixed a bug causing PSG sounds to go missing. Fixes missing sound effects
     in Fire Emblem.
  #: Fixed audio voice looping. Fixes audio problems in Rune Factory.
  #: Fixed a problem with I/O loads on x86 devices. Fixes broken sound.
  #: Changed VRAM map initialization to be null. Fixes corrupt graphics when
     loading a state with Mario Kart.
  #: Changed direct VRAM and RAM display modes to no longer use deferred
     rendering. Fixes graphical corruption in Holly Hobby.
  #: Changed CPU block flushes to flush things which weren't properly being
     flushed, may avoid crashes.
  #: Fixed horizontal offsets for 3D layers. Fixes menu in Fossil Fighters.
  #: ROMs are now auto-trimmed if they can't be loaded, which fixes some problems
     on some Android systems.


r2.2.1.0p:
  +: Added support for 7z and RAR archives for ROM loading. Please thank Lordus
     for implementing this feature.
  +: Added a bare-bones DLDI emulation (experimental, use at your own risk)
     See the section in the readme for how to use it.
  +: Added database info for many newer games.
  +: Added emulation of NAND backup (huge thanks to Normmatt, and his work in
     DeSmuME and much direct help on the matter!) Allows play of
     Wario Ware: DIY and Jam with the Band
  +: Added option to fix main 2D engine to top screen.
  +: Added game icon display in file browser.
  #: Fix initialization of VRAM maps, which could cause some things to break
     when a game is reset or reloaded.
  #: Changed 3D capture to effectively latch the value written if it happens
     before a current capture ends, instead of automatically clearing it.
     Fixes broken screen transition in Knytt Stories.
  #: Fixed audio sample buffers to be usable from more than just main RAM.
     Fixes broken sound in jEnesisDS.
  #: Fixed bug in cheat menu that prevented cheats from updating after being
     set (until you reload the emulator)
  #: Made the framebuffer update if clear depth or clear color changes. Fixes
     regression in Dark Spire which prevents map from being visible.
  #: Changed open bus reads to return 0x0 instead of 0xFFFFFFFF on the entire
     address space, after doing some DS tests. Fixes hangs in Digimon Story
     games, which rely on strange cart 2 bus reads.
  #: Clamps vertex Y to screen size in 3D renderer to avoid crashes in the
     new sorting code. Fixes crash regression in Final Fantasy 4.
  #: Added logic which converts some conditional instructions to unconditional
     when they come after a conditional branch. This helps prevent the
     recompiler from running away into dead code and exploding in recursion.
     Fixes crashes in Inazuma Eleven 2.
  #: Fixed a bug with conditional str instructions. Fixes crash in Penguins of
     Madagascar.
  #: Fixed a bug with conditional swi instructions. Fixes English patch of
     Super Robot Wars Exceed.
  #: Fixed a bug that caused some swp instructions to break, fixes a problem
     with Infinite Space.
  #: Changed DMA timing for Yu-Gi-Oh 5D's World Championship 2010, preventing
     a freeze in battle.
  #: Changed timing for Sonic Chronicles. Fixes crashes.

r2.2.0.2p:
  #: Fixed caching of null textures again. Fixes regression in
     Zelda: Phantom Hourglass.

r2.2.0.1p:
  #: Fixed caching of null textures with valid palettes. Fixes regression in
     Hotel Dusk.

r2.2.0.0p:
  @: Detects constantly changed portions of ITCM code and patches branch targets
     to separate blocks as they're modified. Improves performance in games that
     heavily modify ITCM like Golden Sun.
  @: Several improvements to reduce the cost of compilation flushes, minimizes
     intermittent pauses in games during code modifications (eg Pokemon)
  @: Switched from tracking just a small number of modified literal values to
     all positions in ITCM and main RAM. Prevents massive slowdown in C.O.P.
  @: Now compiled with GCC 4.8.x, has some performance benefits sometimes
  @: Geometry backface test is now done in an optimized NEON function
  @: More 3D rendering functions are now done in optimized NEON
  @: Division/sqrt routines are more properly retaining their values now, less
     redundant updates.
  @: Optimized polygon sorting.
  @: Changed cheat processing to minimize work done for cheats that aren't
     enabled.
  #: Fixed a bug where loads were generated when misalignment wasn't properly
     detected as possible.
  #: Games with backup files > 512KB are no longer stored in savestates
     regardless of the option to do so. Fixes save states with Art Academy and
     Zelda Spirit Tracks.
  #: Fixed a bug with block loads over unallocated registers.
  #: Game database lookup will now match on game code only if name doesn't match
     (useful for some ROM hacks that changed the name) and will match on low
     24-bits of gamecode if the whole thing doesn't match (useful for region
     variations not in the list)
  #: Games that modify VRAM mid-frame via HDMA are now rendered differently to
     accommodate. Fixes screen glitches in Golden Sun.
  #: Texture caching changes to fix some games.
  #: Fixed a bug involving blending between 3D layers that happened when it
     shouldn't have. Fixes bad background color in Dragon Quest IX.
  #: Now caches clear color/depth for next frame. Fixes missing graphics in
     Tales of Innocence.
  #: Fixed a glitch in alpha ID test in NEON code. Fixes corruption during
     screen transition in Zelda games.
  #: Updated savestates to include saving the power management registers. Fixes
     problems with microphone no longer working after resuming from a savestate.
  #: Fixed DMA from ITCM region to read zero. Fixes Knights in the Nightmare
     graphical problems.
  #: Fixed NEON code for some functions handling pixel spans of zero width. 
     Removes some small graphical artifacts with tiny polygons.
  #: Added a clamp for overflows in triangle coordinates. Fixes some crashes.
  #: Register allocation fixes, fixes problems in Front Mission and de Blob 2.

r2.1.6.2p:
  #: Fixed bug causing the RTC to be read incorrectly for the month of October.
     Causes problems and even crashes in some games during October.

r2.1.6.1p:
  #: Fixed issue that could cause frameskip to not always work properly in 2.1.6a
  #: Added event checking after stm instructions, fixes graphics bugs in Donkey
     Kong Jungle Climber
  #: Fixed savestate buffer being the wrong size, could cause crashes with the
     option to include backup in savestates.

r2.1.6p:
  +: Added option to sync speed to values other than 100%. Doesn't change how
     the "fast forward" option works, which is still "fast as ever." Works best
     with auto-frameskip.
  +: Added option to save backup in savestates (defaults on), can prevent save
     corruption in games like Pokemon.
  @: Optimized graphics capture emulation for direct full capture, improves
     performance a bit in games with dual-screen 3D (and some other stuff)
  #: Fixed register allocation bug, fixes crash in Panzer Tactics DS.
  #: Fixed 128KB EEPROM, fixes Pokemon Myster Dungeon Explorers of Sky
  #: Added detection for a bad branch offset in recompiling blocks that don't
     exist yet. Fixes crash when C-Gear is enabled in Pokemon Black/White 2.
  #: Fixed a bug where the palette wasn't being updated correctly because of
     bad memory map management. Fixes blacked out graphics in Pokemon games.
  #: Rolled back to not using DMA or gxfifo timings except in Bowser's Inside
     Story. Fixes a bunch of regressions this caused (and improves performance)
     Going to be more careful rolling this out in the future.
  #: Added a game-specific workaround for Art Academy so it'd read vcount=192
     before the IRQ fires and gets past its load screen. Looking into less hacky
     alternatives.
  #: Fixed DraStic BIOS for GetCRC16 function where a 0 length was passed in.
     Prevents Cooking Mama games from hanging.
  #: Added a bunch of games to game_database.xml
  #: Fixed palette DMA from gamecard mid-frame. Fixes missing graphics in Lock's
     Quest.
  #: Rewrote page unmapper to not be broken. Fixes corrupt graphics in Pokemon
     HeartGold/SoulSilver change Pokemon in battle screen, glitches in
     Phantasy Star 0, and some other games.
  #: Fixed power management register reporting battery level as always low.
  #: Added check in auto-frameskip to not skip on a new capture frame if it's
     the first one in a while. May reduce some graphical glitches from
     frameskip (or not).
  #: Changed window settings to not reset the tracking mid-frame when the
     vertical positions are written to. Fixes missing graphics in some games.
  #: Fix for code which terminated blocks early on 0x0 instructions, to actually
     make them call an indirect branch to resume afterwards. Fixes "mark" cheats
     crashing in Pokemon.
  #: A fix was made to flag settings in the interprocess communication
     emulation. Fixes the touchscreen and music going (in other words, ARM7)
     going dead in Big Bang Mini.

r2.1.5.1p:
  #: Fixed a regression in the last version where DMA timing could cause an
     infinite loop if a new DMA was issued before the last one finished (fixes
     hang in Grand Theft Auto: Chinatown Wars)
  #: Fixed a problem in the implementation of 3D edge expansion which prevents
     it from ruining graphics in a few games.

r2.1.5p:
  #: Added edge expansion in 3D emulation which reduces black line gaps in
     Pokemon games.
  #: Added gamecard timing and DMA timing which reduces problems in
     Pokemon HeartGold, SoulSilver, and Platinum
  #: Added gxfifo timing and some timing hacks which (in addition to the above)
     reduces problems in Mario & Luigi: Bowser's Inside Story
  #: More fixes were made to the cheat engine (thanks Normmatt)
  #: A fix was made to the geometry engine when redundant glBegin commands were
     entered. Fixes crashes in Heroes of Mana, Glory of Heracles, Assassin's
     Creed, Inazuma Eleven, probably otherwise.
  #: Game database has been extensively overhauled by examining the games for
     a special save function AND comparing the results with NoIntro's database.
     Most games should have a correct save type now.

r2.1.4p:
  +: Added option to unzip ROMs to storage instead of memory. Useful if you
     lack RAM. See the options section for more info.
  @: Changed recompiler to terminate blocks containing several 0x0 instructions
     early. Avoids serious slowdowns in Suikoden Tierkreis and
     Zelda: Phantom Hourglass.
  #: Fix for Pokemon Mystery Dungeon Explorers of Sky saving
  #: Added gamecard address wraparound, which fixes anti-piracy triggering in
     games like Pokemon Conquest

r2.1.3p:
  #: A bunch of fixes were made to the cheat database, many more of them should
     actually work and not cause crashes.
  #: Implemented 1-element texture matrix stack. Fixes graphical problems in
     Fire Emblem: Dark Shadow.
  #: Made ARM7 properly reschedule when waking up if IRQs are not enabled.
     Fixes hang at end of a stage in Diddy Kong Racing.
  #: Fixed an ldm emulation bug that triggered with some register allocations.
     Fixes hang in Spore Creatures. 
  #: Changed alpha ID test to not apply to opaque pixels in alpha polygons.
     Fixes missing graphics in Justice League.
  #: Added y-sort for polygons. Fixes a bunch of missing graphics bugs in menus
     and the like, but will probably come at a bit of a performance hit.
  #: Fixed missing power management register emulation. Fixes microphone bugs
     in the Zelda games.
  #: Fixed recompiler flags caching bug that preventing Megaman Battle
     Network 5DS from loading.
  #: No longer requires BIOS/firmware (but will still use ones you've installed).
     Please see the System Files section for more information.
  #: Pokemon Heart Gold/Soul Silver is at least playable due to a timing hack.
  #: Fixed 2D rendering bug where 8bpp affine sprites would show up as opaque
     if the MSB in the palette entry was set. Fixes blue borders around sprites
     in Bob's Game demo.
  #: Implemented signal handler to allow loads from the first 4KB of memory.
     Fixes crashes in homebrew titles like Detective DS and Bob's Game demo.

r2.1.0p:
  +: Added option to compress savestates
  +: Added option to store snapshots in savestates, visible in the menu
  +: Added timestamp display of savestates in the menu
  +: Added option to mirror touchscreen input on the other screen.
  +: Closing the Pandora lid now emulates a DS hinge close.
  +: Added firmware editor option. DraStic can also now run w/o a firmware
     file, but this is experimental - if you have problems please revert to
     using the firmware file and report it.
  +: Added cheat code support. See cheats section for more information. Huge
     thanks to Normmatt for his help with this.
  @: Separated ITCM translation cache from main translation cache. Makes
     games which constantly modify ITCM run better.
  #: Fixed initialization problem with palette and OAM memory mappings. Fixes
     missing text in Pokemon games when not the first game loaded.
  #: Fixed auto-initialization of slot 2 memory for homebrew games that use it.
  #: Implemented sleep mode.
  #: Corrected unaligned ldrh instructions to use ARM9 behavior instead
     of ARM7 behavior. Fixes Italian language selection in Brain Training.
  #: Fixed halt instruction not being woken up if CPU I flag is set.
     Fixes freeze in Pokemon White/Black 2, probably others.
  #: Implemented backup command 0x8, which prevents Pokemon Heart Gold/
     Soul Silver from eventually crashing on screen transitions. Special
     thanks to Normmatt for giving me the heads up on this.
  #: Improved timing slightly by taking extra cycles for unconditional
     block memory instructions. At least seems to be enough to fix
     Pokemon Heart Gold/Soul Silver.
  #: Fixed some real-time clock parameter count bugs. Fixes time in
     Pokemon games.
  #: Fixed direction matrix read registers. Fixes Nanostray 2 missing
     graphics.
  #: Fixed an overflow problem on geometry clipping. Fixes a crash in
     Okami Den.
  #: Added detection of self-modifying code for block store instructions.
     Let's Mario & Luigi: Bowser's Inside Story at least gets in game.
  #: Fixed an issue with texture caching. Fixes graphical glitches in
     Last Window.
  #: Fixed bug in highlight shading. Fixes colors in Cars Mater-National
     Championship.
  #: Fixed bugs in video capture, fixes Splinter Cell night vision
  #: Fixed a bug where toon shading wasn't handled for untextured polygons.
     Fixes Mario Kart invincibility.
  #: Fixed a bug in light vector caching causing some lighting errors,
     fixes GTA:CW and probably others.
  #: Fixed a regression (and possibly earlier problem) causing a bunch
     of Activision games to not start at all.
  #: Fixed capture bug. Fixes bad bottom screen in Need for Speed Carbon.
  #: Fixed bug with clear images. Fixes graphics in Narnia: TLtWatW
  #: Fixed bug with AUXSPICNT status bit. Allows Dementium II to load.

r2.0.1p:
  #: Fix regarding .drastic_file_info.txt files that caused crashes on new game
     loads.

r2.0p:
  @: 3D engine emulation completely redone, with major optimizations in place.
     About 3-5x faster than the old one, with extra optimizations for sprite-
     like graphics.
  @: Geometry engine emulation completely redone. About 2-3x faster than the
     old one, although more optimization is still pending.
  @: Optimization for CPU I/O stores, particularly for geometry commands
  @: Optimization for CPU MSR instruction emulation
  @: Optimization for CPU Thumb BL instruction emulation
  @: Optimized some audio emulation functions
  @: Optimized paletted texture caching
  +: Auto frameskip now operates every other frame when capture is enabled,
     to better frameskip dual-screen 3D graphics
  #: Improved fake wifi emulation, enough to get Pokemand Diamond/Pearl and
     Castlevania: Order of Ecclesia working. Special thanks to Slaeshjag for
     his help on this!
  #: Fixed emulation of LDRD CPU instruction
  #: Fixed bug in square root and division engine emulation. Fixes missing
     graphics in Eragon.
  #: Fixed bug in flags emulation. Fixes graphics problems in the LEGO Harry
     Potter games.
  #: Worked around a possible kernel bug that seemed to cause cache flushes to
     not always work. May prevent some crashes.
  #: Fixed a bug causing crashes in some 2D affine backgrounds with extreme
     zooms. Fixes Chrono Trigger crash near beginning.

r1.4.0p:
  +: Save game files now automatically update one second after the emulated
     game last modifies its save game storage.
  @: Made audio capture only take place after first time the emulated game
     tries to use the microphone, saves substantial battery power in the normal
     case. Should also mitigate some system freezes.
  #: File loader will ignore drastic_file_info.txt files if they're older than
     the game database file.
  #: File loader no longer leaves empty drastic_file_info.txt files in
     directories traversed that have no games.
  #: Corrected broken behavior of VEC_TEST geometry command.
  #: Made speed display show up on active screen if single screen mode is
     selected.
  #: Made capture unit capture 3D even if 3D layer is disabled. Fixes inability
     to select objects in Hotel Dusk.
  #: Fixed box test implementation, restores graphics in Ragnarok DS.
  #: Fixed decal blending (for real this time, hopefully), improves graphics in
     Picross 3D.
  #: Fixed a bug with conditional swap instructions, fixes crash in
     Tony Hawk's Proving Grounds.

r1.3.3p:
  #: Changed fix again, to catch other stuff that might have went wrong

r1.3.2p:
  #: Fixed fix in regression fix (broke Tetris DS and probably others)

r1.3.1p:
  #: Fixed regression leading to crashes when uninitialized textures are used
     (like Hotel Dusk)

r1.3p:
  *: Changed release numbers in this file to be more consistent with PND
     releases
  +: Added option to delete game specific configurations
  +: Added option to output one screen scaled on the Pandora and another
     screen to TV out. Select main display (/dev/fb0) in Pandora TV out
     configuration and "TV split" scaling option in config.
  +: Added ability to switch between vertical and horizontal orientation with
     Z key and single and double screen display with C key.
  +: Added 1x:2x and 2x:1x scale modes.
  +: Added option to display games in file list by database title or internal
     ROM name + game code. Press Y to switch display modes. This may take a
     while the first time it's done in a directory; (a few seconds to a minute 
     or more depending on the quality of the SD card and the number of files 
     in the directory, so be patient) after that a cache file is created to
     make it faster for future uses.
  +: Game database is now coded by game code + header title, which is checked
     before CRC32. Allows modified (trimmed, hacked, etc) games to match and
     hopefully not have save problems, and allows games to load faster
     (unzipped games load instantly)
  +: Made 1 and 2 keys save and load state respectively.
  @: Reduced RAM requirements by about 37MB.
  #: Implemented geometry BOX_TEST, POS_TEST, VEC_TEST, which fixes Ace Attorney
     series inability to investigate items.
  #: Fixed cases where tiled affine BG in wrap mode crashed if too zoomed out.
     Fixes crash in Brain Training.
  #: Fixed a bug that causes conditional loads from non-standard memory regions
     to fail. Fixes crash in Phantom Hourglass.
  #: Added ability for VRAM to map code in ARM9. Fixes crash in Pokemon Black.
  #: Tweaked auto-frameskip/sync more.
  #: Fixed crashes that could happen due to textures accessing unallocated
     banks. Fixes sporadic crashes in Eragon.
  #: Fixed decal blending. Fixes some glitchy graphics.
  #: Fixed texture cache lookup and allocation bugs. Fixes some glitchy
     graphics.
  #: Fixed crashes caused by games reading past edge of backup memory. Picross
     3D boots without crashing.
  #: Now initializes POWCNT1 to 0x1. Picross DS boots.
  #: Now ignores pld instructions instead of improperly handling them as
     unconditional loads. Fixes Zoo Keeper.
  #: Fixed a bug with flags state and internal forward branches. Fixes
     Lego Star Wars II not loading save games correctly.
  #: Changed something in audio emulation again, prevents Rayman DS from
     crashing.
  #: Fixed liveness information for swap instruction. Fixes crash at start of
     Tony Hawk's Proving Grounds
  #: Fixed a memory mapping bug that could cause issues when switching between
     games

r1.2p:
  +: Implemented microphone. Not the greatest audio quality but should be
     sufficient. Be sure to enable it and set the volume high enough using
     the OS volume controls. Thanks to Slaeshjag for help.
  +: Improved frame sync. Auto frameskip should be better too, as well as
     audio latency. Thanks to notaz for help.
  #: Corrected path for BIOS image error
  #: Fixed broken sp-relative reg + reg memory accesses. Fixes crash at
     start of Anno 1701.
  #: Fixed liveness analysis/register allocation bug, fixes crash in Tetris DS
     music selection
  #: Fixed bug with uninitialized textures, prevents crash in Cartoon Network
     Racing
  #: Fixed bug with uninitialized extended palettes, prevents crash in
     Ghost Trick
  #: Added a boot indicator in memory that allows a lot of games to boot
     that previously stalled at a white screen or crashed.
  #: Fixed bad value in geometry emulation reset, fixes some bad savestates
     that crashed, and handled it better so they don't still cause crashes.
  #: Changed something in audio emulation that prevents The World Ends With
     You from crashing early on.
  #: Fixed self-modifying code clear bug. Fixes crash in 999.
  #: Fixed bug in swp instruction breaking register allocation, fixes crash
     in Yoshi's Touch n Go and probably others
  #: Fixed register allocation bug in recompiler, fixes crash in Mario Kart,
     FF4, and probably others
  #: Fixed bug caused by capture engine writing to ARM7 mapped VRAM, fixes
     crash in Mario Kart
  #: Fixed bug breaking screen on large zipped ROMs

r1.1p:
  #: Fixed a bug that caused some games to crash with the following error:
     'Code block @ pc fffffffe is to invalidly mapped memory.'
  -: Took out 'Q' key exiting the emulator by request (hit accidentally)
  
r1.0p: First release, Dragonbox Compo version, Pandora only

-- About --

DraStic is a Nintendo DS emulator written with ARM devices in mind. The goal of
this emulator project is to lower the baseline system requirements vs other
available Nintendo DS emulators, while still providing an acceptable level of
game compatibility. 

This emulator is not derived from any other existing emulators (including gpSP).


-- Status --

Most DS features are emulated, although some are currently missing:
 - Some 3D features are missing, like anti-aliasing
 - Wifi connections of any sort aren't emulated
 - Minor missing features like 2D mosaic for sprites, sound capture
 - No controls remapping support
 - Compatibility issues with some games
 - Loads compressed (.zip, .7z, .rar) ROMs but if you don't have enough free
   memory for the uncompressed ROM they'll be forced to be uncompressed (maybe),
   or crash the emulator. 128MB ROMs should work on 256MB Pandoras, and 256MB
   ROMs on 512MB ones. Note that now there's also an option to uncompress to
   storage.


-- System Files --

DraStic no longer needs BIOS or firmware files to run, except to play encrypted
ROMs. You can still install them the following in the appdata directory:

 nds_bios_arm9.bin          4KB
 nds_bios_arm7.bin          16KB
 nds_firmware.bin           256KB

You can dump these files from a DS if you own a flashcart. You can use this
homebrew program to do it:

http://library.dev-scene.com/index.php?dir=DS/Hardware (Firmware) 07/DSBF dump/

If you notice a problem with that only happens with our BIOS please report it.
Note that load states won't work if the BIOS types used while the state was
made don't match what you're currently using.


-- Game Database --

DraStic uses a game database file that was originally based on one available
from ADVANsCEne. But their database had a lot of errors in it, so it was
heavily revised with a direct scan of ROMs. This worked by using heuristics to
detect a library function where save type is specified. This was then checked
against the No-Intro database for more accuracy.

If you find problems in the database still, or missing entries, please contact
me.


-- Controls --

(Note: controls can't be customized yet)

Menu:
  D-pad:                            Navigate cursor
  B button:                         Select option
  X button:                         Cancel/exit menu
  Y button:                         Switch display mode in file browser
  A button:                         Move up directory in file browser
  L shoulder:                       Page up in file browser
  R shoulder:                       Page down in file browser
  
In-game:
  D-pad:                            Nintendo DS d-pad
  A button:                         Nintendo DS Y button
  B button:                         Nintendo DS A button
  X button:                         Nintendo DS B button
  Y button:                         Nintendo DS X button
  L shoulder:                       Nintendo DS L shoulder
  R shoulder:                       Nintendo DS R shoulder
  Start or enter key:               Nintendo DS start button
  Select:                           Nintendo DS select button

  Escape key:                       Exit emulator
  S or 1 key:                       Save savestate to slot 0
  L or 2 key:                       Load savestate from slot 0
  Z key:                            Switch horizontal/vertical orientation
  X key:                            Swap screens (useful in single-screen mode)
  C key:                            Switch single/double screen display
  F key:                            Toggle fast-forward
  M, enter, or space key:           Enter menu
  Shift + M, enter, or space key:   Enter file menu directly


-- Menu Options --

From the main menu you can select "Configure Options" to change these. These
options will get saved if you select to save them at the bottom of the screen.
You can save them either for every game or for the currently loaded game only.
If you save them for every game the settings will be the default that will be
loaded for all games that don't have a game-specific configuration file.

The options are as follows:
 - Frame skip type
   Setting to "manual" or "automatic" will allow frames to be skipped. When a
   frame is skip no 2D or 3D is rendered (although the geometry engine is still
   emulated), which allows for speedup. The amount of frameskip is configurable
   with a frameskip value of "N": if set to manual every N out of (N + 1) frames
   will be skipped. So for instance, if you set the frameskip value to 2 only
   1/3rd of the frames will be rendered. If frameskip type is set to automatic
   the emulator will attempt to skip frames only when the emulation is slowing
   down below normal speed, but it will not skip more than N frames in a row.
  
   Note that frameskip can visually break some games more than just making them
   choppier, so if you get glitches check to see if turning off frameskip helps.

 - Frameskip value
   The value to use for "manual" or "automatic" frameskip as described above.
   This value can be set from 0 to 9.

 - Screen orientation
   Set if you want the screens to be laid out left to right (horizontal), top
   to bottom (vertical), or just one screen (single)

 - Screen scaling
   You can select from the following options:

   None:
    No scaling. Will display each DS screen in 256x192 pixels, centered on the
    Pandora's screen.

   Stretch aspect:
    Will stretch the emulated displays to fill the height of the screen in
    vertical orientation, or the width in horizontal orientation, while
    maintaining the correct aspect ratio.

   1x:2x:
    Will display the two screens as 256x192 on the left and 512x384 on the
    right. If vertical orientation is selected the 256x192 screen will
    be positioned towards the bottom, while if horizontal orientation is
    selected it will be positioned in the center. This mode does not work
    with single screen mode; instead stretched aspect will be used.
    
   2x:1x:
    Will display the two screens as 512x384 on the left and 256x192 on the
    right. If vertical orientation is selected the 256x192 screen will
    be positioned towards the top, while if horizontal orientation is
    selected it will be positioned in the center. This mode does not work
    with single screen mode; instead stretched aspect will be used.

  TV Split:
    Will display one screen in stretched aspect on the Pandora and another
    screen in fullscreen over TV out. Set TV out to

 - Screen swap
   Set to swap the top and bottom screen display positions. If screen
   orientation is set to single mode it'll swap which screen is displayed.

 - Show speed
   Set to show an indicator of how fast the emulator is running.

 - Enable sound
   Set to enable or disable sound. Note that this doesn't increase performance
   much, but might be better than listening to choppy audio.

 - Fast forward
   Set to make the emulator run as fast as possible instead of syncing to real
   Nintendo DS speed.

 - Mirror touchscreen
   When set touch presses to the DS's non-touch screen will be registered as if
   they were pressed on the touch screen.

 - Compress savestates
   Set to compress savestates with LZSS compression. May cause saving and
   loading states to take a little longer, but will make them take up a lot
   less storage space.

 - Snapshots in savestates
   Set to store a snapshot of the current game screen in the savestate. Visible
   on the menu when the load state option is highlighted.

 - Enable cheats
   Set to enable cheat code processing in games. See the Cheat Codes section for
   more information. Note that this setting is NOT saved in per-game settings,
   and will only be saved (for all games) when the "Exit: save for all games"
   option is selected.

 - Uncompress ROMs
   When this option is selected compressed (zip, rar, 7z) ROMs will be
   uncompressed to storage before the game is executed. The location they're
   uncompressed to is appdata/DraStic/unzip_cache. Be warned: this can make
   games take a very long time to load, and you'll need to have enough free
   space for this to work. If you try to load the same game multiple times in a
   row, DraStic will see a cache file which lets it know that it doesn't have to
   be unzipped again (unless you deleted the unzipped file).

 - Backup in Savestates
   Setting this option on will make it so backup data (in-game saves) are stored
   to savestates. This is useful because some games will get confused if they
   see an in-game save that comes from the "future", ie if you perform a load
   state after making a save. This can cause save games to be corrupted, which
   can make games unplayable. On the other hand, this option increases the risk
   of accidentally losing saves while making savestates, and makes savestates
   larger.

 - Speed override
   This option allows synchronizing the emulation speed to 50%, 150%, 200%,
   250%, 300%, 350%, or 400% instead of the standard 100%. This works best in
   conjunction with auto-frameskip, which will skip frames as necessary to
   try to maintain the speed specified.

 - Fix main 2D screen
   Nintendo DS has two display engines, one that displays 2D + 3D content
   and the other that only displays 2D content. Normally, the games are able to
   swap which engine is displayed on the top screen and which is displayed on
   the bottom screen. Setting this option effectively ignores the swap and
   always displays the main engine (2D + 3D) on the emulated top screen and the
   sub engine (2D only) on the emulated bottom screen. This can be useful for
   some games like the (mostly) 2D Sonics (Rush, Chronicles), because it will
   cause the emulated top screen to follow the screen the player is on during
   main gameplay sequences.

 - Disable edge marking
   This option will prevent edge marking in games that have it enabled. Edge
   marking draws outlines around some 3D models to give a cel-shaded effect.
   Since DraStic doesn't emulate anti-aliasing, it'll cause edges to look
   harsher than they may on a real DS. For this reason and because of
   possibility of glitches or user preference, this option is provided to
   override the game's edge marking.


-- Firmware Editor --

From the main menu you can select "Configure Firmware" to edit the emulated
 firmware. Using this you can emulate user settings like username, language,
 birthday, and favorite color. Note that these settings may only take effect
 after the game is reloaded.


-- Cheat Codes --

DraStic supports Action Replay Nintendo DS cheat codes using a cheat code
 library that comes with the emulator. If you want to override the library
 you can place a new library file named usrcheat.dat in the DraStic appdata
 directory. This file can be found for instance with the DS-Scene ROM Tool
 which can be downloaded from here:

 http://filetrip.net/nds-downloads/rom-hacks/
  download-retrogamefan-ds-scene-rom-tool-v10-build-1205-f25536.html

 Individual cheats in a game can be enabled or disabled by selecting the
 "Configure Cheats" option in the main menu. The cheat database file will be
 updated to remember the cheat settings.

 NOTE: A bad or unsupported cheat setting can cause the emulator to crash.
 If you cause a game to always crash after setting some cheat you should
 first set the "Enable cheats" option to "no" in the options menu. Then
 reload the game and disable the problematic cheat.

As of version 2.3.0.0, it is also possible to provide custom cheat files.
 These are placed in the appdata/DraStic/cheats directory. To create a
 custom cheat file, make a file named <ROM name>.cht in a text editor
 (for example, you can use vim on a terminal on the Pandora). <ROM name>
 should be the same as the ROM filename to apply the cheat to, not
 including the extension.

 Cheats are specified in the cheat file with the following format:

 [Cheat Name]+
 <cheat code>

 For example, a file Mario_Kart.cht may have the following contents:

 [Always Be Shyguy]+
 923cdd40 0000000c
 023cdd40 00000001
 d2000000 00000000
 94000130 fffb0000
 023cdd40 0000000c
 d2000000 00000000

 The + after the cheat name is used to specify that the cheat is activated. If
 the + is removed the cheat will be ignored.


-- Slot 2 Cartridges --

The slot 2 function allows inserting a virtual cartridge into an emulated
 slot 2 cartridge slot, which was present on original Nintendo DS and DS Lite
 units. Usually, this will be used to load Gameboy Advance games, which enables
 bonus features or save file transferring for some games.

 Slot 2 cartridge files should be placed in the appdata/DraStic/slot2 directory.
 If a file named <ROM file>.gba is found, it will be loaded when <ROM file>.nds
 (or .zip, .rar, or .7z) is loaded. If a save file named <ROM file>.sav is found
 it will be loaded in the backup memory portion of the cartridge. The save file
 format is raw without any header and should be compatible with .sav files
 produced by VBA or gpSP. SRAM, FRAM, and flash are supported as save types.

 If <ROM file>.gba is not present slot2.gba (if present) and slot2.sav will be
 loaded by default.


-- DLDI --

If a homebrew game is loaded DraStic will emulate an R4 flashcart and will
redirect emulated SD card reads and writes to a file called drastic_dldi.img.
This file should be placed in appdata/DraStic.

You can create the image file like this, on a Linux machine (probably including
the Pandora itself, please let me know if this doesn't work)

dd if=/dev/zero of=drastic_dldi.img bs=1M count=<size in MB> of=drastic_dldi.img
mkdosfs drastic_dldi.img

Then if you mount like so:

mkdir <mount directory>
sudo mount -t vfat drastic_dldi.img -o loop <mount directory>

You can throw files on it in a normal shell. Then umount with:

sudo umount <mount directory>

(be sure the file isn't mounted when you run DraStic!)

No automatic DLDI patching is performed (yet), so you must make sure that the
homebrew already works with R4. You can do this either by using homebrew that
works out of the box with R4 or by using an offline patcher. For example, you
can use dlditool which can currently be downloaded here:

http://chishm.drunkencoders.com/DLDI/

Download dlditool and r4ts.dldi, and invoke it with the following to patch a
DLDI-compliant homebrew ROM to use R4:

dlditool r4ts.dldi <homebrew.nds>
 

-- FAQ --

Q: Do I need a Pandora with 512MB of RAM to run DraStic?
A: No. However, there is one caveat (and this can just as well apply to the
   512MB versions) - if you're running a compressed ROM the uncompressed file
   needs to fit in memory. Compressed ROMs 128MB (original size, not compressed)
   and smaller should work on a 256MB Pandora. 256MB will fail, but should be
   okay on a 512MB Pandora. 512MB zipped ROMs won't work on anything.

   It _may_ be possible to alleviate this requirement if you use a swap drive,
   which also _may_ be fast enough; I haven't tested it so I can't confirm or
   deny this.

   As of version 2.1.6a it's also possible to allow the emulator to temporarily
   uncompress ROMs to storage.

Q: Do I need a 1GHz Pandora to run DraStic?
A: No, but of course it will help, as will overclocking whatever you do have.

Q: Why are so few games fullspeed?
A: As of version 2.0p a lot more games should run full speed with moderate
   frameskip on a 1GHz Pandora (especially if overclocked to 1.1-1.2GHz). Of
   course some games are still too demanding for Pandora.

Q: How much can the performance situation improve?
A: There should continue to be optimizations in the future, but probably nothing as
   dramatic as the shift brought forth by 2.0p, except maybe for some games that
   have really specific serious performance problems.

Q: Why not use OpenGL ES hardware to help with the 3D engine emulation?
A: This has a lot of compatibility problems since the DS's 3D engine is so
   weird, plus has its own implications for performance because of how it
   mixes 2D and 3D engine output (getting framebuffer data back is slow).
   Pandora's GPU isn't very strong anyway, and DS emulation would need some
   heavy fragment shaders.

Q: Will there be any better screen orientation options?
A: 1x + 2x is implemented now. I'd like to add an option to have a configurable
   gap between vertical oriented screens (since a lot of DS games are designed
   around this), but this will cut into how much the screens can be scaled.
   We'd like to add an option to rotate the screen too, but this won't be ready
   until a later version.

Q: What about an option to take up the entire screen?
A: There's code for this but I haven't enabled it because with two screens on
   it's extremely distorting, and with one screen on it doesn't make a big
   difference. I may reconsider if there's strong demand.

Q: Will you add cheat support?
A: It's added as of version 2.1.0.0p.

Q: What about DLDI support?
A: It's added as of version 2.1.1.0p, although it's bare bones and requires some
   external work to setup. See the DLDI section.

Q: What about wifi support?
A: Wifi emulation is very complex to implement. Lordus and I are currently
   experimenting with ways to implement it. Bear in mind that Nintendo is
   shutting down their servers soon, so if we ever get emulation it may only
   be useful for so-called "NiFi", that is, emulating DSes communicating
   with each other directly when in close proximity. There are no promises
   that we'll ever support anything, only that if support is added to the
   Android version it'll also be available in the Pandora version, and we'd
   try to allow interoperability between the two.

   Right now it should be "working" enough so that games don't hang or give
   errors while trying to access it.

Q: What should I do to report a bug?
A: Report it on the Pandora forums (http://boards.openpandora.org). There will
   probably be a thread for this. Don't e-mail or PM me unless you absolutely
   can't post there - if you keep this public other people may be able to help.

   The most important first step in reporting bugs is being able to reproduce
   them. This will usually involve loading a savestate and performing some steps
   until it happens, then specifying those steps in the report.

   As of version 2.3.0.0, a file called input_capture_crash.ir is created in
   appdata/DraStic if the emulator crashes. This contains an input record which
   may help is in reproducing and fixing the crash - if you have this file
   please send it to me when reporting a crash bug.

Q: Is there a compatibility list somewhere?
A: Yes, see here:
   http://pandorawiki.org/DraStic_Compatibility_List
   (it doesn't seem that well maintained anymore, though)

Q: Where's the source code?
A: DraStic is closed source for the foreseeable future. It's not violating
   any licenses. If you want the source code for some reason send me an e-mail
   and I will consider it, depending on what you want it for and how trustworthy
   you seem.

Q: Why does DraStic need BIOS files? Other DS emulators don't need them.
A: It doesn't as of version 2.1.3p, since it comes bundled with alternatives.
   This isn't high level emulation but BIOS replacements written by us. But if
   the Nintendo BIOS files are present they will still be used. Please let us
   know if you find a compatability problem that requires them, so we can fix
   our BIOS.

   The firmware is also no longer strictly necessary as of version 2.1.0p,
   but if you have problems you can still use one downloaded from a real DS.

   You can use this homebrew program to dump the BIOS and firmware files if you
   have a flash cart:

   http://library.dev-scene.com/index.php?dir=DS/Hardware (Firmware) 07/DSBF dump/

   Note that as of version 2.1.0p your settings will always be taken from the
   emulator's internal firmware settings and not from the firmware. I might
   release a program to convert DS firmware settings to DraStic's configuration
   file if there's a lot of demand.

Q: Are you going to release this for other platforms?
A: The Android version is now out as of August 7, 2013, and is something we've
   been heavily focusing on improving. This is a joint effort between me
   (Exophase) and Lordus, so it has some features that the Pandora version
   doesn't (since I didn't implement them); then again, the Pandora version may
   also have a unique feature or two of its own.

   I'd like for there to be an iOS version one day but this is a little muddled
   right now.
  
Q: Will DraStic cost money?
A: The Pandora version will remain free. Other ports vary. The Android version
   currently costs $5.99 USD and comparable prices in other countries.


-- Credits & Thanks --

Lordus    - Work on Android version, unarchive code, testing and ideas
notaz     - Invaluable coding help and testing
Slaeshjag - Testing and coding help, and some nice demo videos
Normmatt  - Lots of help with cheat codes and other stuff
Neelix    - Hardcore beta tester
Tensuke   - logo #1
felix20   - logo #2
AsHperson - Beta tester and big help with support
JayHaru   - Beta tester and big help with support
beeflot   - Beta tester and Chinese specialist
Kaikun    - Beta testing

