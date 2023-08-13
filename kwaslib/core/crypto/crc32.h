#pragma once

#include <stdint.h>

/*
	https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
*/

uint32_t crc32_encode(const uint8_t* data, const uint64_t size);