---
slug: /apps/clock
description: Set your Onions time
---

# Clock
<p><i>{frontMatter.description}</i></p>

## Presentation

Simple clock app which allows you to manually set the clock of your device. Especially usefull for the Miyoo Mini which doesn't have an internal RTC (which means that the time is reset at each boot). By default, the stop time is saved and increased of 4 hours on next boot.

![](./assets/clock.png)


:::note
On the Miyoo Mini Plus, Onion can use the wifi connection to set the time at each boot. Configure it from [Tweaks app](tweaks#set-automatically-via-the-internet).
:::

## Usage

Clock is available in [Package Manager](package-manager).

Use the D-pad to set the current date. 



## Advanced

[Clock source code](https://github.com/OnionUI/Onion/tree/main/src/clock)
