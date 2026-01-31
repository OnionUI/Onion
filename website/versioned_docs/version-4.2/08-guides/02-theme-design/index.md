---
slug: /theme-design
---


# Theme design

*![](https://user-images.githubusercontent.com/98862735/178886298-516ac53b-a7da-4568-a788-f97ece6b6c67.png)*


## Theme Repository

The Onion Theme Repository has moved to its own *repository*.

[Go to Themes ➜](https://github.com/OnionUI/Themes/blob/main/README.md)


## Contributing

Do you want to share your own custom themes with the community? <sup><sub>❤️</sub></sup> [Click here !](https://github.com/OnionUI/Themes/blob/main/CONTRIBUTING.md)


## Template 

### Blueprint Theme <sub><sup>(Credit: Aemiii91)</sup></sub>

The *Blueprint* theme can be used as theming layout template for starting any new theme :

<p align="center"><a href="https://github.com/OnionUI/Themes/blob/main/release/Blueprint%20by%20Aemiii91.zip?raw=true"><img title="Blueprint by Aemiii91" width="280px" src="https://github.com/OnionUI/Themes/raw/main/themes/Blueprint%20by%20Aemiii91/preview.png?raw=true" /><br/>
Download the Blueprint theme</a></p>

Since Onion v4 you have additional assets in `skin/extra` folder: 
* `bootScreen.png` : Custom boot screen
* `Screen_Off.png` : Shutting down screen
* `Screen_Off_Save.png` : Saving + Shutting down screen
* `chargingState0.png` -> `chargingState23.png` : 24 images for charging animation, and you can add `chargingState.json` containing a frame delay value (default: 80 milliseconds - values over 10,000 will be parsed as microseconds):
  ```json
  { "frame_delay": 80 }
  ```
* `lum0.png` -> `lum10.png`: 11 images for brightness slider
* `toggle-on` and `toggle-off` -> little toggle which are present in Pack Manager and Tweaks for example.

### Default Fonts <sub><sup>(Credit: Aemiii91)</sup></sub>

It is no longer necessary to include the following fonts in your theme, just point the font paths in `config.json` to `/mnt/SDCARD/miyoo/app/[FONT FILE]`.

<details>
<summary><i>See the list of included fonts</i></summary><br/>

**BPreplay Bold**  
`/mnt/SDCARD/miyoo/app/BPreplayBold.otf`

> ![BPreplayBold.otf](https://user-images.githubusercontent.com/44569252/180784703-d18c8522-9ced-4edb-807e-bcc0f3cbf6c5.png)

<sup><a href="https://www.fontsquirrel.com/fonts/download/BPreplay">Download BPreplay</a> • <a href="https://www.fontsquirrel.com/license/BPreplay">OFL License</a></sup>


**Exo 2 Bold Italic**  
`/mnt/SDCARD/miyoo/app/Exo-2-Bold-Italic.ttf`

> ![Exo-2-Bold-Italic.ttf](https://user-images.githubusercontent.com/44569252/180785009-27df242d-6b42-4a09-9291-2694026abda3.png)

<sup><a href="https://www.ndiscover.com/exo-2-0/">Download Exo 2</a> • <a href="https://www.fontsquirrel.com/license/exo-2">OFL License</a></sup>


**Helvetica Neue**  
`/mnt/SDCARD/miyoo/app/Helvetica-Neue-2.ttf`

> ![Helvetica-Neue-2.ttf](https://user-images.githubusercontent.com/44569252/180785120-e3af9ea8-63d0-413a-ab11-712de82f81d3.png)

<sup>Free for personal use</sup>


**Helvetica Neue Bold**  
`/mnt/SDCARD/miyoo/app/HENB.ttf`

> ![HENB.ttf](https://user-images.githubusercontent.com/44569252/180785278-1aeb528e-2c86-4a4e-827a-ea14cd5e4fff.png)

<sup>Free for personal use</sup>


**WenQuanYi Micro Hei**  
`/mnt/SDCARD/miyoo/app/wqy-microhei.ttc`

> ![wqy-microhei.ttc](https://user-images.githubusercontent.com/44569252/180785427-496b49df-037c-4d3b-897b-ac20881aef44.png)

<sup><a href="http://wenq.org/wqy2/index.cgi?MicroHei(en)">Download Micro Hei</a> • GPL License</sup>


**Adobe Blank**  
`/mnt/SDCARD/miyoo/app/AdobeBlank.ttc`

*Use this font to hide specific text elements in your theme.*

> "Adobe Blank is a special-purpose OpenType font that is intended to render all Unicode code points using non-spacing and non-marking glyphs"

<sup><a href="https://github.com/adobe-fonts/adobe-blank">Download Adobe Blank</a> • <a href="https://github.com/adobe-fonts/adobe-blank/blob/master/LICENSE.txt">OFL License</a></sup>
</details>


## Theme Overrides

Since Onion v4 you can override some aspects of themes. This allows you to have elements which stay the same after updating and no matter which theme is applied.

**Example:** To apply your own charging animation, just put the files `chargingState0.png` ... `chargingState23.png` in this folder: `Saves/CurrentProfile/theme/skin/extra`.


## Theme Development Tools

### Onion Theme Viewer
<sup>(Credit: ruidacosta)</sup>

A theme viewer desktop application, available for both Mac and Windows.

<p align="center"><a href="https://github.com/syphen/onion-theme-viewer/releases"><img title="Theme Viewer" width="280px" src="https://user-images.githubusercontent.com/98862735/179321252-235d31ba-579a-46dd-a266-8cca32b8f4fd.png?raw=true" /><br/>
Get Onion Theme Viewer</a></p>


### HTML Preview page
<sup>(Credit: Weston Campbell)</sup>

Drop the html file in the theme folder and launch it.

<p align="center"><a href="https://github.com/OnionUI/Onion/files/9037560/Theme_Preview_2022MAY07.1.zip"><img title="Theme preview" width="280px" src="https://user-images.githubusercontent.com/16885275/167720395-196d8fd3-9cdc-4295-b49d-6d617feee8d0.png?raw=true" /><br/>
Download the Preview page</a></p>
