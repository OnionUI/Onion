---
slug: /apps/quick-guide
---

# Quick Guide

*Getting started with Onion*

## Presentation

Shows the quick guide that is shown after installing Onion

<p align="center"><img src={require('./assets/quick-guide.webp').default} style={{width: 320}} /></p>

## Usage

### Controls

| Button | Function                                                 |
| ------ | -------------------                                      |
| <kbd>D-pad</kbd>  | Navigate                                      |
| <kbd>A</kbd>      | Next slide                                    |
| <kbd>B</kbd>      | Previous slide                                |
| <kbd>X</kbd>      | Check/Uncheck everything in the current tab   |
| <kbd>Y</kbd>      | Reset all the current modification            |
| <kbd>L1/R1</kbd>  | Previous/Next tab                             |
| <kbd>L2/R2</kbd>  | Previous/Next item page                       |
| <kbd>Start</kbd>  | Install summary/validate the modifications  |


## Advanced

Quick Guide app and Screenshot Viewer app use the same binary called `infoPanel`. It use the `-d` command to read all the png files from a directory passed in parameter.
