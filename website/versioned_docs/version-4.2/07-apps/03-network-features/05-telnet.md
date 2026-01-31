---
slug: /network/telnet
---

# Telnet

*![](https://github.com/OnionUI/Onion/assets/47260768/62ee0d6c-1cce-43a4-976a-c8212850bf2f)*

Telnet provides an unencrypted command line method to communicate with your Miyoo Mini+.


## Features

*![](https://github.com/OnionUI/Onion/assets/47260768/64e1bf60-3670-4e84-b0f8-89f3575cc378)*

- Command-Line Interface

- Scripting and automation

- Debugging and troubleshooting


## Enabling Telnet

You'll find it in **Apps** › **Tweaks** › **Network** › **Telnet**

This is a master level toggle only and does not contain a submenu.


## Connecting

Once you've activated your telnet server in Tweaks you'll now be able to connect using software such as PuTTY, SolarPuTTY, Hyperterm.

You'll need the IP of the device, found below (**Tweaks** › **Network**)

![](https://github.com/OnionUI/Onion/assets/47260768/94f2ab4f-f776-4bb3-9edf-fa05ef0a88ba)


## Authentication

We've made the choice to remove authentication on Telnet as it is unsecure and will transmit the password in plain text - Telnet is now disabled by default.


## Security

:::caution Network security
Although we've taken every precaution to offer as much security as possible, remember to keep your Onion safe. It is not recommended you use telnet on an insecure Wi-Fi network that is open or public as traffic is unencrypted and transported in plain text, for this reason we strongly recommend you toggle telnet off when you're on the move!
:::
