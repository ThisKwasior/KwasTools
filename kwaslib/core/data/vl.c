#include "vl.h"

SU_STRING* vl_data_to_hex(const uint8_t* data, const uint32_t size)
{
    const uint32_t new_size = size*2;
    SU_STRING* hex = su_create_string(NULL, new_size);
    
    uint32_t data_it = 0;
    for(uint32_t i = 0; i != new_size; i+=2)
    {
        vl_byte_to_ascii(data[data_it], &hex->ptr[i+0], &hex->ptr[i+1]);
        data_it += 1;
    }
    
    return hex;
}

SU_STRING* vl_hex_to_data(const char* hex, const uint32_t size)
{
    const uint32_t new_size = size/2;
    SU_STRING* data = su_create_string(NULL, new_size);
    
    uint32_t hex_it = 0;
    for(uint32_t i = 0; i != new_size; ++i)
    {
        const uint8_t l = vl_ascii_to_nibble(hex[hex_it]);
        const uint8_t r = vl_ascii_to_nibble(hex[hex_it+1]);
        data->ptr[i] = (l<<4)+r;
        hex_it += 2;
    }
    
    return data;
}

uint8_t vl_ascii_to_nibble(char character)
{
    character -= '0';
    
    if(character < 10)
        return (uint8_t)(character);
        
    character &= 0x0F;
    character += 9;
    return (uint8_t)(character);
}

void vl_byte_to_ascii(const uint8_t byte, char* l, char* r)
{
    *l = (byte&0xF0)>>4;
    *r = byte&0x0F;
    
    if((*l) < 10)
        (*l) += '0';
    else
        (*l) += 87;
    
    if((*r) < 10)
        (*r) += '0';
    else
        (*r) += 87;
}
