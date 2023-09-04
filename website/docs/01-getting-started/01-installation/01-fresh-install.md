---
slug: /installation/fresh
description: Installing on an empty SD card
---


# Fresh install


## Step 1: Preparing the SD card (only necessary first time)


:::caution
The SD card included with the Miyoo Mini is known to be slow, and will likely ruin your experience and/or data.
Stick to using SD cards from reputable brands (e.g. SanDisk or Samsung).
:::

1. Format your SD card as `FAT32` (not `exFAT`). FAT32 is known as `MS-DOS (FAT)` on Mac. If your SD card is larger than 32 GB, we recommended using a tool like [rufus](https://rufus.ie/) or guiformat.


## Step 2: Copy the installation files and boot up your device to begin the installation

1. Extract the contents of the [`Onion-v4.x.x.zip`](https://github.com/OnionUI/Onion/releases/latest) file and copy _all_ 7 folders to the root of your SD card:

        1  .tmp_update
        2  BIOS
        3  miyoo
        4  miyoo354
        5  Media
        6  RetroArch
        7  Themes

:::note
Make sure that you've enabled viewing hidden folders on your operating system to correctly copy _all_ of the folders to your SD card (hit <kbd>Command</kbd>+<kbd>Shift</kbd>+<kbd>.</kbd> on Mac).
:::

2. The installation will begin when you insert your SD Card and boot up your Miyoo Mini.
3. When the installation is done, press the <kbd>A</kbd> button and the device will turn off.


## Step 3: Copy over your bios and rom files

1. Copy your bios files to the `BIOS` folder (stock bios files are available at `RetroArch/.retroarch/system` on the stock SD card).  
2. Copy your roms into their respective subfolders in `Roms` (see the *[Emulators](../emulators)* page for more information - here you'll find the [Rom Folders](../emulators/folders) table).

:::info
Despite its name, the `.tmp_update` folder must stay on your SD card.
:::

3. Insert your SD card, boot up your Mini and press <kbd>SELECT</kbd> from the `Games` tab in the main menu to refresh roms.
