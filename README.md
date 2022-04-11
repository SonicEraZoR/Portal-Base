Portal
=====

This is the old Portal 1 source code ported to Source Engine 2013. I made this because I wanted to make proper Half-Life 2 with Portal gun, but I have no idea how to fix all of the bugs that occur in Portal on Half-Life 2 maps (some crashes, displacement collision not working near portals). So I hope someone more experienced with Source engine will help me fix them.

## Current Differences from original Portal
* You can fizzle portals by reloading portal gun
* Health regeneration is togelable with "sv_regeneration_enable" without annoying red tint
* You can toggle between Half-Life 2 gamerules and Portal gamerules with "sv_use_portal_gamerules". As far as I know it controls if you receive fall damage or not, but it might also control something else, I'm not really sure
* Flashlight is enabled
* NPCs can actually shoot you now (And even through portals! Because apparently Valve left some code for that)
* Backported crosshair from current version of Half-Life 2
* No crashes on the map load in Half-Life 2 and Episodes
* Even if "sv_regeneration_wait_time" is higher than 1 second, red tint will now be on the screen only for 1 second
* You won't get stuck in non-solid static props near portals

## Dependencies

### Windows
* [Visual Studio 2013 with Update 5](https://visualstudio.microsoft.com/vs/older-downloads/)

### macOS
* [Xcode 5.0.2](https://developer.apple.com/downloads/more)

### Linux
* GCC 4.8
* [Steam Client Runtime](http://media.steampowered.com/client/runtime/steam-runtime-sdk_latest.tar.xz)

## Building

Compiling process is the same as for Source SDK 2013. Instructions for building Source SDK 2013 can be found here: https://developer.valvesoftware.com/wiki/Source_SDK_2013

You will need Portal installed though

## Installing:

Same as Source SDK 2013 mod: https://developer.valvesoftware.com/wiki/Setup_mod_on_steam

P. S. This ReadMe was mostly copied from this repo https://github.com/NicknineTheEagle/TF2-Base
