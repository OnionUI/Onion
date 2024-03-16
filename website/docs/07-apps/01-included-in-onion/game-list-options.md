---
slug: /apps/game-list-options
---

# Game List Options (GLO)

*Make actions on your game list*

## Presentation
Game List Options (GLO menu) is a tool for performing actions for the selected game or for the entire list. <br />
Press <kbd>Y</kbd> in a game list then from this menu you can choose the RetroArch core, download the image cover of your game (WiFi/MMP required), launch a Netplay session, reset your game (to skip the last save state), filter the list of games with a keyword,...

Thanks to GLO menu flexibility, you can even add your own scripts here!

## Usage

GLO Menu is a native application of Onion, it is installed by default.

<table><tr>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/226447277-a9452ecc-92ad-407f-a629-307491a652b4.png"/></td>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/226447388-8c9c95f1-536a-4946-8b0f-f4b8ead0e97e.png"/></td>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/226447466-71d4f5c1-7675-4355-9b60-20dbd1a03eae.png"/></td>
</tr><tr>
<td align="center" valign="top"><p><i>When in a game list, press <kbd>Y</kbd> to open GLO (this action can be reassigned in Tweaks)</i></p></td>
<td align="center" valign="top"><p><i>GLO - specific options for the selected game or the entire list</i></p></td>
<td align="center" valign="top"><p><i>Use the "Game core" option to change core per game</i></p></td>
</tr></table>

### Scripts available in Onion

- **Reset game**: *load game without save state, useful to start a game from beginning*
- **Game core**: *set custom Retroarch core per game, useful to switch from mGBA to gPSP for example*
- **Filter list**: *use a keyword to filter the list*
- **Refresh roms**: *refresh the list’s game cache, useful when you have added some games*
- *Custom scripts:*
  - **Random game**: *added by Random Game app - launches a random game from the list*
  - **Set time**: *added by Clock app - quick access to setting date and time*
- *WiFi scripts: <sub>(Miyoo Mini Plus only)</sub>*
  - **Scraper**: *Download image covers for your games. <a href="scraper">More details here</a>*
  - **Netplay**: *Launch or join a multiplayer game. <a href="../multiplayer">More details here</a>*


## Advanced

GLO Menu allows you to create your own scripts.
The GLO scripts are located in `sdcard/App/romscripts`. 

The existing scripts are a great source of inspiration for your future script. Here some examples: 
- [Scraper script](https://github.com/OnionUI/Onion/blob/main/static/build/App/romscripts/emu/Scraper.sh): good example to use rom path
- [Random game script](https://github.com/OnionUI/Onion/blob/main/static/packages/App/Random%20Game/App/romscripts/Random%20game.sh): good example to get the current section
- [Netplay script](https://github.com/OnionUI/Onion/blob/main/static/build/App/romscripts/emu/Netplay.sh): good example for dynamic labels entry in GLO menu


*Some useful variables: *

- `require_networking=1`: will show this GLO entry only on the MMP which is equipped of WiFi
- `%LIST%`: is the name of the current system (for example GENESIS)
- `$1`: will contain the full path of the selected rom 
- `$2`: will contain the full path of the current emulator
