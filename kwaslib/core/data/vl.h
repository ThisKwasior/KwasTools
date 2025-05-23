#pragma once

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>

/*
    Converts binary data to ascii.
    e.g. \xAB\xCD\x02\x49 => "abcd0249"
    
    Returns a pointer to SU_STRING with hex.
*/
SU_STRING* vl_data_to_hex(const uint8_t* data, const uint32_t size);

/*
    Converts ascii to binary data.
    e.g. "abcd0249" => \xAB\xCD\x02\x49
    
    Returns a pointer to SU_STRING with binary data.
*/
SU_STRING* vl_hex_to_data(const char* hex, const uint32_t size);

/*
    Converts ascii character to value.
    e.g. 'f' => 15
*/
uint8_t vl_ascii_to_nibble(char character);

/*
    Converts a byte to two ascii characters.
    e.g. \0x1D => {'1','d'}
*/
void vl_byte_to_ascii(const uint8_t byte, char* l, char* r);

