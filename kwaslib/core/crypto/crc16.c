#include "crc16.h"

uint16_t crc16_calc_hash_bit_by_bit(const uint8_t* data, const uint64_t size,
                                    const uint16_t poly,
                                    const uint16_t init, const uint16_t xorout,
                                    const uint8_t refin, const uint8_t refout)
{
    uint16_t crc = init;

    for(uint64_t i = 0; i < size; ++i)
    {
        uint8_t c = data[i];
        
        if(refin == CRC_REFLECTION_TRUE)
            c = crc_reflect_u8(c);
        
        for(uint8_t j = 0; j < 8; ++j)
        {
            const uint16_t bit = (c >> (7-j)) & 1;
            const uint16_t msb = (crc >> 15) & 1;
            crc <<= 1;
            
            if(bit ^ msb)
                crc ^= poly;
        }
    }

    if(refout == CRC_REFLECTION_TRUE)
        crc = crc_reflect_u16(crc);
    
    crc ^= xorout;
    return crc;
}