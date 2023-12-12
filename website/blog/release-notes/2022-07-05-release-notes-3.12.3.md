---
slug: release-notes/3.12.3
authors: [totofaki]
tags: [release-notes, stable]
---

# Release notes: Onion V3.12.3

*Official release notes for Onion V3.12.3*

<Truncate />


### Eggs keymon integration

    Low battery red frame warning in game.  

<img src="https://user-images.githubusercontent.com/16885275/177217769-32533d60-d052-45f2-b108-ecd187124733.png" width="350"/>

    Light sleep mode on power tap.     
    Deep sleep mode on long power press.   
    Save and show the game switcher on a menu button tap.   
    Save and exit game on menu button long press.   

    The hibernation is properly implemented (The device will save and goes to sleep after X minutes without touching a button) 
    The mainUI binary is repatched to be able to change this feature in the main menu settings, and set to 3mn on a fresh onion install.   
    Screenshot everywhere. (Menu+power)     

<details>
<summary>More keymon informations here</summary>


**POWER button:**  
Suspend by press for one second or less.
Press and hold for 5 seconds to force close the current foreground application.
Press and hold for 10 seconds to force shutdown.

- Unlike stock, suspend actually stops the foreground application.
- Both close/shutdown will auto-save if retroarch is running and AutoSaveState setting is enabled. (also for Hibernate below)

- Shutdown when MainUI/onionLauncher is running.
- DeepSleep when retroarch is running and AutoSave setting is ON.
- Otherwise, nothing happens.

**Hibernate function:**  
Suspend after the time set in Setting > Hibernate has elapsed without any buttons being pressed.
Shutdown after 5 minutes of suspended state and no USB power connected.

The current onion disables Hibernate during installation and does not allow configuration.
- If you want to enable, you can change it by booting without SD, configure in Setting, and then boot with onion again.
- It can also be set on a minute-by-minute basis by editing /appconfigs/system.json using Commander. ("hibernate" line)

**Low battery warning:**  
If battery falls below about 17%, a warning will appear in the screen.
- Stock displays an icon in the upper right corner, but custom displays a red frame across the entire screen.

If battery falls below about 4%, AutoSave & Exit when retroarch is running and AutoSave setting is ON.

**SELECT button:**  
Adjust the brightness by pressing L2/R2 while holding down.

- Extended functions:

**During suspend - MENU button:**  
Take a screenshot. a png file will be saved in /Screenshots in SD. (Same function as scrshot app)

</details>


### Game switcher / Main UI tight integration    
<img src="https://user-images.githubusercontent.com/16885275/177045714-bc6ab713-4fbf-49b8-b34c-f8ac1ce5e034.png" width="350"/>

    Change brightness with the up and down key.    
    MainUI and the game switcher are merged and they use the same launch script.     
    The game screens are now compressed. (from 1mo to 20-100ko)    
    Bootup script rewritten.     

### New ports in the port collection (Credits : r0b0-tr0n, Schmurtz)   
<img src="https://user-images.githubusercontent.com/16885275/177214139-b874b0e6-df84-4c9c-a12c-c827ae82747e.png" width="500"/>   

    New entries :  
    Quake   
    CannonBall   
    Flashback   
    Powder   
    Rick dangerous   
    MrBoom   
    Spear of Destiny   
    Super 3d Noahs Ark   
    Dinothawr   
    Wolfenstein 3d   

[Ports Help (Credits : r0b0-tr0n)](https://github.com/OnionUI/Onion/files/9044872/Ports.Help.pdf)

### Revamped Wiki (Credits : Olywa123) 
[<img title="GBMini by Kitsuvi" width="350px" src="https://user-images.githubusercontent.com/16885275/177215817-21a20392-3cd3-4b37-a667-ee42dbc93450.jpg?raw=true" />](https://github.com/Sichroteph/Onion/wiki/2.-Onion-Emulators-&-Ports) &nbsp; 

### Gamelists are accepted in every console by default (Olywa123)

---

    (v3.12.0) 
    Initial release

    (v3.12.1) 
    Power button long press delay reduced from 1s to 0.5s for more snappiness. 
    The main menu shutdown panel is not displayed anymore.
    The short pulse rumble (Main menu tap, power button tap..) is lowered by 20%
    The default DiMo Onion theme is updated to v1.2

    (v3.12.2)
    Option to turn off button vibrations feedback. (Add a .noVibration file in the .tmp_update folder)
    Save states / auto save states times reduced : Faster exit time to the menu.
    Onion timers integration in the Keymon, boot script and play activity UI.
    (The RetroArch ones had a bug that added hundreds of hours erratically)
    The activity time is properly paused when the device is in sleep mode, even if it is not a RetroArch game.
    
    (v3.12.3) (Files updated : onionLauncher, keymon_onion, menuBar.png)     

<img src="https://user-images.githubusercontent.com/16885275/178365221-0af0b012-0e6a-45cf-b341-823a5dfa98cc.png" width="350"/>  


    Game switcher changes :      
        Total time display.    
        Game launch / No game screen freeze fix.    
    Keymon :      
        Start button waking up the device fix.    
