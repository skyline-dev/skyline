# Skyline
An enviroment for linking, runtime hooking and code patching in Super Smash Bros Ultimate

# Usage
1. Head to the Release page
2. Download the latest release (skyline.zip)
3. Copy the ``exefs`` directory on your SD in the following directory: ``atmosphere/contents/<game titleid>/``
4. Remove the ''main.npdm'' file from the newly added directory unless you are modding ``Super Smash Bros. Ultimate``.  
This file has to be made on a game-by-game basis and will usually be provided by plugin makers or made yourself using a mix of ``HACTool`` and ``npdmtool``.
5. If you wish to install a plugin, drop it in the following directory: ``atmosphere/contents/<game titleid>/romfs/skyline/plugins/``

# Compatibility issues
Skyline's way of initializing is not compatible with every game out of the box due to various reasons that can't be anticipated.  
Mod developers might sometimes modify Skyline to make it compatible with a specific game which you should use if you are running into crashes.

Here are links to a few derivates (that you might have to build yourself):
- [Animal Crossing New Horizon](https://github.com/3096/skyline)
- [Pokémon Sword/Shield](https://github.com/3096/skyline/tree/sword)
- [Xenoblade Chronicles Definitive Edition](https://github.com/3096/skyline/tree/xde)
- [Dragon Quest XI S](https://github.com/3096/skyline/tree/jack)
- [Persona 5 Royal](https://github.com/Raytwo/p5rcbt)
- [Persona 5 Strikers](https://github.com/Raytwo/masquerade-rs)
- [Fire Emblem Three Houses](https://github.com/three-houses-research-team/aldebaran-rs)

# Contributors
This project is derived from OdysseyReversed and Starlight
- [3096](https://github.com/3096)
- [khang06](https://github.com/khang06)
- [OatmealDome](https://github.com/OatmealDome)
- [Random0666](https://github.com/random0666)
- [shadowninja108](https://github.com/shadowninja108)
- [shibbo](https://github.com/shibbo) - Repo derived from their work on OdysseyReversed
- [Thog](https://github.com/Thog) - Expertise in how rtld is implemented
- [jakibaki ](https://github.com/jakibaki) - Advice with numerous things, including runtime hooking

# Credits
- devkitA64
- libnx - switch build rules
