DSfix Developer Readme
======================

This file is intended for developers who want to improve DSfix or use it as a base for other modifications.

All the source code is released under the conditions of the GPLv3, except for
- SuperFastHash
- SMAA
which have their own licensing terms included.

Note that the code base is currently in quite a terrible state. 
TODO.txt lists some of the things that should be done to improve it in the "Cleanup" section.

Also note that I would prefer for no one except myself to make binary releases of DSfix for now, 
simply to keep things easier to track for people and reduce confusion.

Additionally, if you start working on some feature, I'd appreciate if you contacted me, so that we don't duplicate effort.

- Peter


Requirements
============

- Visual C++
- The DirectX SDK
- Microsoft Detours (Express) 3
- DSfix must be compiled in the Release/Win32 config to work with the game


File Overview
=============

- The "DATA" folder contains the files for distribution, including .inis, effects and textures
- "TODO.txt" contains a list of open (and closed) TODOs, look here if you want to find something to work on
- "main.*" includes the main function and a few utilities

- The "d3d9*" files implement d3d wrapping
- The "dinput*" files are very straightforward dinput wrapping
- "Detouring.*" files implement function overriding using the Detours library

- "KeyActions.*" files implement keybindings, together with the Xmacro files "Keys.def" and "Actions.def"
- "SaveManager.*" files implement save backup management
- "WindowManager.*" files implement window management (cursor hiding & capturing, borderless fullscreen)

- "RenderstateManager.*" is where most of the magic happens, implements detection and rerouting of the games' rendering pipeline state
- "SMAA.*", "VSSAO.*", "GAUSS.*" and "Hud.*" are effects optionally used during rendering (derive from the base Effect)
- "Textures.def" is a database of known texture hashes

