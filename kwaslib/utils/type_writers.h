#pragma once

/*
    Set of functions for writing LE and BE types.
*/

#include <stdint.h>

/*
    Writers for any endianness
*/
void tw_write_u8(const uint8_t data, uint8_t* out);
void tw_write_array(const uint8_t* data, const uint64_t len, uint8_t* out);

/*
    Writers for host endianness.
    Doesn't check for endianness, just writes data.
*/
void tw_write_u16(const uint16_t data, uint8_t* out);
void tw_write_u32(const uint32_t data, uint8_t* out);
void tw_write_u64(const uint64_t data, uint8_t* out);
void tw_write_f32(const float data, uint8_t* out);
void tw_write_f64(const double data, uint8_t* out);

/*
    Writers for little endian
*/
void tw_write_u16le(const uint16_t data, uint8_t* out);
void tw_write_u32le(const uint32_t data, uint8_t* out);
void tw_write_u64le(const uint64_t data, uint8_t* out);
void tw_write_f32le(const float data, uint8_t* out);
void tw_write_f64le(const double data, uint8_t* out);

/*
    Writers for big endian
*/
void tw_write_u16be(const uint16_t data, uint8_t* out);
void tw_write_u32be(const uint32_t data, uint8_t* out);
void tw_write_u64be(const uint64_t data, uint8_t* out);
void tw_write_f32be(const float data, uint8_t* out);
void tw_write_f64be(const double data, uint8_t* out);