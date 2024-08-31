#pragma once

#include <stdint.h>

typedef enum PIXEL_FORMAT
{
	FMT_RGB		= 1,
	FMT_RGBA	= 2,
	FMT_GRAY	= 3,
	FMT_RGB565	= 4,
	FMT_ARGB	= 5,
	PIXEL_FORMAT_COUNT
} PIXEL_FMT;

typedef enum PIXEL_FORMAT_BPP
{
	FMT_RGB_BPP		= 3,
	FMT_RGBA_BPP	= 4,
	FMT_GRAY_BPP	= 1,
	FMT_RGB565_BPP	= 2,
	PIXEL_FORMAT_BPP_COUNT
} PIXEL_FMT_BPP;

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} PIXEL_RGB;

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} PIXEL_RGBA;

typedef struct
{
	uint8_t g;
} PIXEL_GRAY;

typedef struct
{
	uint8_t r : 5;
	uint8_t g : 6;
	uint8_t b : 5;
} PIXEL_RGB565;

typedef union
{
	PIXEL_RGB rgb;
	PIXEL_RGBA rgba;
	PIXEL_GRAY gray;
	PIXEL_RGB565 rgb565;
	
	uint64_t data;
} PIXEL;

/* Converters between pixel formats*/

inline static const PIXEL pix_rgb565_to_rgb(PIXEL pixel)
{
	PIXEL px = {.data = 0};
	px.rgb565.r = (pixel.rgb.r<<3);
	px.rgb565.g = (pixel.rgb.g<<2);
	px.rgb565.b = (pixel.rgb.b<<3);
	return px;
}

inline static const PIXEL pix_rgb_to_rgb565(PIXEL pixel)
{
	PIXEL px = {.data = 0};
	px.rgb565.r = (pixel.rgb.r>>3);
	px.rgb565.g = (pixel.rgb.g>>2);
	px.rgb565.b = (pixel.rgb.b>>3);
	return px;
}

inline static const PIXEL pix_from_raw_rgb565(const uint16_t pixel)
{
	PIXEL px = {.data = 0};
	px.rgb565.r = pixel>>11;
	px.rgb565.g = pixel>>5;
	px.rgb565.b = pixel;
	return px;
}

inline static const uint16_t pix_to_raw_rgb565(const PIXEL pixel)
{
	const uint16_t rgb565 = ((pixel.rgb565.r)<<11)
							| ((pixel.rgb565.g)<<5)
							| (pixel.rgb565.b);
	return rgb565;
}
