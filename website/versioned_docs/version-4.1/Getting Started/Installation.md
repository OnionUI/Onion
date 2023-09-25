---
sidebar_position: 1
slug: /installation
description: Official installation guide
---

![Onions5](https://user-images.githubusercontent.com/98862735/179318051-39abe99e-79eb-43da-9778-aa9b4f6d1b28.png)


## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/189428439-d70d00ac-3b1b-416f-a8fb-b47c8c6cead2.png" width="54" /></sup>Check your firmware

- **INFO:** A new firmware version was found on devices bought in late December (version `20221216****` and `20221224****`)
  - A community firmware patch is available [here](https://www.reddit.com/r/MiyooMini/comments/104qbak/community_firmware_patch_for_new_devices/), which fixes some issues present in 1216 and 1224 firmware.
- **IMPORTANT:** The April 2022 firmware from Miyoo (version `202204******` - [get it here](https://drive.google.com/drive/folders/1VtfcBCoIcpMIBY2FIyAtjV-Dg02JUvhG)) is the minimum requirement for Onion v3.10 and above.
- You can check your current version in `Settings` › `Device Info` › `Version`.
  - *Most devices produced after July 2022 will already have the latest firmware.*
  > ***Info:** The firmware is the backend part of the OS, which lives inside your device. Onion on the other hand is a frontend that will stay fresh on your SD card.*

<table align="center"><tr><td>
<img src="https://user-images.githubusercontent.com/44569252/189426364-9984efe1-08f1-4c85-94aa-d48546ca0c45.png" width="40" />
</td><td>

We can't take responsibility if your device is bricked in the process of  
upgrading the firmware. Take a look at the [*firmware upgrade guide*](https://user-images.githubusercontent.com/16885275/170205258-8add4be7-1a1e-4ae5-a8f2-cb13c6703e06.png) - beware that the guide contains an outdated link to lemiyoo.cn, we provide an alternative download link above.

</td></tr></table>


## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179302769-4169e57a-860f-4c0e-8792-007e7557ba48.png" width="54" /></sup>Download the installation files

- Download the [latest version of Onion](https://github.com/OnionUI/Onion/releases/latest) from the release page.


## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179306127-e8a2c99c-a078-46b0-9561-47abf5c16208.png" width="54" /></sup>Installing (on an empty SD card)

### Step 1: Preparing the SD card (only necessary for first use)
- Stick to using SD cards from reputable brands (e.g. SanDisk or Samsung).
  > *The SD card included with the Miyoo Mini is known to be slow, and will likely ruin your experience and/or data.*
- Format your SD card as `FAT32`
  > **Note:** This is not the same format as `exFAT`.
  > FAT32 is known as `MS-DOS (FAT)` on Mac.
  > If your SD card is larger than 32 GB, we recommended using a tool like [rufus](https://rufus.ie/) or guiformat.

### Step 2: Copy the installation files and boot up your device to begin the installation
- Extract the contents of the [`Onion-v4.x.x.zip`](https://github.com/OnionUI/Onion/releases/latest) file and copy _all_ 6 folders to the root of your SD card.
  > The 6 folders are: `.tmp_update`, `BIOS`, `miyoo`, `Media`, `RetroArch`, and `Themes`.  
  > Make sure that you've enabled viewing hidden folders on your operating system to correctly copy _all_ of the folders to your SD card (hit <kbd>Command</kbd>+<kbd>Shift</kbd>+<kbd>.</kbd> on Mac).
- The installation will begin when you insert your SD card and boot up your Miyoo Mini.
- When the installation is done, press the <kbd>A</kbd> button (or wait 10s) and the device will turn off.

### Step 3: Copy over your bios and rom files
- Copy your bios files into the `BIOS` folder (from `RetroArch/.retroarch/system` on the Miyoo SD card).  
- Copy your roms into their respective folder in `Roms` (see the *[Emulators](emulators)* page for more information).  
- Do NOT delete the `.tmp_update` folder, despite its name this must stay on your SD card.  
- Insert your SD card, boot up your Mini and press <kbd>SELECT</kbd> from the `Games` tab in the main menu to refresh roms.  
- If you are a macOS user, run `Apps` > `Tweaks` > `Tools` > `Remove OSX files`, in Onion, to purge `._` index files from your SD card (and game lists).  


## <sup><img align="left" src="https://user-images.githubusercontent.com/44569252/179321292-8198613d-380c-4022-8ce6-ea020cc9b347.png" width="54" /></sup>Upgrading (from stock or Onion)

<table align="center"><tr><td>
<img src="https://user-images.githubusercontent.com/44569252/189432421-c3dba93b-c8c3-4456-b244-c252563ae829.png" width="40" />
</td><td>

**When upgrading, your save states, in-game saves, roms, bios files, and  
configs will carry over to the updated system.**

For some systems, ex. *gpSP*, save states won't always work after upgrading,  
please **make sure to create in-game saves** before upgrading, then you can  
delete any non-working save states after upgrading.

</td></tr></table>

### Step 1: Copy the installation files
- Extract the contents of the [`Onion-v4.x.x.zip`](https://github.com/OnionUI/Onion/releases/latest) file and copy _all_ 6 folders to the root of your SD card.
  > The 6 folders are: `.tmp_update`, `BIOS`, `miyoo`, `Media`, `RetroArch`, and `Themes`.  
  > Make sure that you've enabled viewing hidden folders on your operating system to correctly copy _all_ of the folders to your SD card (hit <kbd>Command</kbd>+<kbd>Shift</kbd>+<kbd>.</kbd> on Mac).
  - Click "Yes" when prompted to overwrite (or "Merge" if you're on Mac).
- ***Note:** If you're upgrading from v3.9 or below, we recommend you delete the `.tmp_update` folder first.*

### Step 2: Boot up your device to begin the installation
- The installation will begin when you boot up your Miyoo Mini.
- If you've already got Onion installed, the installer will only update Onion, RetroArch, emulators, and apps - you keep all your settings.
- When the installation is done, press the <kbd>A</kbd> button (or wait 10s) and the device will turn off.

### Step 3: Check that your files are in the correct folders
- If they're not already in place, place your bios files in the `BIOS` folder and roms in their respective subfolder inside the `Roms` folder (see the *[Emulators](emulators)* page for more information, here you'll find the [Rom Folders](emulators#rom-folders---quick-reference) table).
- Despite its name, the `.tmp_update` folder must stay on your SD card.
- Insert your SD card, boot up your Mini and press <kbd>SELECT</kbd> from the `Games` tab in the main menu to refresh roms.  
- If you are a macOS user, run `Apps` > `Tweaks` > `Tools` > `Remove OSX files`, in Onion, to purge `._` index files from your SD card (and game lists).


<table align="center"><tr><td>
<img src="https://user-images.githubusercontent.com/44569252/189427507-27e87caf-2331-485c-a1f4-0f6b250712c8.png" width="40" />
</td><td>

If you experience any issues during the install, try copying over the installation  
files again and use another upgrade option (you can skip copying the `BIOS`,  
`Icons`, and `Themes` folders).

If you still have issues after trying the reinstall option, you may have to reformat  
your SD card (remember to backup the `Roms` and `Saves` folders).

</td></tr></table>
