---
slug: /network/samba
---


# Samba file sharing

The Samba is a file sharing protocol that provides integrated sharing of files and directories between your Miyoo Mini plus and your PC (Windows/Unix/Linux). It enables easy access, allowing you to share, access, and modify files as if they were stored locally on your PC. 


If the folder you try to access does not exist locally on the device you will get an error.


## Features

- File transfer

- Mounted drive in Windows

- Instant file visibility

- Customizable share folders


## Enabling Samba

You'll find it in **Apps** › **Tweaks** › **Network** › **Samba: Network file share** 

<img src="https://github.com/OnionUI/Onion/assets/6023076/5ddbac5b-127d-44ea-8409-d3b931c8b1ca" width="400" />


Make a note of the IP address at the top here, you'll need it for the next steps!

:::note DEFAULT CREDENTIALS
The default **username** and **password** is currently `onion`:`onion`
:::


## Usage

Samba will provide you a streamlined method of transferring files across to your Miyoo Mini Plus. The folders you can access will be hosted by the device, and will be accessible through your file browser within Windows/Linux.

Once you've enabled Samba within tweaks, it will instantly activate. 

To browse to your shared folders within Windows:

1. Hit the start button

2. Now, type in the IP address of your MMP in this format: \\\x.x.x.x, e.g: 

   ![](https://github.com/OnionUI/Onion/assets/47260768/9cf02397-09fb-4517-8ede-7b6dc44ee2b4)

3. Press Enter. It may take a second but an explorer window will open and you'll be met with some pre-configured locations we thought would get you started! 

   ![](https://github.com/OnionUI/Onion/assets/47260768/632782d4-b827-44ca-9806-7102a239200f)

4. If you have Network File Sharing enabled on Windows, you'll also be able to pin the location to your `quick access` by finding it in the left pane of your explorer window, and clicking `Pin to Quick access`

   ![](https://github.com/OnionUI/Onion/assets/47260768/2d5c7366-40ad-4992-ae9d-86e1c9019fb6)

5. The shares will now appear in your quick access pane:

   ![](https://github.com/OnionUI/Onion/assets/47260768/5cbcaf2c-f4eb-4921-9fc3-17974f0ddf9e)

:::tip Network shares
The subfolders can also be mapped as network shares by right clicking on "This PC" or "My Computer" and clicking "Map network drive" - Bare in mind you will have the map to a share directory instead of the root; for example: `\\192.168.1.215\screenshots` - Give it a drive letter and it's ready!
:::


## Share control

We've made controlling shares as easy as possible; from the "Samba: Network file share..." menu item, press A. You'll now be met with a list of shares, these can be instantly toggled on or off.

![](https://github.com/OnionUI/Onion/assets/47260768/1cf85fc9-1583-4c1f-bce6-b441645e1cde)


## Adding more shares (or removing them!)

It's your Onion.. So let's get you setup with more (or less!) shares. We'll need to edit a file called smb.conf which is found in `/mnt/SDCARD/.tmp_update/config/smb.conf`. 

As always, there's many ways to access this file. You can use [FTP](https://github.com/OnionUI/Onion/wiki/FTP)/[HTTP](https://github.com/OnionUI/Onion/wiki/HTTP-fileserver) or pull out the SD card and do it the manual way. 

To add a share folder, just make a copy of and existing entry and modify the path/name, restart Samba in tweaks and you're ready to go - it'll automatically appear in your Samba menu in tweaks and be ready to access.


## Security

Samba offers auth at a unix user level. The current username and password is onion:onion

You'll see there's a share in `smb.conf` called "__sdcard"; this should be left disabled unless you're absolutely sure you want to share the whole SD folder structure.
