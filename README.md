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
* `he_anim_tool` - Hedgehog Engine anim converter, like uv-anim and cam-anim

### CRIWARE
**Everything here will be moved to [CryTools](https://github.com/ThisKwasior/CryTools) repo sometime in the future. I'm keeping it all in KwasTools until I rewrite everything in there with kwaslib.**
* `cri_utf_tool` - CRI UTF parser to and from XML. Will parse structures from VLDATA (currently only UTF and AWB).
* `cri_awb_tool` - CRI AWB packer/unpacker. Supports creating AWBs from folder of files, custom XML config and unpacks AWBs to folder and XML config.

### Platinum Games
* `platinum_dat_tool` - DAT unpacker/packer with experimental support for Xbox 360
* `platinum_wta_wtp_tool` - WTA/WTP unpacker/packer with WIP Xbox 360 support
* `platinum_wtb_tool` - WTB unpacker/packer with WIP Xbox 360 support
* `platinum_misc_dat2dds2dat` - WIP WTA/WTP/WTB unpacker/packer directly from DAT/DTT

## Addons
### io_kwastools
WIP addon for Blender 3.0+. 
Currently allows for importing uv-anim and cam-anim in XML format and exporting cam-anim to XML format.
