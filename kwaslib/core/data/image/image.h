#pragma once

#include <stdint.h>

#include "image_pixel.h"

typedef struct
{
	uint32_t width;
	uint32_t height;
	PIXEL_FMT fmt;
	PIXEL_FMT_BPP bpp;
	PIXEL* data;
} IMAGE;

/* Functions */

IMAGE* img_create_blank_image(const uint32_t width, const uint32_t height,
							  const PIXEL_FMT fmt);

IMAGE* img_load_from_data(const uint32_t width, const uint32_t height,
						  uint8_t* data, const PIXEL_FMT fmt);
						  
const PIXEL img_get_pixel(const IMAGE* img, const uint64_t x, const uint64_t y);
void img_set_pixel(const IMAGE* img, const uint64_t x, const uint64_t y, const PIXEL pixel);

IMAGE* img_crop_image(const IMAGE* img,
					  const int64_t x, const int64_t y,
					  const uint64_t w, const uint64_t h);
					  
void img_draw_on_image(const IMAGE* surface, const IMAGE* img,
					   const int64_t x, const int64_t y);

uint8_t* img_to_raw_data(const IMAGE* img, uint64_t* size);

IMAGE* img_alloc_image();
void img_free_image(IMAGE* image);