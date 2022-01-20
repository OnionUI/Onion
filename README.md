![githubcover](https://user-images.githubusercontent.com/85693713/148580152-0bc4aec1-310d-405c-8ab3-e7655991a7f3.png)

**Custom Overhaul Solution for Miyoo Mini. Made by the community FOR the community.<br>
Pre-configured, streamlined, fully skinnable and stuffed to the gills with retro systems.**

Disclaimers - If you read nothing else on this page, read this part: 
<br>Proceed with caution and use at your own risk. (It all boots off SD so there really should be zero risk)
<br>Please submit feedback/bugs/core requests to onionsarentgross@gmail.com or on the github tracker. If you ask on Discord I may miss it!
<br>Do not report bugs for the cores in the "Expert" section, those are provided for experimental purposes and are mostly untested. Report fixes and enhancements only.

Install Instructions:
IMPORTANT: You must have a charged battery and be on the 20220108 firmware or a previous version of Onion. (Flashing instructions: https://github.com/jimgraygit/OnionOS/wiki/Official-Miyoo-Firmware)
1) Make a backup of your Sd Card
2) Unzip and copy the folder "The_Onion_Installer" in your App folder
3) Make sure that the hibernation timer is disabled (Settings page > Hibernate > Never)
4) Launch The Onion Installer from the App section
4) Wait for your unit to turn off. (NOTE: A new installation can take upwards of 15 minutes, do not panic!)

Known Issues (Most are stock kernel related):
- Some NES roms crash. Seems to only pertain to ones that are meant for virtual console on 3DS.
- Occasionally after adding or removing ROMs, ROM titles will vanish until you select them. to fix this navigate to your Apps section and run the "Nuke" app.
- Large romsets (500ish) crash the menu back to the home screen when scrolling. The current workaround is to press right on the D-Pad to enter coverart preview mode and scroll through the list there.
- Due to a memory leak at the kernel level, sometimes after switching roms or systems multiple times (typically over a dozen), the UI glitches out and requires a reboot.
