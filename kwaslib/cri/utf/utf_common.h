#pragma once

#include <stdint.h>

uint8_t utf_get_type_size(const uint8_t type);

const char* utf_type_to_str(const uint8_t type);

uint8_t utf_str_to_type(const char* str, const uint32_t size);