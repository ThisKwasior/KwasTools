# KwasTools
Library (`kwaslib`) and tools for video game modding.

`kwaslib` is a C library inspired by [Gnulib](https://www.gnu.org/software/gnulib/), in a way of being easy to incorporate into your own projects - entire library can be compiled as-is without needing to set any environmental variables/defines or external dependencies.

## Requirements
Must-have:
* **GCC/[MinGW-w64](https://github.com/niXman/mingw-builds-binaries)** (or [MSYS2](https://www.msys2.org/))
* **[CMake](https://cmake.org/)** (but it's possible to compile any program with a one-liner)

Optional:
* **Git** (project doesn't use any external dependencies, downloading a zip is fine)

## Building
I write mostly on Windows and compile with GCC from MinGW-w64.<br>
Tested on Windows (x86/x64 GCC/Clang) and Linux (ARM/x86/x64 GCC/Clang).<br>
**This project does not support Visual Studio compilers.**

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
| Program      | Description                      | Supported formats                                                                                                  |
|--------------|----------------------------------|--------------------------------------------------------------------------------------------------------------------|
| he_anim_tool | Hedgehog Engine *-anim converter | Reading/writing:<br>- uv-anim<br>- cam-anim<br>- vis-anim<br>- morph-anim<br>- pt-anim<br>- mat-anim<br>- lit-anim |

### CRIWARE
| Program          | Description                                                                                                                                                                                  | Supported formats                                                                     |
|------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------|
| cri_utf_tool     | CRI UTF parser to and from XML.<br>Will parse structures from VLDATA (UTF, AWB and ACB commands).                                                                                            | Reading/writing:<br>- ACB<br>- ACF<br>- AAX<br>- Any `@UTF` table i forgot to mention |
| cri_afs_tool     | CRI AFS packer/unpacker to and from XML.<br>If metadata section is not present, it will detect and add extensions to known file formats (AFS, AHX, ADX)                                      | Reading/writing:<br>- AFS                                                             |
| cri_awb_tool     | CRI AWB packer/unpacker to and from XML.<br>Will detect and add extensions to known file formats (HCA, ADX, BCWAV)                                                                           | Reading/writing:<br>- AWB                                                             |
| CRI Scramble Key | Python script and HTML+JS page to compute ACB key for encoding HCA/ADX files.<br>Found in `scripts` directory.<br>Also [hosted on my site](https://thiskwasior.ct8.pl/cri_scramble_key.htm). |                                                                                       |

### NW4R
| Program             | Description                                                         | Supported formats                                           |
|---------------------|---------------------------------------------------------------------|-------------------------------------------------------------|
| nw4r_misc_to_he_xml | Wii NW4R converter to Hedgehog Engine XML supported by he_anim_tool | Reading/writing:<br>- SCN0 -> cam-anim<br>- SRT0 -> uv-anim |

### Platinum Games
| Program           | Description                                                                       | Supported formats                           |
|-------------------|-----------------------------------------------------------------------------------|---------------------------------------------|
| platinum_dat_tool | Unpacker/packer with experimental support for Big Endian archives (X360/PS3/WiiU) | Reading/writing:<br>- DAT<br>- DTT<br>- EFF |
| platinum_wtb_tool | Unpacker/packer with WIP X360/PS3 texture conversion to PNG                       | Reading/writing:<br>- WTB<br>- WTA+WTP      |

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
* [VGAudio](https://github.com/Thealexbarney/VGAudio) - HCA format specifics.
* [Xenia](https://github.com/xenia-project/xenia) - Untiling code for Xbox 360 textures. Copyright (c) 2015, Ben Vanik. All rights reserved.
* [RPCS3](https://github.com/RPCS3/rpcs3) - Texture swizzling code for GTF textures. Licensed under GNU GPL-2.0 license.
