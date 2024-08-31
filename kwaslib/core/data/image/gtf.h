#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/image/image.h>

/*
	https://psdevwiki.com/ps3/Multimedia_Formats_and_Tools#GTF
	https://buildbot.libretro.com/doxygen/a00386_source.html
	
	http://homebrew.pixelbath.com/wiki/PSP_texture_swizzling
	https://github.com/RPCS3/rpcs3/blob/09ea858dbf2eabf26ce0ab1a275c6350a54626b7/rpcs3/Emu/RSX/rsx_utils.h#L371
	
	https://en.wikipedia.org/wiki/Z-order_curve
	https://demonstrations.wolfram.com/ComparingXYCurvesAndZOrderCurvesForTextureCoding/
*/

typedef enum
{
	GTF_TEX_GRAY8 = 0x81,
	GTF_TEX_A8R8G8B8 = 0x85,
	GTF_TEX_DXT1 = 0x86,
	GTF_TEX_DXT3 = 0x87,
	GTF_TEX_DXT5 = 0x88,
} GTF_PIXELFORMAT;

typedef struct
{
	GTF_PIXELFORMAT pf;
	uint8_t mipmaps;
	uint8_t dimension;
	uint8_t unk_1;
	uint32_t remaps;
	uint16_t width;
	uint16_t height;
	uint16_t depth;
	uint16_t unk_2;
	uint32_t pad[22];
} GTF_TEXTURE;

typedef struct
{         
	uint32_t index;
	uint32_t offset;		/* Offset to tex data from the start of the file */
	uint32_t size;
	GTF_TEXTURE tex;
} GTF_TEX_INFO;

typedef struct
{
	uint32_t version;
	uint32_t texture_size;
	uint32_t texture_count;
	GTF_TEX_INFO info;
	
	uint8_t* data;
} GTF_FILE;

GTF_FILE gtf_read_file(FU_FILE* gtf);
IMAGE* gtf_to_image(const GTF_FILE* gtf);

void gtf_free(GTF_FILE* gtf);

/*
	https://en.wikipedia.org/wiki/Moser%E2%80%93de_Bruijn_sequence
*/
const uint64_t gtf_num_to_distpow4(const uint32_t value);

const double gtf_log2(const double v);

/*
	https://github.com/rygorous/rygblog-src/blob/master/posts/texture-tiling-and-swizzling.md
	https://github.com/RPCS3/rpcs3/blob/master/rpcs3/Emu/RSX/rsx_utils.h#L371
*/
void gtf_swizzle_to_linear_gray(const uint8_t* input, uint8_t* output, const uint16_t width, const uint16_t height);
void gtf_swizzle_to_linear_argb(const uint8_t* input, uint8_t* output, const uint16_t width, const uint16_t height);