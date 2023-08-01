---
slug: /advanced
sidebar_position: 6
---

# Advanced

![Advanced](https://user-images.githubusercontent.com/98862735/178853223-10d53d46-3d80-4d44-95d6-3cd345a730e1.png)


## Theme Overrides

Since Onion v4 you can override some aspects of themes. This allows you to have elements which stay the same after updating and no matter which theme is applied.

**Example:** To apply your own charging animation, just put the files `chargingState0.png` ... `chargingState23.png` in this folder: `Saves/CurrentProfile/theme/skin/extra`.


## Activity Tracker database file

Your play times are stored in `/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db`.  
There is a versioning of these files stored in `/mnt/SDCARD/Saves/CurrentProfile/saves/PlayActivityBackup`.
    
In case of problems (file corruption, etc) check this folder, and find the file that have a size of 0 KB, and grab the one just before it and replace `/mnt/SDCARD/Saves/CurrentProfile/saves/playActivity.db` with it.


## Combining different systems/cores

[Read here](./Combining%20different%20systems%20or%20cores.md)
