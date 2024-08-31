#include "image.h"

#include <stdlib.h>
#include <string.h>

IMAGE* img_create_blank_image(const uint32_t width, const uint32_t height, const PIXEL_FMT fmt)
{
	IMAGE* img = img_alloc_image();
	
	if(img == NULL) return NULL;
	
	img->data = (PIXEL*)calloc(width*height, sizeof(PIXEL));
	
	if(img->data == NULL) 
	{
		free(img);
		return NULL;
	}
	
	img->width = width;
	img->height = height;
	img->fmt = fmt;
	
	switch(img->fmt)
	{
		case FMT_RGB:		img->bpp = FMT_RGB_BPP; break;
		case FMT_RGBA:		img->bpp = FMT_RGBA_BPP; break;
		case FMT_GRAY:		img->bpp = FMT_GRAY_BPP; break;
		case FMT_RGB565:	img->bpp = FMT_RGB565_BPP; break;
		case FMT_ARGB:		img->bpp = FMT_RGBA_BPP; break;
		default: 			img->bpp = 0;
	}

	return img;
}
#include <stdio.h>
IMAGE* img_load_from_data(const uint32_t width, const uint32_t height,
						  uint8_t* data, const PIXEL_FMT fmt)
{
	IMAGE* img = img_create_blank_image(width, height, fmt);
	
	if(img == NULL) return NULL;
	
	for(uint64_t i = 0; i != width*height; ++i)
	{
		switch(fmt)
		{
			case FMT_RGB:
				img->data[i].rgb.r = data[0];
				img->data[i].rgb.g = data[1];
				img->data[i].rgb.b = data[2];
				break;
			case FMT_RGBA:
				img->data[i].rgba.r = data[0];
				img->data[i].rgba.g = data[1];
				img->data[i].rgba.b = data[2];
				img->data[i].rgba.a = data[3];
				break;
			case FMT_GRAY:
				img->data[i].gray.g = data[0];
				break;
			case FMT_ARGB:
				img->data[i].rgba.a = data[0];
				img->data[i].rgba.r = data[1];
				img->data[i].rgba.g = data[2];
				img->data[i].rgba.b = data[3];
				break;
			case FMT_RGB565:
				const uint16_t raw_px = *((uint16_t*)&data[i]);
				const PIXEL px = pix_from_raw_rgb565(raw_px);
				img->data[i] = px;
				break;
			default: break;
		}
		
		data += img->bpp;
	}

	if(fmt == FMT_ARGB)
	{
		img->fmt = FMT_RGBA;
	}

	return img;
}

const PIXEL img_get_pixel(const IMAGE* img, const uint64_t x, const uint64_t y)
{
	PIXEL pixel = {.data = 0};
	
	if(img->width <= x) return pixel;
	if(img->height <= y) return pixel;
	
	pixel = img->data[y*img->width+x];
	
	return pixel;
}

void img_set_pixel(const IMAGE* img, const uint64_t x, const uint64_t y, const PIXEL pixel)
{
	if(img->width <= x) return;
	if(img->height <= y) return;
	
	memcpy(&img->data[y*img->width+x], &pixel, sizeof(PIXEL));
}

IMAGE* img_crop_image(const IMAGE* img,
					  const int64_t x, const int64_t y,
					  const uint64_t w, const uint64_t h)
{
	IMAGE* crop = img_create_blank_image(w, h, img->fmt);
	
	if(crop == NULL) return NULL;

	for(int64_t img_y = y; img_y != (y+h); ++img_y)
	{
		for(int64_t img_x = x; img_x != (x+w); ++img_x)
		{
			const PIXEL pixel = img_get_pixel(img, img_x, img_y);
			const uint32_t crop_x = img_x - x;
			const uint32_t crop_y = img_y - y;
			img_set_pixel(crop, crop_x, crop_y, pixel);
		}
	}
	
	return crop;
}

void img_draw_on_image(const IMAGE* surface, const IMAGE* img,
					   const int64_t x, const int64_t y)
{
	if(surface == NULL) return;
	if(img == NULL) return;
	
	for(int64_t surface_y = y; surface_y != (y+img->height); ++surface_y)
	{
		for(int64_t surface_x = x; surface_x != (x+img->width); ++surface_x)
		{
			const uint32_t img_x = surface_x - x;
			const uint32_t img_y = surface_y - y;
			const PIXEL pixel = img_get_pixel(img, img_x, img_y);
			img_set_pixel(surface, surface_x, surface_y, pixel);
		}
	}
}					   

uint8_t* img_to_raw_data(const IMAGE* img, uint64_t* size)
{
	*size = img->width*img->height*img->bpp;
	uint8_t* data = (uint8_t*)calloc(1, *size);

	uint8_t* data_ptr = data;
	for(uint64_t i = 0; i != img->width*img->height; ++i)
	{
		const PIXEL* pixel = &img->data[i];
		switch(img->fmt)
		{
			case FMT_RGB:
			case FMT_RGBA:
			case FMT_GRAY:
				memcpy(data_ptr, &pixel->data, img->bpp);
				data_ptr += img->bpp;
				break;
			case FMT_RGB565:
				const uint16_t rgb565 = pix_to_raw_rgb565(*pixel);
				memcpy(data_ptr, &rgb565, img->bpp);
				data_ptr += img->bpp;
				break;
			default: break;
		}
	}
	
	return data;
}

IMAGE* img_alloc_image()
{
	return (IMAGE*)calloc(1, sizeof(IMAGE));
}

void img_free_image(IMAGE* image)
{
	image->width = 0;
	image->height = 0;
	image->fmt = 0;
	free(image->data);
	image->data = NULL;
	free(image);
}