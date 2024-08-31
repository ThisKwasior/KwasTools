#include "x360_texture.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <kwaslib/core/data/image/s3tc.h>
#include <kwaslib/core/cpu/endianness.h>

IMAGE* x360_texture_to_image(const D3DBaseTexture xpr, uint8_t* data, const uint64_t data_size)
{
	if(data == NULL) return NULL;
	
	/* Check if it's a texture */
	if(xpr.format.type != GPUCONSTANTTYPE_TEXTURE) return NULL;
	
	/* Check if it's a texture 2D */
	if(xpr.format.dimension != GPUDIMENSION_2D) return NULL;
	
	const uint16_t width = xpr.format.size.two_dee.width + 1;
	const uint16_t height = xpr.format.size.two_dee.height + 1;
	
	uint8_t fmt = 0;
	const uint8_t xpr_fmt = xpr.format.data_format;
	uint8_t* data_ptr = data;

	switch(xpr_fmt)
	{
		case GPUTEXTUREFORMAT_8:		fmt = FMT_GRAY; break;
		case GPUTEXTUREFORMAT_8_8_8_8:	fmt = FMT_ARGB; break;
		case GPUTEXTUREFORMAT_DXT1:		fmt = FMT_RGB; break;
		case GPUTEXTUREFORMAT_DXT2_3:	fmt = FMT_RGBA; break;
		case GPUTEXTUREFORMAT_DXT4_5:	fmt = FMT_RGBA; break;
		default: 						fmt = FMT_RGBA;
	}
	
	printf("X360: W:%u|H:%u %u %u %u %u\n", width, height, xpr_fmt, fmt, xpr.format.tiled, xpr.format.endian);
	
	if(xpr_fmt == GPUTEXTUREFORMAT_8)
	{
		uint8_t* pixel_data = calloc(1, width*height);
		
		if(xpr.format.tiled) x360_texture_untile(data_size, data_ptr, pixel_data, width, height, xpr_fmt);
		else memcpy(pixel_data, data_ptr, width*height);
		
		IMAGE* img_gray = img_load_from_data(width, height, pixel_data, fmt);
		free(pixel_data);
		return img_gray;
	}
	
	if(xpr_fmt == GPUTEXTUREFORMAT_8_8_8_8)
	{
		uint8_t* pixel_data = calloc(1, width*height*4);
		
		if(xpr.format.tiled) x360_texture_untile(data_size, data_ptr, pixel_data, width, height, xpr_fmt);
		else memcpy(pixel_data, data_ptr, width*height*4);
		
		IMAGE* img_argb = img_load_from_data(width, height, pixel_data, fmt);
		free(pixel_data);
		return img_argb;
	}
	
	/* Convert data to correct endianess */
	switch(xpr.format.endian)
	{
		case GPUENDIAN_8IN16:
			uint16_t* data16 = (uint16_t*)data;
			for(uint64_t i = 0; i != data_size/2; ++i)
				data16[i] = ed_swap_endian_16(data16[i]);
			break;
	}
	
	if(xpr.format.tiled)
	{
		uint8_t* pixel_data = NULL;
		
		switch(xpr_fmt)
		{
			case GPUTEXTUREFORMAT_DXT1:
			case GPUTEXTUREFORMAT_DXT2_3:
			case GPUTEXTUREFORMAT_DXT4_5:
				pixel_data = calloc(1, data_size);
				x360_texture_untile(data_size, data_ptr, pixel_data, width, height, xpr_fmt);
				data_ptr = pixel_data;
				break;
			default: break;
		}
	}
	
	/*
		S3TC compressed textures
	*/
	
	IMAGE* img = img_create_blank_image(width, height, fmt);
	IMAGE* block = img_create_blank_image(4, 4, fmt);
	
	uint8_t pixel_buf[64] = {0};
	uint32_t data_it = 0;
	
	for(uint32_t y = 0; y != height; y+=4)
	{
		for(uint32_t x = 0; x != width; x+=4)
		{
			switch(xpr_fmt)
			{
				case GPUTEXTUREFORMAT_DXT1:
					s3tc_decode_dxt1_block(&data_ptr[data_it], &pixel_buf[0]);
					data_it += 8;
					
					for(uint8_t i = 0; i != 16; ++i)
					{
						block->data[i].rgb.r = pixel_buf[i*3];
						block->data[i].rgb.g = pixel_buf[i*3+1];
						block->data[i].rgb.b = pixel_buf[i*3+2];
					}
					break;
				case GPUTEXTUREFORMAT_DXT2_3:
					s3tc_decode_dxt3_block(&data_ptr[data_it], &pixel_buf[0]);
					data_it += 16;
					
					for(uint8_t i = 0; i != 16; ++i)
					{
						block->data[i].rgba.r = pixel_buf[i*4];
						block->data[i].rgba.g = pixel_buf[i*4+1];
						block->data[i].rgba.b = pixel_buf[i*4+2];
						block->data[i].rgba.a = pixel_buf[i*4+3];
					}
					break;
				case GPUTEXTUREFORMAT_DXT4_5:
					s3tc_decode_dxt5_block(&data_ptr[data_it], &pixel_buf[0]);
					data_it += 16;
					
					for(uint8_t i = 0; i != 16; ++i)
					{
						block->data[i].rgba.r = pixel_buf[i*4];
						block->data[i].rgba.g = pixel_buf[i*4+1];
						block->data[i].rgba.b = pixel_buf[i*4+2];
						block->data[i].rgba.a = pixel_buf[i*4+3];
					}
					break;
				default:
					printf("Unknown xpr pixel format: 0x%02x\n", xpr_fmt);
					img_free_image(block);
					img_free_image(img);
					return NULL;
			}
			
			img_draw_on_image(img, block, x, y);
		}
	}

	if(data != data_ptr) free(data_ptr);

	img_free_image(block);
	
	return img;
}

// https://github.com/BinomialLLC/crunch/blob/ea9b8d8c00c8329791256adafa8cf11e4e7942a2/inc/crn_decomp.h#L4108
uint32_t TiledOffset2DRow(uint32_t y, uint32_t width, uint32_t log2_bpp)
{
  uint32_t macro = ((y / 32) * (width / 32)) << (log2_bpp + 7);
  uint32_t micro = ((y & 6) << 2) << log2_bpp;
  return macro + ((micro & ~0xF) << 1) + (micro & 0xF) +
         ((y & 8) << (3 + log2_bpp)) + ((y & 1) << 4);
}

uint32_t TiledOffset2DColumn(uint32_t x, uint32_t y, uint32_t log2_bpp, uint32_t base_offset)
{
  uint32_t macro = (x / 32) << (log2_bpp + 7);
  uint32_t micro = (x & 7) << log2_bpp;
  uint32_t offset =
      base_offset + (macro + ((micro & ~0xF) << 1) + (micro & 0xF));
  return ((offset & ~0x1FF) << 3) + ((offset & 0x1C0) << 2) + (offset & 0x3F) +
         ((y & 16) << 7) + (((((y & 8) >> 2) + (x >> 3)) & 3) << 6);
}

void Untile(const uint8_t* input_buffer, uint8_t* output_buffer,
			const uint16_t width, const uint16_t height,
			const uint32_t input_bytes_per_block, const uint32_t output_bytes_per_block,
			const uint32_t pitch)
{
	uint32_t output_pitch = pitch * output_bytes_per_block;

	uint32_t offset_x = 0;
	uint32_t offset_y = 0;

	// Bytes per pixel
	uint32_t log2_bpp = (input_bytes_per_block / 4) +
				      ((input_bytes_per_block / 2) >> (input_bytes_per_block / 4));

	// Offset to the current row, in bytes.
	uint32_t output_row_offset = 0;
	for (uint32_t y = 0; y < height; y++)
	{
		uint32_t input_row_offset = TiledOffset2DRow(offset_y + y,
													 pitch,
													 log2_bpp);

		// Go block-by-block on this row.
		uint32_t output_offset = output_row_offset;

		for (uint32_t x = 0; x < width; x++)
		{
			uint32_t input_offset = TiledOffset2DColumn(offset_x + x, 
														offset_y + y, 
														log2_bpp,
														input_row_offset);

			input_offset >>= log2_bpp;

			memcpy(&output_buffer[output_offset], 
				   &input_buffer[input_offset * input_bytes_per_block],
				   output_bytes_per_block);

			output_offset += output_bytes_per_block;
		}

		output_row_offset += output_pitch;
	}
}

void x360_texture_untile(const uint32_t input_size, const uint8_t* input_buffer, uint8_t* output_buffer, const uint16_t width, const uint16_t height, GPUTEXTUREFORMAT format)
{
	uint32_t block_size = 0;
	uint32_t texel_pitch = 0;
	
	switch(format)
	{
		case GPUTEXTUREFORMAT_8:
			block_size = 1;
			texel_pitch = 1;
			break;
		case GPUTEXTUREFORMAT_8_8_8_8:
			block_size = 1;
			texel_pitch = 4;
			break;
		case GPUTEXTUREFORMAT_DXT1:
		case GPUTEXTUREFORMAT_CTX1:
			block_size = 4;
			texel_pitch = 8;
			break;
		case GPUTEXTUREFORMAT_DXT2_3:
		case GPUTEXTUREFORMAT_DXT4_5:
			block_size = 4;
			texel_pitch = 16;
			break;
		default: break;
	}
	
	uint32_t block_width = floor((width/(float)block_size));
	uint32_t block_height = floor((height/(float)block_size));
	
	for(uint32_t j = 0; j != block_height; ++j)
	{
		for(uint32_t i = 0; i != block_width; ++i)
		{
			uint32_t block_offset = j*block_width+i;

			uint32_t x = x360_texture_address_2d_tiled_x(block_offset, block_width, texel_pitch);
			uint32_t y = x360_texture_address_2d_tiled_y(block_offset, block_width, texel_pitch);
			
			uint32_t src_offset = j*block_width*texel_pitch + i*texel_pitch;
			uint32_t dst_offset = y*block_width*texel_pitch + x*texel_pitch;
			
			if(dst_offset < input_size)
				memcpy(&output_buffer[dst_offset], &input_buffer[src_offset], texel_pitch);
		}
	}
}

uint32_t x360_texture_address_2d_tiled_x(uint32_t offset, uint32_t width, uint32_t texel_pitch)
{
	uint32_t aligned_width = (width + 31) & ~31;

	uint32_t LogBpp = (texel_pitch >> 2) + ((texel_pitch >> 1) >> (texel_pitch >> 2));
	uint32_t OffsetB = offset << LogBpp;
	uint32_t OffsetT = ((OffsetB & ~4095) >> 3) + ((OffsetB & 1792) >> 2) + (OffsetB & 63);
	uint32_t OffsetM = OffsetT >> (7 + LogBpp);

	uint32_t MacroX = ((OffsetM % (aligned_width >> 5)) << 2);
	uint32_t Tile = ((((OffsetT >> (5 + LogBpp)) & 2) + (OffsetB >> 6)) & 3);
	uint32_t Macro = (MacroX + Tile) << 3;
	uint32_t Micro = ((((OffsetT >> 1) & ~15) + (OffsetT & 15)) & ((texel_pitch << 3) - 1)) >> LogBpp;

	return (Macro + Micro);
}

uint32_t x360_texture_address_2d_tiled_y(uint32_t offset, uint32_t width, uint32_t texel_pitch)
{
    uint32_t aligned_width = (width + 31) & ~31;

    uint32_t LogBpp = (texel_pitch >> 2) + ((texel_pitch >> 1) >> (texel_pitch >> 2));
    uint32_t OffsetB = offset << LogBpp;
    uint32_t OffsetT = ((OffsetB & ~4095) >> 3) + ((OffsetB & 1792) >> 2) + (OffsetB & 63);
    uint32_t OffsetM = OffsetT >> (7 + LogBpp);

    uint32_t MacroY = ((uint32_t)floor(OffsetM / (aligned_width >> 5)) << 2);
    uint32_t Tile = ((OffsetT >> (6 + LogBpp)) & 1) + (((OffsetB & 2048) >> 10));
    uint32_t Macro = (MacroY + Tile) << 3;
    uint32_t Micro = ((((OffsetT & (((texel_pitch << 6) - 1) & ~31)) + ((OffsetT & 15) << 1)) >> (3 + LogBpp)) & ~1);

    return (Macro + Micro + ((OffsetT & 16) >> 4));
}