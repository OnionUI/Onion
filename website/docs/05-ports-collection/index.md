---
slug: /ports
---


# Ports collection


*![](https://user-images.githubusercontent.com/98862735/177056415-02a5f05e-7e95-4184-900a-c0e7945d9207.png)*


The Ports Collection in Onion V4.1 has been completely redone and now includes 61 game ports!  
All the necessary files (except licensed game files) are now hosted in its own repository.

[Go to Ports Collection ➜](https://github.com/OnionUI/Ports-Collection)


<table align="center"><tr>
<td align="center"><a href="https://user-images.githubusercontent.com/44569252/227540219-bf2734a3-9686-45d9-a32e-6ad20aa56d07.png"><img src="https://user-images.githubusercontent.com/44569252/227540283-5551c998-7bc9-4a89-af96-ac7a3de7db98.png" /></a></td>
</tr><tr>
<td align="center" valign="top"><p><i>Click the image to view it in full size</i></p></td>
</tr></table>


- 24 free game ports are offered as complete packs
- 37 game ports are available through the collection of game engines, and you are only required to supply the licensed data files from the original game
- Box arts are also included
- Pre-configured config files are included - which gives you the best settings and uniform key mapping for FPS games (*credit: axcelon*)

<details>
<summary><i><b>Click to see FPS key mapping</b></i><sub> (By axcelon)</sub></summary> 
<table><td>

```
Generic Layout:
===============
L1                                    R1
Strafe left                           Strafe right                           
L2                                    R2
Previous weapon                           Next weapon


            ↑                                    X
       Move forward                           Interact

   ←                  →                  Y                  A
Turn left         Turn right         Shoot          Jump/Strafe/etc.

            ↓                                    B
        Move backward                            Run


         Select                              Start
         Map/other                           Pause

                           Menu
                           Quick switcher


-------------------------------------------------------------------------------------


Quake Layout:
=============
L1                                    R1
Strafe left                           Strafe right                           
L2                                    R2
Look up                                    Look down


            ↑                                    X
        Move forward                           Freelook

    ←                  →                Y                  A
 Turn left         Turn right         Shoot           Change weapon

            ↓                                    B
       Move backward                            Jump


         Select                                    Start
         Walk toggle                               Pause

                           Menu
                       Quick switcher

-------------------------------------------------------------------------------------


Duke3D Layout:
==============
L1                                    R1
Strafe left                           Strafe right                           
ALT
Map
L2                                    R2
Last weapon used                  Quick kick

ALT
Use inventory

            ↑                                    X
         Move forward                           Interact
         ALT                                    ALT
         Inventory right                        Aim up
   ←                  →                  Y                  A
   Turn left         Turn right         Shoot                  Crouch
   ALT                  ALT             ALT                  
   Prev. weapon         Next weapon     Center view
            ↓                                    B
        Move backward                           Jump
        ALT                                     ALT
        Inventory left                          Aim down


        Select                                    Start
       Modifier (ALT)                           Quickturn

                           Menu
                           Pause

```


<img src="https://user-images.githubusercontent.com/44569252/189995592-9d9e4702-e237-40a2-a0b7-b5e4578f0d7d.png" />
</td></table>
</details>


### Installing a port

[The ports repository](https://github.com/OnionUI/Ports-Collection) contains individual `7z` files for each port, as well as one `7z` file containing all ports.

To install these you just have to extract the contents of the archive to the root of your SD card (merging the folders).

* Licensed game files are not included for the game engines, you will need to supply the necessary game files yourself. These are detailed (along with any necessary subfolder structure) in the `_required_files.txt` file, within each `Roms/PORTS/Games/[Game folder]/` .  
* For freely available (unlicensed) ports, all files required to run the game are included (these will not have a `_required_files.txt` file).  

> **Notes**  
> Ensure you have enabled Ports Collection in `Apps` › `Package Manager` › `Verified`.   
> Also see [this helpful video tutorial](https://www.youtube.com/watch?v=ifBQ-1KC570) by _Retro Breeze_  


#### Common file structure

<table><td>

- 📁`Roms/PORTS/`
  - 📁`Games/`
    - 📁`[Game folder]/`
      - 📄`_required_files.txt`
      - 🎁`[Port files]` <sub><sup>(required)</sup></sub>
      - ...
      - ➕`[Add files specified in _required_files.txt here]` <sub><sup>(required)</sup></sub>
  - 📁`Imgs/`
    - 🖼️`[Game name].png` <sub><sup>(optional)</sup></sub>
  - 📁`Manuals/`
    - 📖`[Game name].pdf` <sub><sup>(optional)</sup></sub>
  - 📁`Shortcuts/`
    - 📁`[Category]/`
      - 📄`[Game name].notfound` <sub><sup>(required)</sup></sub>

</td></table>



### Migrating ports from Onion versions prior to 4.1.0

1. Rename your existing "Roms/PORTS" folder into something like Roms/PORTS_OLD  
2. Update Onion to version 4.1.0 or later (the latest release is recommended) (see [upgrade guide](installation#upgrading-from-stock-or-onion)).  
3. Enable "Ports Collection" in `Apps` › `Package Manager` › `Verified`.  
4. Download the "the full Ports-Collection" from [the official repository](https://github.com/OnionUI/Ports-Collection) (first link in the description).  
5. Extract the archive to the root of your SD card.  
6. Manually populate each `Roms/PORTS/Games/[Game folder]/` with your old assets, as detailed in the `_required_files.txt` file for each.  
7. We have pre-configured many things, so during your copy paste of your assets, do not overwrite the existing files.  
8. Run the `~import ports` script from the ports games list : it will refresh the list of ports which are present.  

> **Notes:**  
> Also see [this helpful video tutorial](https://www.youtube.com/watch?v=ifBQ-1KC570) by _Retro Breeze_  
> Once you have verified all of your Ports are launching correctly, you may remove your old, renamed Ports folder from step 1   
  
  
***
  

### How to add your own ports (for advanced users)  

Ports are now launched with the script included in their own shortcut.
These shortcut scripts are standardized in 3 different kind : 
* Standalone port launcher (for example [Hocoslamfy](https://github.com/OnionUI/Ports-Collection/blob/main/Hocoslamfy/Roms/PORTS/Shortcuts/Reflex/Hocoslamfy.notfound))
* Retroarch port launcher (for example [Dinothawr](https://github.com/OnionUI/Ports-Collection/blob/main/Dinothawr/Roms/PORTS/Shortcuts/Puzzle%20games/Dinothawr.notfound))
* Python port launcher (for example [Double Cross](https://github.com/OnionUI/Ports-Collection/blob/main/Double%20Cross%20v.2.0%20(PyGame)/Roms/PORTS/Shortcuts/Puzzle%20games/Double%20Cross%20v.2.0%20(PyGame).notfound))

Please always use one of these 3 scripts as a template to add your own port.


**Descriptions of the scripts settings :**

 * `GameName` : the name that will displayed in the Onion Time Tracker app  
 * `GameDir` : the name of the directory which contains your game assets `Roms/PORTS/Games/[Game folder]/`  
 * `GameExecutable` : the filename of the binary that will be launched from the GameDir directory  
 * `GameDataFile` : the file in the GameDir directory which will be used to detect the presence of the game when running `~Import ports` script from the rom list. If it is not specified then `GameExecutable` will be used for port detection.  
 * `KillAudioserver` set it to 1 if your port requires to kill audioserver (it will be restored automatically when you close your port after)  
 * `PerformanceMode` set it to 1 if you use a demanding port  
 * `Arguments` : use this field to parameter additional args to the launch command line. (See [Duke Nukem](https://github.com/OnionUI/Ports-Collection/blob/main/Duke%20nukem%203D%20(eduke32)/Roms/PORTS/Shortcuts/FPS%20-%20Duke%20Collection%20(eduke32)/Duke%20nukem%203D%20(eduke32).notfound) as example)  
 * `touch /tmp/disable_menu_button` : use this flag only if you want to disable the monitoring of menu button pressing but Onion keymon, the you can remap the menu button in your port. (Useful in games which requires many buttons). Don't forget to insert `rm -f /tmp/disable_menu_button` at the end of your script.  
 * `running command line` : Do not modify this (it is standardized)   


**Specific to retroarch script :**

 * `Core` : then name of the core that will be used without `_libretro.so`, for example `ecwolf` for Wolfenstein  
 * `RomDir` : similar to `GameDir` : it is the path where your rom is located in `Roms/PORTS/Games/[Game folder]/`  
 * `RomFile` : it is the name of the rom that will be passed as a parameter to the retroarch core, will be also used to detect the presence of the game when running `~Import ports` script from the rom list. (the `Core` will be used for detection if not specified).  


> **Notes about the `~Import ports` script :**  
> The import script reads the content of each shortcut (`.port` and `.notfound`) files to find the field `GameDataFile`.  
> `GameDataFile="CP01.MAP"` for example. If the file `CP01.MAP` exists in the `Roms/PORTS/Games/[Game folder]/` directory, then the shortcut is renamed with `.port` extension and will be displayed in the roms list otherwise it will be named with `.notfound`.  

