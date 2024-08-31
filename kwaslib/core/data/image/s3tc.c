#include "s3tc.h"

/*
	Converts 16-bit RGB565 to 24-bit RGB values.
	`out` is expected to be pre-allicated and able to hold 3 bytes.
*/
void s3tc_rgb565_to_rgb(const uint16_t rgb565, uint8_t* out)
{
	out[0] = (rgb565>>11)<<3;
	out[1] = (rgb565>>5)<<2;
	out[2] = (rgb565)<<3;
}

/*
	Converts 24-bit RGB to 16-bit RGB565 value.
	Returns converted RGB565.
*/
const uint16_t s3tc_rgb_to_rgb565(const uint8_t* rgb)
{
	const uint8_t r = (rgb[0]>>3)&0x1F;
	const uint8_t g = (rgb[1]>>2)&0x3F;
	const uint8_t b = (rgb[2]>>3)&0x1F;
	return ((r<<11) | (g<<5) | b);
}

/*
	Decodes a DXT1 block of data (8 bytes) to 16 RGB values (48 bytes).
	`out` is expected to be pre-allocated and able to hold 48 bytes of data.
*/
void s3tc_decode_dxt1_block(const uint8_t* block, uint8_t* out)
{
	const uint16_t c0 = *(const uint16_t*)&block[0];
	const uint16_t c1 = *(const uint16_t*)&block[2];
	const uint64_t clut = (*(const uint64_t*)block);
	uint8_t c[4][3] = {0};
	
	s3tc_rgb565_to_rgb(c0, &c[0][0]);
	s3tc_rgb565_to_rgb(c1, &c[1][0]);
	
	if(c0 > c1)
	{
		c[2][0] = ((uint32_t)(2*c[0][0]+c[1][0]))/3;
		c[2][1] = ((uint32_t)(2*c[0][1]+c[1][1]))/3;
		c[2][2] = ((uint32_t)(2*c[0][2]+c[1][2]))/3;
		c[3][0] = ((uint32_t)(c[0][0]+c[1][0]*2))/3;
		c[3][1] = ((uint32_t)(c[0][1]+c[1][1]*2))/3;
		c[3][2] = ((uint32_t)(c[0][2]+c[1][2]*2))/3;
	}
	else
	{
		c[2][0] = ((uint32_t)(c[0][0]+c[1][0]))/2;
		c[2][1] = ((uint32_t)(c[0][1]+c[1][1]))/2;
		c[2][2] = ((uint32_t)(c[0][2]+c[1][2]))/2;
	}

	uint8_t out_it = 0;
	for(uint8_t i = 0; i != 16; ++i)
	{
		const uint8_t index = (clut>>(32+(i*2)))&0x03;
		out[out_it++] = c[index][0];
		out[out_it++] = c[index][1];
		out[out_it++] = c[index][2];
	}
}

/*
	Decodes a DXT3 block of data (16 bytes) to 16 RGBA values (64 bytes).
	`out` is expected to be pre-allocated and able to hold 64 bytes of data.
*/
void s3tc_decode_dxt3_block(const uint8_t* block, uint8_t* out)
{
	uint8_t rgb[48] = {0};
	uint8_t alpha[16] = {0};
	
	s3tc_decode_dxt3_color_chunk(&block[8], &rgb[0]);
	s3tc_decode_dxt3_alpha_chunk(&block[0], &alpha[0]);
	
	for(uint8_t i = 0; i != 16; ++i)
	{
		out[i*4]	 = rgb[i*3];
		out[i*4 + 1] = rgb[i*3 + 1]; 
		out[i*4 + 2] = rgb[i*3 + 2];
		out[i*4 + 3] = alpha[i];
	}
}

/*
	Decodes a DXT5 block of data (16 bytes) to 16 RGBA values (64 bytes).
	`out` is expected to be pre-allocated and able to hold 64 bytes of data.
*/
void s3tc_decode_dxt5_block(const uint8_t* block, uint8_t* out)
{
	uint8_t rgb[48] = {0};
	uint8_t alpha[16] = {0};
	
	s3tc_decode_dxt3_color_chunk(&block[8], &rgb[0]);
	s3tc_decode_dxt5_alpha_chunk(&block[0], &alpha[0]);
	
	for(uint8_t i = 0; i != 16; ++i)
	{
		out[i*4]	 = rgb[i*3];
		out[i*4 + 1] = rgb[i*3 + 1]; 
		out[i*4 + 2] = rgb[i*3 + 2];
		out[i*4 + 3] = alpha[i];
	}
}

/*
	Decodes a DXT3/DXT5 color chunk (8 bytes) to 16 RGB values (48 bytes).
	`out` is expected to be pre-allocated and able to hold 48 bytes of data.
*/
void s3tc_decode_dxt3_color_chunk(const uint8_t* chunk, uint8_t* out)
{
	const uint16_t c0 = *(const uint16_t*)&chunk[0];
	const uint16_t c1 = *(const uint16_t*)&chunk[2];
	const uint64_t clut = (*(const uint64_t*)chunk);
	uint8_t c[4][3] = {0};
	
	s3tc_rgb565_to_rgb(c0, &c[0][0]);
	s3tc_rgb565_to_rgb(c1, &c[1][0]);
	
	c[2][0] = ((uint32_t)(2*c[0][0]+c[1][0]))/3;
	c[2][1] = ((uint32_t)(2*c[0][1]+c[1][1]))/3;
	c[2][2] = ((uint32_t)(2*c[0][2]+c[1][2]))/3;
	c[3][0] = ((uint32_t)(c[0][0]+c[1][0]*2))/3;
	c[3][1] = ((uint32_t)(c[0][1]+c[1][1]*2))/3;
	c[3][2] = ((uint32_t)(c[0][2]+c[1][2]*2))/3;
	
	uint8_t out_it = 0;
	for(uint8_t i = 0; i != 16; ++i)
	{
		const uint8_t index = (clut>>(32+(i*2)))&0x03;
		out[out_it++] = c[index][0];
		out[out_it++] = c[index][1];
		out[out_it++] = c[index][2];
	}
}

/*
	Decodes a DXT3 alpha chunk (8 bytes) to 16 alpha values (16 bytes).
	`out` is expected to be pre-allocated and able to hold 16 bytes of data.
*/
void s3tc_decode_dxt3_alpha_chunk(const uint8_t* chunk, uint8_t* out)
{
	const uint64_t alut = *(uint64_t*)chunk;

	for(uint8_t i = 0; i != 16; ++i)
	{
		const uint8_t value = ((alut>>(i*4))&0x0F)<<4;
		out[i] = value;
	}
}

/*
	Decodes a DXT5 alpha chunk (8 bytes) to 16 alpha values (16 bytes).
	`out` is expected to be pre-allocated and be able to hold 16 bytes of data.
*/
void s3tc_decode_dxt5_alpha_chunk(const uint8_t* chunk, uint8_t* out)
{
	uint8_t a[8] = {chunk[0], chunk[1], 0, 0, 0, 0, 0, 0};
	const uint64_t alut = (*(const uint64_t*)chunk);
	
	if(a[0] > a[1])
	{
		a[2] = ((uint32_t)(6*a[0] + a[1]))/7;
		a[3] = ((uint32_t)(5*a[0] + 2*a[1]))/7;
		a[4] = ((uint32_t)(4*a[0] + 3*a[1]))/7;
		a[5] = ((uint32_t)(3*a[0] + 4*a[1]))/7;
		a[6] = ((uint32_t)(2*a[0] + 5*a[1]))/7;
		a[7] = ((uint32_t)(1*a[0] + 6*a[1]))/7;
	}
	else
	{
		a[2] = ((uint32_t)(4*a[0] + a[1]))/5;
		a[3] = ((uint32_t)(3*a[0] + 2*a[1]))/5;
		a[4] = ((uint32_t)(2*a[0] + 3*a[1]))/5;
		a[5] = ((uint32_t)(1*a[0] + 4*a[1]))/5;
		a[6] = 0;
		a[7] = 255;
	}

	for(uint8_t i = 0; i != 16; ++i)
	{
		const uint8_t index = (alut>>(16+(i*3)))&0x07;
		out[i] = a[index];
	}
}