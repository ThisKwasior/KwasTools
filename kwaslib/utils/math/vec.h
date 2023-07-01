#pragma once

#include <stdint.h>

/*
	Types
*/

typedef struct
{
	uint32_t x;
	uint32_t y;
} VEC2_UINT32;

typedef struct
{
	float x;
	float y;
} VEC2_FLOAT;

typedef struct
{
	float x;
	float y;
	float z;
} VEC3_FLOAT;

/* 
	Functions
*/

VEC2_FLOAT vec2_mul(const VEC2_FLOAT v1, const VEC2_FLOAT v2);
VEC3_FLOAT vec3_mul(const VEC3_FLOAT v1, const VEC3_FLOAT v2);