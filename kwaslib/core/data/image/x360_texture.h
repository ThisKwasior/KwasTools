#pragma once

#include <stdint.h>

#include <kwaslib/core/data/image/image.h>

#include "x360_enum.h"

/*
	https://burnout.wiki/wiki/Texture/Xbox_360
	https://github.com/leeao/Noesis-Plugins/blob/master/Textures/inc_xbox360_untile.py
*/

typedef struct
{
	uint32_t type : 4;
	uint32_t flags : 28;
} D3DResource_common;

typedef struct
{
	D3DResource_common common;
	uint32_t reference_count;
	uint32_t fence;
	uint32_t read_fence;
	uint32_t identifier;
	uint32_t base_flush;
} D3DResource;

typedef union
{
    struct GPUTEXTURESIZE_1D
    {
        uint32_t width : 24;
    } one_dee;
    
    struct GPUTEXTURESIZE_2D
    {
        uint32_t width : 13;
        uint32_t height : 13;
    } two_dee;
    
    struct GPUTEXTURESIZE_3D
    {
        uint32_t width : 11;
        uint32_t height : 11;
        uint32_t depth : 10;
    } three_dee;
    
    struct GPUTEXTURESIZE_STACK
    {
        uint32_t width : 13;
        uint32_t height : 13;
        uint32_t depth : 6;
    } stack;
} GPUTEXTURESIZE;

typedef union
{
	struct
	{
		/* 0 */
		uint32_t type : 2; /* GPUCONSTANTTYPE */
		uint32_t sign_x : 2;
		uint32_t sign_y : 2;
		uint32_t sign_z : 2;
		uint32_t sign_w : 2;
		uint32_t clamp_x : 3;
		uint32_t clamp_y : 3;
		uint32_t clamp_z : 3;
		uint32_t multi_sample : 2;
		uint32_t pad : 1;
		uint32_t pitch : 9;
		uint32_t tiled : 1;

		/* 1 */
		uint32_t data_format : 6; /* GPUTEXTUREFORMAT */
		uint32_t endian : 2;
		uint32_t request_size : 2;
		uint32_t stacked : 1;
		uint32_t clamp_policy : 1;
		uint32_t base_address : 20;

		/* 2 */
		GPUTEXTURESIZE size;

		/* 3 */
		uint32_t num_format : 1;
		uint32_t swizzle_x : 3;
		uint32_t swizzle_y : 3;
		uint32_t swizzle_z : 3;
		uint32_t swizzle_w : 3;
		int32_t exp_adjust : 6;
		uint32_t mag_filter : 2;
		uint32_t min_filter : 2;
		uint32_t mip_filter : 2;
		uint32_t aniso_filter : 3;
		uint32_t pad2 : 3;
		uint32_t border_size : 1;

		/* 4 */
		uint32_t vol_mag_filter : 1;
		uint32_t vol_min_gilter : 1;
		uint32_t min_mip_level : 4;
		uint32_t max_mip_level : 4;
		uint32_t mag_aniso_walk : 1;
		uint32_t min_aniso_walk : 1;
		int32_t lod_bias : 10;
		int32_t grad_exp_adjust_h : 5;
		int32_t grad_exp_adjust_v : 5;

		/* 5 */
		uint32_t border_color : 2;
		uint32_t force_bcw_to_max : 1;
		uint32_t tri_clamp : 2;
		int32_t aniso_bias : 4;
		uint32_t dimension : 2;
		uint32_t packed_mips : 1;
		uint32_t mip_address : 20;
	};
	
	uint32_t data[6];
} GPUTEXTURE_FETCH_CONSTANT;

typedef struct
{
	D3DResource base;
	uint32_t mip_flush;
	GPUTEXTURE_FETCH_CONSTANT format;
} D3DBaseTexture;

IMAGE* x360_texture_to_image(const D3DBaseTexture xpr, uint8_t* data, const uint64_t data_size);

uint32_t TiledOffset2DRow(uint32_t y, uint32_t width, uint32_t log2_bpp);
uint32_t TiledOffset2DColumn(uint32_t x, uint32_t y, uint32_t log2_bpp, uint32_t base_offset);
void Untile(const uint8_t* input_buffer, uint8_t* output_buffer,
			const uint16_t width, const uint16_t height,
			const uint32_t input_bytes_per_block, const uint32_t output_bytes_per_block,
			const uint32_t pitch);
			
void x360_texture_untile(const uint32_t input_size, const uint8_t* input_buffer, uint8_t* output_buffer, const uint16_t width, const uint16_t height, GPUTEXTUREFORMAT format);

uint32_t x360_texture_address_2d_tiled_x(uint32_t offset, uint32_t width, uint32_t texel_pitch);
uint32_t x360_texture_address_2d_tiled_y(uint32_t offset, uint32_t width, uint32_t texel_pitch);