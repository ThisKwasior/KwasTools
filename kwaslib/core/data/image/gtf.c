#include "gtf.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kwaslib/core/data/image/s3tc.h>

GTF_FILE gtf_read_file(FU_FILE* gtf)
{
	GTF_FILE header = {0};
	
	header.version = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	header.texture_size = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	header.texture_count = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	
	header.info.index = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	header.info.offset = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	header.info.size = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	
	header.info.tex.pf = fu_read_u8(gtf, NULL);
	header.info.tex.mipmaps = fu_read_u8(gtf, NULL);
	header.info.tex.dimension = fu_read_u8(gtf, NULL);
	header.info.tex.unk_1 = fu_read_u8(gtf, NULL);
	header.info.tex.remaps = fu_read_u32(gtf, NULL, FU_BIG_ENDIAN);
	header.info.tex.width = fu_read_u16(gtf, NULL, FU_BIG_ENDIAN);
	header.info.tex.height = fu_read_u16(gtf, NULL, FU_BIG_ENDIAN);
	header.info.tex.depth = fu_read_u16(gtf, NULL, FU_BIG_ENDIAN);
	header.info.tex.unk_2 = fu_read_u16(gtf, NULL, FU_BIG_ENDIAN);
	fu_read_data(gtf, (uint8_t*)&header.info.tex.pad[0], 88, NULL);
	
	fu_seek(gtf, header.info.offset, FU_SEEK_SET); 
	header.data = (uint8_t*)calloc(1, header.info.size);
	fu_read_data(gtf, header.data, header.info.size, NULL);
	
	return header;
}

IMAGE* gtf_to_image(const GTF_FILE* gtf)
{
	uint8_t fmt = FMT_RGBA;
	const uint8_t gtf_fmt = gtf->info.tex.pf;
	if(gtf_fmt == GTF_TEX_GRAY8) fmt = FMT_GRAY;
	if(gtf_fmt == GTF_TEX_A8R8G8B8) fmt = FMT_ARGB;
	if(gtf_fmt == GTF_TEX_DXT1) fmt = FMT_RGB;
	
	const uint16_t width = gtf->info.tex.width;
	const uint16_t height = gtf->info.tex.height;
	printf("W:%u H:%u | PF: %02x %u\n", width, height, gtf_fmt, fmt);
	
	if(gtf_fmt == GTF_TEX_GRAY8)
	{
		uint8_t* pixel_data = calloc(1, width*height);
		gtf_swizzle_to_linear_gray(&gtf->data[0], pixel_data, width, height);
		IMAGE* img_gray = img_load_from_data(width, height, pixel_data, fmt);
		free(pixel_data);
		return img_gray;
	}
	
	if(gtf_fmt == GTF_TEX_A8R8G8B8)
	{
		uint8_t* pixel_data = calloc(1, width*height*4);
		gtf_swizzle_to_linear_argb(&gtf->data[0], pixel_data, width, height);
		IMAGE* img_argb = img_load_from_data(width, height, pixel_data, fmt);
		free(pixel_data);
		return img_argb;
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
			switch(gtf_fmt)
			{
				case GTF_TEX_DXT1:
					s3tc_decode_dxt1_block(&gtf->data[data_it], &pixel_buf[0]);
					data_it += 8;
					
					for(uint8_t i = 0; i != 16; ++i)
					{
						block->data[i].rgb.r = pixel_buf[i*3];
						block->data[i].rgb.g = pixel_buf[i*3+1];
						block->data[i].rgb.b = pixel_buf[i*3+2];
					}
					break;
				case GTF_TEX_DXT3:
					s3tc_decode_dxt3_block(&gtf->data[data_it], &pixel_buf[0]);
					data_it += 16;
					
					for(uint8_t i = 0; i != 16; ++i)
					{
						block->data[i].rgba.r = pixel_buf[i*4];
						block->data[i].rgba.g = pixel_buf[i*4+1];
						block->data[i].rgba.b = pixel_buf[i*4+2];
						block->data[i].rgba.a = pixel_buf[i*4+3];
					}
					break;
				case GTF_TEX_DXT5:
					s3tc_decode_dxt5_block(&gtf->data[data_it], &pixel_buf[0]);
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
					printf("Unknown gtf pixel format: 0x%02x\n", gtf_fmt);
					img_free_image(block);
					img_free_image(img);
					return NULL;
			}
			
			img_draw_on_image(img, block, x, y);
		}
	}

	img_free_image(block);
	return img;
}

void gtf_free(GTF_FILE* gtf)
{
	free(gtf->data);
	memset(gtf, 0, sizeof(GTF_FILE));
}

const double gtf_log2(const double v)
{
	return log(v)/log(2.f);
}

/*
	https://en.wikipedia.org/wiki/Moser%E2%80%93de_Bruijn_sequence
*/
const uint64_t gtf_num_to_distpow4(const uint32_t value)
{
	uint64_t dp4 = 0;
	
	uint8_t dp_it = 0;
	for(uint8_t i = 0; i != 32; ++i)
	{
		const uint32_t bit = (value>>i)&1;
		dp4 |= (bit<<dp_it);
		dp_it += 2;
	}
	
	return dp4;
}

void gtf_swizzle_to_linear_gray(const uint8_t* input, uint8_t* output, const uint16_t width, const uint16_t height)
{
	const uint32_t l2w = (uint32_t)gtf_log2(width);
	const uint32_t l2h = (uint32_t)gtf_log2(height);
	
	uint32_t x_mask = 0x55555555;
	uint32_t y_mask = 0xAAAAAAAA;
	
	uint32_t limit_mask = l2w < l2h ? l2w : l2h;
	limit_mask = 1 << (limit_mask << 1);

	x_mask = (x_mask | ~(limit_mask - 1));
	y_mask = (y_mask & (limit_mask - 1));
	
	uint32_t offs_y = 0;
	uint32_t offs_x = 0;
	uint32_t offs_x0 = 0;
	uint32_t y_incr = limit_mask;
	
	const uint32_t pitch_in_blocks = width;
	uint32_t row_offset = 0;

	for(int y = 0; y < height; ++y, row_offset += pitch_in_blocks)
	{
		const uint8_t* src = input + offs_y;
		uint8_t* dst = output + row_offset;
		offs_x = offs_x0;

		for(int x = 0; x < width; ++x)
		{
			dst[x] = src[offs_x];
			offs_x = (offs_x - x_mask) & x_mask;
		}

		offs_y = (offs_y - y_mask) & y_mask;

		if(offs_y == 0)
		{
			offs_x0 += y_incr;
		}
	}
}

void gtf_swizzle_to_linear_argb(const uint8_t* input, uint8_t* output, const uint16_t width, const uint16_t height)
{
	const uint32_t l2w = (uint32_t)gtf_log2(width);
	const uint32_t l2h = (uint32_t)gtf_log2(height);
	
	uint32_t x_mask = 0x55555555;
	uint32_t y_mask = 0xAAAAAAAA;
	
	uint32_t limit_mask = l2w < l2h ? l2w : l2h;
	limit_mask = 1 << (limit_mask << 1);

	x_mask = (x_mask | ~(limit_mask - 1));
	y_mask = (y_mask & (limit_mask - 1));
	
	uint32_t offs_y = 0;
	uint32_t offs_x = 0;
	uint32_t offs_x0 = 0;
	uint32_t y_incr = limit_mask;
	
	const uint32_t pitch_in_blocks = width;
	uint32_t row_offset = 0;

	for(int y = 0; y < height; ++y, row_offset += pitch_in_blocks)
	{
		const uint32_t* src = (uint32_t*)input + offs_y;
		uint32_t* dst = (uint32_t*)output + row_offset;
		offs_x = offs_x0;

		for(int x = 0; x < width; ++x)
		{
			dst[x] = src[offs_x];
			offs_x = (offs_x - x_mask) & x_mask;
		}

		offs_y = (offs_y - y_mask) & y_mask;

		if(offs_y == 0)
		{
			offs_x0 += y_incr;
		}
	}
}