#include "vec.h"

VEC2_FLOAT vec2_mul(const VEC2_FLOAT v1, const VEC2_FLOAT v2)
{
	VEC2_FLOAT v = {.x = v1.x * v2.x,
					.y = v1.y * v2.y};
	return v;
}

VEC3_FLOAT vec3_mul(const VEC3_FLOAT v1, const VEC3_FLOAT v2)
{
	VEC3_FLOAT v = {.x = v1.x * v2.x,
					.y = v1.y * v2.y,
					.z = v1.z * v2.z};
	return v;
}