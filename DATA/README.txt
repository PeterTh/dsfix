DSfix 1.5 by Durante -- README
==============================

**Please read this whole document before running anything!**

As most of you may know, the Dark Souls port for PC has a fixed internal rendering resolution of 1024x720. 
I couldn't believe this when it was first rumoured, and when it turned out true I vowed to do my best to fix it.
DSfix is a tool developed by me that does just that.

What is it?
===========
It's an interception DINPUT8.dll that you place in the same folder as the game executable. It intercepts the game's calls to the DirectX 9 API and changes them as necessary to enable a higher internal rendering resolution.

How do I use it?
================
1) Delete previous version of the mod if you have any
2) Place the contents of the .zip into the game's binary directory. (The place where DARKSOULS.exe is)
  (this may be something like C:\Program Files (x86)\Steam\steamapps\common\Dark Souls Prepare to Die Edition\DATA)
3) Adjust the settings in DSfix.ini as desired
4) Adjust the keybindings in DSfixKeys.ini as desired
IMPORTANT: turn OFF the in-game AA option, otherwise you *will* get graphical issues
See DSfixKeys.ini for a list of keybindable actions, including screenshots.

Will it work?
=============
As of version 0.4, it seems to work well for many people, however your mileage may vary.
*I can not and will not guarantee that it will work for anyone else, or not have any adverse effect on your game/computer. Use at your own risk!*

Are there known issues?
=======================
- At vertical resolutions higher than 720 the sun may not be rendered correctly under some circumstances. This is under investigation, any insight is appreciated.
- When using custom HUD settings damage numbers and subtitles are not handled correctly

Will it cause performance problems?
===================================
That depends on your system configuration. Usually, performance scales rather linearly with framebuffer size, and so far this game does not seem different. My 660 maintained a locked 30 FPS throughout ~ 1/2 hour of testing in the starting area of the game at 2560x1440.
AMD cards seem to have more problems than NVidia at this point. Performance problems with the game may not be related to my mod but also CPU power.

Can I donate?
=============
If you really want to donate I won't say no, I'm not particularly rich :P.
Here is my blog post about the fix with a donation link:
http://blog.metaclassofnil.com/?p=92
You can also simply Paypal to peter@metaclassofnil.com

Will you release the source code?
=================================
I plan to, once the worst bugs are fixed and it is cleaned up *a lot*.

It crashes, help!
=================
First, make sure that the DSfix.ini file is present in the correct location.
Additionally, the "dsfix" folder should be in that same location.
Second, turn off tools such as MSI Afterburner or other overlays that manipulate D3D.
Then try restoring the default settings in the .ini file
If neither of these help then check if the problem still occurs when you remove/rename DINPUT8.dll
Finally, try rebooting
If it still persists, then report the problem, otherwise it has nothing to do with DSfix.

How can I uninstall the mod?
============================
Simply remove or rename DINPUT8.dll
The mod makes *no* permanent changes to *anything*.

Some other points
=================
- Please refrain from disrespectful remarks like "lol Japanese development". There are plenty of Japanese developers that deliver technically excellent PC games
- Buy Dark Souls if you like hardcore action RPGs

Thanks to
=========
- TatniumD3D developers for providing a basic D3D9 interception code base
- From Software for Dark Souls
- The authors of SMAA
- The authors of the OBGE VSSAO effect
- The artists of:
Crow's Claw, Demetori, Silver Forest, Foreground Eclipse, Thousand Leaves, Undead Corporation, Unlucky Morpheus, Dark Phoenix, Aquaelie, TAMUSIC, Riverside, Kissing the Mirror, xi-on, SAVE THE QUEEN, East New Sound and Shinigawa Satellite
for providing the development soundtrack


Contact information
===================
Contact me at peter@metaclassofnil.com
