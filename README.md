# widescreenUI_mod

This module is for the game Vampire: The Masquerade - Bloodlines.

I decided to code this external module to avoid changing the original libraries that the Unofficial Patch includes.
 
Author: L@Zar0\
Date: 27/03/2025\
Updated: 16/06/2025\
Version: 1.4k, based on Unofficial Patch 11.5\
Contact: formerly ClanDLAN.net member, now in Discord.

This module patches some visual aspects when using Widescreen resolutions. For better usage, I recommend using the textures included.\
The textures of this module are done specifically for 16:9 resolutions (and can be a bit compatible with 16:10), if you use it for 3:2, 25:16, 21:9, there could appear incorrect graphical proportions. Please, don't use this textures for 4:3 resolution, the default textures work well with that aspect ratio.\
The default textures of the game are prepared for 4:3 resolutions.

 The changes added to this module are:
* a.  Skip Intro checkbox in Character Creation is positioned correctly for Widescreen resolutions.\
  a1. Added checkbox DeltaXPos and CheckboxWidth values for wider texts
* b.	'Hotkeys' word in Hotkeys Window is centered for Widescreens resolutions.
* c.  Numbers in Hotkeys Window are REMOVED (only present left numbers) and added in texture (only WideScreen 16:9 resolutions).\
      I include a texture with drawn numbers.
* d.  Horizontal Dividers in Hotkeys Window are REMOVED (all Widescreen resolutions)
* e.  Dialog Box width fixed to be wider and show the lines INSIDE the box.
* f.  Fix for "Dividers" \material\interface\charactermaintenance\cm_divider" to align with windows
* g.  Fix for "Titles" of the windows (Options/Load/Save...) for some small resolutions
* h.  Set size of row manually for loading/saving tables
* i.  Fix for inventory buttons shadow outside boundaries
* j.	 Fix loading bar aspect ratio for some 16:9 and 16:10 widescreens

# Install

You will need to install the module (.vtm and .ini) in `\Bin\loader` folder in the path where Vampire the Masquerade - Bloodlines has been installed.

If you want to use the textures adapted to the 16:9 aspect ratio, you will need to uncompress the `materials` folder inside the path where Vampire the Masquerade UP 11.5 has been installed in case you want to use them.\
**IMPORTANT: YOU WILL OVERWRITE ANY OTHER TEXTURE THAT EXISTS IN THOSE FOLDERS**\
You can disable the numbers (keyword `enabled=1`) or enable them (keyword `enabled=0`) the Numbers in Hotkey features modifying the .ini file. You must use `enable=0` if using 4:3 resolutions.\
Unofficial Patch 11.5 by Wesp5 required.

# Acknowledgements
+ Niko from Planet Vampire Discord in appointing me to documentation for coding this.
+ Wesp5 from the Unofficial Patch for helping me test the mod and his knowledge.
+ Psycho-A from the Unofficial Patch for helping me test the mod and his observations.
