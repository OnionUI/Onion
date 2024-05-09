---
slug: /apps/game-switcher
---

# GameSwitcher

*Stay on top of your games!*


import NetStdHost from './assets/game-switcher.mp4';

## Presentation

The GameSwitcher is designed to be the central user interface of Onion. It allows to scroll between the save states of the last game played in a fast and efficient way.

<p align="center">
<table><tr>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/189434217-72ef0daf-c630-4ec9-b96a-30fd76cb709f.png"/></td>
<td width="33%" align="center" valign="top"><img src="https://user-images.githubusercontent.com/44569252/189434224-aeba7b8f-c881-4784-ba2f-65c60d41d20c.png"/></td>
</tr><tr>
<td align="center" valign="top"><p><i>Game Switcher, detailed view with current game playtime / total playtime </i></p></td>
<td align="center" valign="top"><p><i>Game Switcher, fullscreen mode after pressing <kbd>Y</kbd></i></p></td>
</tr></table>
</p>




It also allows you to quickly change games and many other features:

- Quick boot your last played game
- Quickly switch games from your history
- Full overlay with accurate brightness, battery readings and playtimes
- Display playtime (current game playtime/Total games playtime)
- Minimal view mode: beautiful full screen mode
- Improved *Sleep Mode* with full game suspension (press <kbd>POWER</kbd>)
- Low battery indicator: Red frame when <15% (can be adjusted in Tweaks)
- When the battery goes below 4%, the device will automatically save and exit to prevent losing progress

## Usage

Launch the GameSwitcher by pressing the <kbd>MENU</kbd> button.
The last game played will be resumed in a few seconds, and with the simple press of a button, save your progress and turn off the console.  


### GameSwitcher usage demo
<p align="center"><video controls><source src={NetStdHost}/></video></p>

### Controls

<p align="center">

| Button | Function        |
| ------ | --------------- |
| <kbd>Left/Right</kbd>             | Navigate between last played games                                           |
| <kbd>Up/Down</kbd>                | Set brightness                                                               |
| <kbd>A</kbd> or <kbd>Menu</kbd>   | Resume selected game                                                         |
| <kbd>B</kbd>                      | Quit                                                                         |
| <kbd>X</kbd>                      | Remove from GameSwitcher <br /><sub>(it will keep the save state)</sub>     |
| <kbd>Y</kbd>                      | Toggle fullscreen                                                            |
| <kbd>Select</kbd>                 | Toggle playtime display                                                      |

</p>

## Advanced

:::note Tips:
In game you can double click on menu button to quickly switch to the previous game played.
:::


:::info standalone emulators are not compatible
GameSwitcher builds on RetroArch's Save State functionality so it's not compatible with standalone emulators.
:::
