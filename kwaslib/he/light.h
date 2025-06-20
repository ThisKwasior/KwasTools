#pragma once

#include <stdint.h>

/* HE1 */
#define HE_LIGHT_TYPE_SUN   0
#define HE_LIGHT_TYPE_OMNI  1

typedef struct
{
    uint32_t type;
    float direction_x;
    float direction_y;
    float direction_z;
    float red;
    float green;
    float blue;
} HE_LIGHT_SUN;

