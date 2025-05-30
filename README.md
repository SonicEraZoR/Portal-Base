Portal
=====

[![Build status](https://ci.appveyor.com/api/projects/status/g0cv8na9uq9tmadt/branch/master?svg=true)](https://ci.appveyor.com/project/SonicEraZoR/portal-base/branch/master)

This is the old Portal 1 source code ported to Source Engine 2013. I made this because I wanted to make proper Half-Life 2 with Portal gun, but I have no idea how to fix all of the bugs that occur in Portal on Half-Life 2 maps (some crashes, displacement collision not working near portals). So I hope someone more experienced with Source engine will help me fix them.

## Current Differences from original Portal
* New "Mod Options" menu
* You can erase portals by reloading portal gun
* Health regeneration is toggleable with "sv_regeneration_enable" or in "Mod Options" menu without annoying red tint
* You can toggle between Half-Life 2 gamerules and Portal gamerules with "sv_use_portal_gamerules". As far as I know it controls if you receive fall damage or not, but it might also control something else, I'm not really sure
* Fall damage is toggleable with "sv_receive_fall_damage" or in "Mod Options" menu (only for Half-Life 2 gamerules, Portal's gamerules always have fall damage disabled
* Flashlight is enabled
* NPCs can actually shoot you now (And even through portals! Because apparently Valve left some code for that. Only at specific angles though)
* Backported crosshair from current version of Half-Life 2
* No crashes on the map load in Half-Life 2 and Episodes
* Even if "sv_regeneration_wait_time" is higher than 1 second, red tint will now be on the screen only for 1 second
* You won't get stuck in non-solid static props near portals
* You can change the **maximum** delay between shooting a Portal and it opening with sv_portal_projectile_delay (default is 0.5 seconds), if set to a very high number behavior will be the same as in normal Portal
* You can rapid click to shoot Portals faster, like in Portal 2
* You can set Chell playermodel (only portal gun animations work properly on the stock model though), Gordon playermodel or default Half-Life 2 playermodel with "*cl_playermodel "models/player/chell.mdl"*", "*cl_playermodel "models/sirgibs/ragdolls/gordon_survivor_player.mdl"*" or "*cl_playermodel "models/player.mdl"*' respectively or in "Mod Options" menu.
* There is HUD QuickInfo (aka crosshair) from Portal's beta. Toggleable with "beta_quickinfo 1|0" or in "Mod Options" menu (the code for it was taken from here and improved upon https://github.com/reepblue/project-beta/blob/master/src/game/client/project-beta/hud_quickinfo.cpp)
* Crowbar swings through Portals
* No crash when Magnusson Device hits the ground
* "impulse 102" gives portal gun
* There's portal gun bind in the settings
* Animation system now properly displays third person animations.
* New animations for the third person with a new Gordon model.
* A bunch of stuff has been added to the portal's trace filter, which means that a lot of NPCs and items don't block portals' projectile now.
* The Emplacement Gun works.
* The portal gun's "lowering" animation was removed (plays when looking at a friendly NPC) since it just enlarges portal gun with a stock model. Can be enabled back with "*allow_portalgun_lowering_anim 1*" or in the Mod Options menu.
* Fixed a bug of music playing twice on some maps when portals are placed. It was fixed by removing teleportation of sound though, so kind of a band-aid fix, but the sound teleporting wasn't that useful anyway imo...
* The reorientation of player's camera when go through a portal is faster just like in Portal 2. And you can adjust it by changing "cl_reorient_rate" and "cl_reorient_acceleration_rate" in console (Default value in Portal 1 is 120 and 400). [PR](https://github.com/SonicEraZoR/Portal-Base/pull/14).
* The camera detach counter is fixed. Check [PR](https://github.com/SonicEraZoR/Portal-Base/pull/19) for more context.

## Some things that don't work properly (yet, hopefully they will someday)
* **There's no collision for displacements near portals**. Which means you will **fall through** displacements close to portals. Displacements are usually used to make bumpy ground or  hills, valleys, trenches, slopes etc. though they also can be flat and just used to blend two or more textures together. It's a pretty complicated issue to fix, so for now get your noclip ready. You can also just avoid displacements near portals by jumping over them for example because it's usually pretty easy to distinguish displacements from normal brush terrain since the brushes are **always** flat
* There are some visual glitches with portal gun, some of them can be fixed by saving and loading
* Sometimes the game will crash if you place your portals in certain places (like on the floor of the red barn in d1_canals_06) or if you push physics engine too far
* Guns are still attached to NPC's hands when they're supposed to be hidden
* Then you grab something with gravity gun and go through portal weird glitches can happen
* Particle (and some other colors) are inverted on Linux

## Demonstration (version 0.8-beta though, so it's a bit outdated)
[![Demonstration](https://img.youtube.com/vi/xhmXAUB8P4Y/0.jpg)](https://www.youtube.com/watch?v=xhmXAUB8P4Y)

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

Same as Source SDK 2013 mod: https://developer.valvesoftware.com/wiki/Setup_mod_on_steam. You'll have to use steam_legacy beta branch for everything to work properly since the mod hasn't been tested to work with 20th anniversary update.

## Credits

* Portal Gun weapon icon by RealBeta, uploaded to GameBanana by RaraKnight. [Link](https://gamebanana.com/guis/34111). 
* Mod icon courtesy of u/Leodemerak on Reddit. [Link](https://www.reddit.com/r/HalfLife/comments/10nt67r/halflife_series_and_mods_icon_reworki_wanted_to/).
* Enhanced Survivor Gordon Freeman by Sirgibsalot. [Link](https://steamcommunity.com/sharedfiles/filedetails/?id=155216268).
* Portal gun world model from [this pack](https://gamebanana.com/mods/235155). Needed for Gordon's portal gun animations to work.
* Portal gun animations themselves for the Gordon model were converted from [this](https://gamebanana.com/mods/235102) model.
* Bugbait world model is from Synergy, needed for bugbait animations to work.
* World models for the Magnum, crossbow, grenade and gravity gun that have support for the third person animations are from Half-Life 2 Deathmatch.
* Font for the main menu logo is by AmoMeDi. [Link](https://fontmeme.com/fonts/portal-amomedi-font/).

## P. S.

If you want the game to behave exactly like normal Portal first use *Half-Life 2 binaries not Episodic ones*, second, create autoexec.cfg in your mod directory and add those commands:
```
sv_regeneration_enable 1
sv_use_portal_gamerules 1
cl_playermodel "models/player/chell.mdl"
```
And it should be just like normal Portal
