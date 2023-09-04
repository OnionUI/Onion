---
slug: /installation/update
description: Upgrading from stock or Onion
---


# Upgrading


*When upgrading, your save states, in-game saves, roms, bios files, and
configs will carry over to the updated system.*

:::caution
For some systems, save states won't always work after upgrading,
please **make sure to create in-game saves** before upgrading, then you can
delete any non-working save states after upgrading.
:::


## Step 1: Copy the installation files

1. Extract the contents of the [`Onion-v4.x.x.zip`](https://github.com/OnionUI/Onion/releases/latest) file and copy _all_ 7 folders to the root of your SD card:

        1  .tmp_update
        2  BIOS
        3  miyoo
        4  miyoo354
        5  Media
        6  RetroArch
        7  Themes

2. Click "Yes" when prompted to overwrite (or "Merge" if you're on Mac).

:::note
Make sure that you've enabled viewing hidden folders on your operating system to correctly copy _all_ of the folders to your SD card (hit <kbd>Command</kbd>+<kbd>Shift</kbd>+<kbd>.</kbd> on Mac).
:::

:::caution
If you're upgrading from v3.9 or below, we recommend you delete the `.tmp_update` folder first.
:::


## Step 2: Boot up your device to begin the installation

1. The installation will begin when you boot up your Miyoo Mini.

:::note
If you've already got Onion installed, the installer will only update Onion, RetroArch, emulators, and apps - you keep all your settings.
:::

2. When the installation is done, press the <kbd>A</kbd> button and the device will turn off.


## Step 3: Check that your files are in the correct folders

1. If they're not already in place, transfer your bios files to the `BIOS` folder and your roms in their respective subfolders inside the `Roms` folder (see the *[Emulators](../emulators)* page for more information - here you'll find the [Rom Folders](../emulators/folders) table).

:::info
Despite its name, the `.tmp_update` folder must stay on your SD card.
:::

2. Insert your SD card, boot up your Mini and press <kbd>SELECT</kbd> from the `Games` tab in the main menu to refresh roms.


:::tip
If you experience any issues during the install, try copying over the installation
files again and use another upgrade option (you can skip copying the `BIOS`,
`Icons`, and `Themes` folders).

If you still have issues after trying the reinstall option, you may have to reformat
your SD card (remember to backup the `Roms` and `Saves` folders).
:::
