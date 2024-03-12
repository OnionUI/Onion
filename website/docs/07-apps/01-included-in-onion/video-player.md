---
slug: /apps/video-player
description: A fully featured video player for Onion.
---

# Video Player
<sup>by Steward Fu & BoboTraX</sup>

## Presentation

A fully featured video player for Onion based on FFplay.

<p align="center"><img src="https://user-images.githubusercontent.com/17168896/193084909-ec97ca37-3b0f-4433-a240-2ccc62421671.gif" alt="animated" /></p>


## Installation

- Run Package Manager from apps section and install the Video Player
- Place your videos in `Media/Videos` folder of your SD card, using subfolders is possible


## Usage

Many formats are supported, after a few tests a little everything below 1080p is supported. The easiest way is to test!

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
<td>Full screen/16:9</td>
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

## Advanced

#### How to force 16:9 startup

1. On your SD card, go to the folder `.tmp_update/bin`

2. Rename the file `ffplay` to `ffplay_4_3`

3. Rename the file `ffplay_16_9` to `ffplay`

#### Original repositories

Improving on the [Steward-fu release](https://steward-fu.github.io/website/handheld/miyoo-mini/parasyte_build_ffplay.htm), the dependency on Parasyte is now removed with only necessary libraries included.  

- [Original compilation by Steward Fu](https://steward-fu.github.io/website/handheld/miyoo-mini/parasyte_build_ffplay.htm)

- [Edited by BoboTraX](https://github.com/bobotrax/ffplay_Miyoo)
