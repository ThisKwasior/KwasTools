# KwasTools
Library (kwaslib) and tools for video game modding.

## Requirements
* [CMake](https://cmake.org/)
* GCC/MinGW-w64 (For Windows, I recommend [winlibs MSVCRT](https://winlibs.com/), but [MSYS2](https://www.msys2.org/) is also fine)
* git (Like above, [Git for Windows](https://gitforwindows.org/) or [MSYS2](https://www.msys2.org/))

**This project does not support Visual Studio.**

## Building

### Windows
```
$ git clone https://github.com/ThisKwasior/KwasTools
$ cd KwasTools
$ mkdir build && cd build
$ cmake .. -G"MinGW Makefiles"
$ mingw32-make -j 8
```

### Linux/MSYS2
```
$ git clone https://github.com/ThisKwasior/KwasTools
$ cd KwasTools
$ mkdir build && cd build
$ cmake ..
$ make -j 8
```

After compiling, CMake script will output binaries and libraries in the `./bin` directory in the root of the repo.

## Software
### Hedgehog Engine
* `he_anim_tool` - Hedgehog Engine anim converter. Supports uv-anim, cam-anim, vis-anim, morph-anim, pt-anim, mat-anim and lit-anim.

### CRIWARE
* `cri_utf_tool` - CRI UTF parser to and from XML. Will parse structures from VLDATA (UTF, AWB and ACB Commands).
* `cri_afs_tool` - CRI AFS packer/unpacker. Supports creating AFS files from custom XML config and unpacks AFS files to a folder and XML config.
* `cri_awb_tool` - CRI AWB packer/unpacker. Supports creating AWBs from custom XML config and unpacks AWBs to a folder and XML config.
* `CRI Scramble Key` - Python script and HTML page to compute ACB key for encoding HCA/ADX files. Found in `scripts` directory. Also [hosted on my site](https://thiskwasior.ct8.pl/cri_scramble_key.htm).

### NW4R
* `nw4r_misc_to_he_xml` - Converts SCN0 and SRT0 from Wii NW4R to Hedgehog Engine XML supported by he_anim_tool.

### Platinum Games
* `platinum_dat_tool` - DAT unpacker/packer with experimental support for Big Endian archives (X360/PS3)
* `platinum_wtb_tool` - WTB/WTA+WTP unpacker/packer with WIP X360/PS3 texture conversion to PNG
* `platinum_misc_dat2dds2dat` - (At the moment not usable) WIP WTA/WTP/WTB unpacker/packer directly from DAT/DTT

## Addons
### io_kwastools
WIP addon for Blender 3.0/4.0 (currently not usable as of 06/06/2025).

Features:
- cam-anim import and export
- uv-anim import and export
- Camera setup creation for cam-anim (searchable with F3)
- UV Animator creation for uv-anim (searchable with F3, to use in material->Shift+A->Group)

## Special Thanks
* [ik-01](https://github.com/ik-01) for multiple contributions, both in code and knowledge, in Hedgehog Engine formats.
* [ecksdii](https://github.com/ecksdii) for extensive testing and end-user feedback.
* [Ashrindy](https://github.com/Ashrindy) for contributions and help with Hedgehog Engine `-anim` formats.
* [Adam Gąsior](https://github.com/anhosh) for giving feedback on my code whenever I ask.
* [Skyth](https://github.com/blueskythlikesclouds) for help with Hedgehog Engine related issues and insight.
* [Mario Tainaka](https://github.com/MarioTainaka) for help with CRI Middleware related issues and insight.
* [Sewer56](https://github.com/Sewer56) for help with CRI Middleware related issues and insight.
* Metal Gear Rising Revengeance modding community for feedback and bug reports.
* And anyone else I forgot to mention!

## Acknowledgements
* [vgmstream](https://github.com/vgmstream/vgmstream) - Learned about the CRI `@UTF` file format from their extensive implementation.
* [Xenia](https://github.com/xenia-project/xenia) - Untiling code for Xbox 360 textures. Copyright (c) 2015, Ben Vanik. All rights reserved.
* [RPCS3](https://github.com/RPCS3/rpcs3) - Texture swizzling code for GTF textures. Licensed under GNU GPL-2.0 license.
