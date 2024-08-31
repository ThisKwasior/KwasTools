#pragma once

#include <stdint.h>
#include <stdio.h>

/*
	https://www.khronos.org/opengl/wiki/S3_Texture_Compression
	https://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats
*/

/*
	Converts 16-bit RGB565 to 24-bit RGB values.
	`out` is expected to be pre-allicated and able to hold 3 bytes.
*/
void s3tc_rgb565_to_rgb(const uint16_t rgb565, uint8_t* out);

/*
	Converts 24-bit RGB to 16-bit RGB565 value.
	Returns converted RGB565.
*/
const uint16_t s3tc_rgb_to_rgb565(const uint8_t* rgb);

/*
	Decodes a DXT1 block of data (8 bytes) to 16 RGB values (48 bytes).
	`out` is expected to be pre-allocated and able to hold 48 bytes of data.
*/
void s3tc_decode_dxt1_block(const uint8_t* block, uint8_t* out);

/*
	Decodes a DXT3 block of data (16 bytes) to 16 RGBA values (64 bytes).
	`out` is expected to be pre-allocated and able to hold 64 bytes of data.
*/
void s3tc_decode_dxt3_block(const uint8_t* block, uint8_t* out);

/*
	Decodes a DXT5 block of data (16 bytes) to 16 RGBA values (64 bytes).
	`out` is expected to be pre-allocated and able to hold 64 bytes of data.
*/
void s3tc_decode_dxt5_block(const uint8_t* block, uint8_t* out);

/*
	Decodes a DXT3/DXT5 color chunk (8 bytes) to 16 RGB values (48 bytes).
	`out` is expected to be pre-allocated and able to hold 48 bytes of data.
*/
void s3tc_decode_dxt3_color_chunk(const uint8_t* chunk, uint8_t* out);

/*
	Decodes a DXT3 alpha chunk (8 bytes) to 16 alpha values (16 bytes).
	`out` is expected to be pre-allocated and able to hold 16 bytes of data.
*/
void s3tc_decode_dxt3_alpha_chunk(const uint8_t* chunk, uint8_t* out);

/*
	Decodes a DXT5 alpha chunk (8 bytes) to 16 alpha values (16 bytes).
	`out` is expected to be pre-allocated and be able to hold 16 bytes of data.
*/
void s3tc_decode_dxt5_alpha_chunk(const uint8_t* chunk, uint8_t* out);