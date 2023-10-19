---
slug: /multiplayer/standardnetplay
---

# Standard Netplay


*![](https://github.com/OnionUI/Onion/assets/47260768/031e60fa-e6dd-4059-9982-3ec397a3d0cd)*

*Fight against your friends remotely!*

**Standard Netplay:** a feature to create and join Retroarch multiplayer sessions easily.

Each game you'll create will be available over internet and your local network, making it easy for your friends to join in your game.



## Features

- Easy host setup

- Optimized Retroarch configuration

- Automatic core selection for Netplay

- CPU performance boots when it's needed


## Using Standard Netplay


### As the host

1. Find the rom in the Games submenu

2. Press <kbd>Y</kbd> to open GLO

3. Choose `Netplay` -> `Host` -> `Standard Netplay (use current wifi)`
 
Your Onion netplay session is now ready to be joined from your local home network or even on the other side of the world.


### As the client

1. Select the same game as the host

2. Press <kbd>Y</kbd> to open GLO

3. Choose `Netplay` -> `Join` -> `Standard Netplay (use current wifi)`

4. Press Menu + Select to display the retroarch menu, go into Netplay and then click on :
	- `Refresh Netplay Host List` -> to display hosted games over internet
	- `Refresh Netplay Lan List` -> to display hosted games over your local network
	
5. Select your target server in the list.
 
---

## Example

### Host: 




import NetStdHost from './assets/Netplay - standard - host.mp4';

<video controls>
  <source src={NetStdHost}/>
</video>



### Client:

import NetStdJoin from './assets/Netplay - standard - join.mp4';

<video controls>
  <source src={NetStdJoin}/>
</video>


