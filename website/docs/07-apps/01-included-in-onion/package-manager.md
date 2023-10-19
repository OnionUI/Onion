---
slug: /apps/package-manager
description: Choose your apps and emulators to create your own Onion.
---

# Package Manager

<sup>by Onion Team</sup>

## Presentation

The "Package Manager" app is a powerful tool that allows users to easily install or uninstall emulators and apps.

<p align="center"><img src={require('./assets/packagemanager.webp').default} style={{width: 320}} /></p>


## Usage

Package Manager is a native application of Onion, it is installed by default.

- run Package Manager from Apps section.
- Packages are divided into three sections:
  - **Verified:** Contains all verified systems which are fully compatible with Onion and the Game Switcher (see [Emulators](../emulators) for more information).
  - **Apps:** Contains all the included Onion and third-party apps.
  - **Expert:** Contains experimental/expert systems. Some are just alternative, some are standalone emulators (not compatible with Game Switcher) and some are just not fully tested. (see [Emulators](../emulators) for more information). 
- Choose the systems and apps you need and press <kbd>start</kbd>.
- at the top right of the screen you'll see the number of current modifications <sub><sup>(on the screenshot above we see that 1 item will be insntalled)</sup></sub>
- Package Manager will display a summary of the modifications, press <kbd>start</kbd> again to confirm and apply the modifications.

:::note
Removing an emulator or an app will not remove the associated roms and data.
:::

:::info
If you install an item from the `Expert` section, you'll need to display the expert section in [Tweaks app](tweaks#show-recent) -> `Appearance` -> `Show expert mode`
:::

### controls


| Button | Function                                                 |
| ------ | -------------------                                      |
| <kbd>D-pad</kbd>  | Navigate & Check/uncheck                      |
| <kbd>A</kbd>      | Check/uncheck the current item                |
| <kbd>B</kbd>      | Quit                                          |
| <kbd>X</kbd>      | Check/uncheck everything in the current tab   |
| <kbd>Y</kbd>      | Reset all the current modification            |
| <kbd>L1/R1</kbd>  | Previous/Next tab                             |
| <kbd>L2/R2</kbd>  | Previous/Next item page                       |
| <kbd>Start</kbd>  | Install summary / validate the modifications  |




## Advanced

- To reinstall a package, first toggle it off and press <kbd>START</kbd> to apply, then open Package Manager, toggle it on and apply.

