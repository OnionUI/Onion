---
slug: /network/netplay
---


# Easy Netplay


*![](https://github.com/OnionUI/Onion/assets/47260768/031e60fa-e6dd-4059-9982-3ec397a3d0cd)*

*Fight against your friends or fight alongside them, the easy way!*

**Easy Netplay:** a feature for convenience that streamlines multiplayer gaming. This tool effortlessly sets up a hotspot, launches RetroArch and enables Netplay on the host side. On the client side, it joins the hotspot, checks the core and ROM checksums and connects to the RetroArch session, ensuring a seamless and enjoyable multiplayer experience.

When you've finished playing, provided you exit RetroArch through the menu (and don't power down!) the script will continue to work, re-connecting you to the original Wi-Fi network. 


## Features

- 1-click host setup

- 1-click client setup

- Rom/core checksum verification for higher compatibility


## Enabling Easy Netplay

You'll currently find the quick join feature in GLO. GLO can be opened by browsing to the rom you want to play and pressing Y. 


### As the host

To host a rom you'll first need to make sure you're using a core that can support netplay. Cores can be verified using [this sheet](https://docs.google.com/spreadsheets/d/1y1esG3HcNagPG8cy-ulZK_mhhUb93pqG5_XEtRRgKQ4/edit#gid=0)

1. Find the rom in the Games submenu

2. Press <kbd>Y</kbd> to open GLO

3. Choose **Easy Netplay: Host a game**
 
Onion should now take over and setup a session on the built in personal hotspot.


### As the client

To join a host, it's a little easier. Currently you'll start the hotspot client connection using the same process as the host. 

1. Navigate to any game in the Games submenu

2. Press <kbd>Y</kbd> to open GLO

3. Choose **Easy Netplay: Join a game**
 
Onion will now take over and join the hotspot, pull the information from the host and connect! 


## Example

### Host: 

![](https://github.com/OnionUI/Onion/assets/47260768/e319297d-d65d-4060-9fa0-174d9c3b4516)

### Client:

![](https://github.com/OnionUI/Onion/assets/47260768/4d6bb983-e986-47b6-8810-17cd9e15f553)

:::note
If you change the hotspot password on the host then this process will fail. It should be left as `onionos+`.
:::
