# KwasTools
Library (kwaslib) and tools for video game modding.

## Requirements
* CMake
* GCC/MinGW-w64
* git

## Building

### Windows
```
$ git clone --recurse-submodules https://github.com/ThisKwasior/KwasTools
$ cd KwasTools
$ mkdir build && cd build
$ cmake .. -G"MinGW Makefiles"
$ mingw32-make -j 8
```

### Linux/MSYS2
```
$ git clone --recurse-submodules https://github.com/ThisKwasior/KwasTools
$ cd KwasTools
$ mkdir build && cd build
$ cmake ..
$ make -j 8
```

After compiling, all tools and libraries are located in `KwasTools/bin`.

## Software
### Hedgehog Engine
* `he_anim_tool` - Hedgehog Engine anim converter. Supports uv-anim, cam-anim, vis-anim, morph-anim, pt-anim and mat-anim

### CRIWARE
**Everything here will be moved to [CryTools](https://github.com/ThisKwasior/CryTools) repo sometime in the future. I'm keeping it all in KwasTools until I rewrite everything in there with kwaslib.**
* `cri_utf_tool` - CRI UTF parser to and from XML. Will parse structures from VLDATA (UTF, AWB and ACB Commands).
* `cri_awb_tool` - CRI AWB packer/unpacker. Supports creating AWBs from folder of files, custom XML config and unpacks AWBs to folder and XML config.

### NW4R
* `nw4r_misc_to_he_xml` - Converts SCN0 and SRT0 from Wii NW4R to Hedgehog Engine XML

### Platinum Games
* `platinum_dat_tool` - DAT unpacker/packer with experimental support for Big Endian archives (X360/PS3)
* `platinum_wtb_tool` - WTB/WTA+WTP unpacker/packer with WIP X360/PS3 texture conversion to PNG
* `platinum_misc_dat2dds2dat` - (At the moment not usable) WIP WTA/WTP/WTB unpacker/packer directly from DAT/DTT

## Addons
### io_kwastools
WIP addon for Blender 3.0/4.0. 
Current features:
- cam-anim import and export
- uv-anim import and export
- Camera setup creation for cam-anim (searchable with F3)
- UV Animator creation for uv-anim (searchable with F3, to use in material->Shift+A->Group)
