---
sidebar_position: 3
slug: /third-party
---

![Apps](https://user-images.githubusercontent.com/98862735/178884500-8188e34c-b140-4c8e-83d1-44ec4c3b5112.png)


Here you will find a selection of third party applications and tools compatible with Onion.


## Easy Logotweak

<sup>by Schmurtz</sup>

*An app for easily updating the Miyoo Mini boot logo.*

<p align="center">
<a href="https://github.com/schmurtzm/Miyoo-Mini-easy-logotweak"><img title="Easy Logotweak By Schmurtz" width="280px" src="https://user-images.githubusercontent.com/98862735/189468731-c8ee3660-92e2-4cf4-aba6-32ff38a264c0.png" /><br/>Get it here</a>
</p>

Many built in logos to choose from and the ability to add your own.  
This utility will modify your firmware, there is a (very low) risk of bricking your device if you do something wrong so please take the time to read and follow the instructions carefully. 


## Video Player (ffplay)

<sup>by BoboTraX</sup>

*A fully featured video player for Onion.*

<p align="center">
  <img src="https://user-images.githubusercontent.com/17168896/193084909-ec97ca37-3b0f-4433-a240-2ccc62421671.gif" alt="animated" />
</p>

Improving on the [Steward-fu release](https://steward-fu.github.io/website/handheld/miyoo-mini/parasyte_build_ffplay.htm), the dependency on Parasyte is now removed with necessary libraries included.  
Many formats are supported, after a few tests a little everything below 1080p is supported. The easiest way is to test!


### Installation

- Download the [latest version of ffplay](https://github.com/bobotrax/ffplay_Miyoo/releases) from the release page.
- Extract the contents of the archive to the root of the SD card.
- Place your videos in the `Videos` folder (Media/Videos), you can create Subfolders can be used inside your Videos folders but ONLY one level deep (i.e. Media/Videos/DargonBall).


### Usage

A new `Videos` item is now available in `Games` your videos will be accessible there.<br/>
You can delete a video with the <kbd>SELECT</kbd> key and click on `Delete`.<br/>
If you don't see your videos think to refresh the list with the key <kbd>SELECT</kbd> on the `Videos` item as for the `ROMS`.

<table align="center">
<thead>
<tr>
<th>Buttons</th>
<th>Action</th>
</tr>
</thead>
<tbody>
<tr>
<td><kbd>UP</kbd></td>
<td>+60 seconds</td>
</tr>
<tr>
<td><kbd>DOWN</kbd></td>
<td>-60 seconds</td>
</tr>
<tr>
<td><kbd>LEFT</kbd></td>
<td>-10 seconds</td>
</tr>
<tr>
<td><kbd>RIGHT</kbd></td>
<td>+10 seconds</td>
</tr>
<tr>
<td><kbd>A</kbd></td>
<td>Pause</td>
</tr>
<tr>
<td><kbd>B</kbd></td>
<td>Next frame <sub>(frame by frame)</sub></td>
</tr>
<tr>
<td><kbd>X</kbd></td>
<td>+10 minutes</td>
</tr>
<tr>
<td><kbd>Y</kbd></td>
<td>-10 minutes</td>
</tr>
<tr>
<td><kbd>L1</kbd></td>
<td>Cycle audio channel</td>
</tr>
<tr>
<td><kbd>L2</kbd></td>
<td>Cycle video channel</td>
</tr>
<tr>
<td><kbd>R1</kbd></td>
<td>Cycle audio, video and subtitle channel</td>
</tr>
<tr>
<td><kbd>R2</kbd></td>
<td>Cycle subtitle channel</td>
</tr>
<tr>
<td><kbd>START</kbd></td>
<td>Full screen / 16:9</td>
</tr>
<tr>
<td><kbd>SELECT</kbd></td>
<td>Display effect</td>
</tr>
<tr>
<td><kbd>MENU</kbd></td>
<td>Save and leave</td>
</tr>
</tbody>
</table>

### How to force 16:9 startup
1. On your SD card, go to the folder `Emu/ffplay/bin`
2. Rename the file `ffplay` to `ffplay_4_3`
3. Rename the file `ffplay_16_9` to `ffplay`
