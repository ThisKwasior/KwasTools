#include "BINA.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <kwaslib/utils/endianness.h>
#include <kwaslib/utils/type_readers.h>

static uint8_t swap_endianness = 0;

uint8_t BINA_read_header(uint8_t* data, BINA_Header* header)
{
    tr_read_array(data, sizeof(BINA_Header), (uint8_t*)header);
    if(strncmp((const char*)header->magic, "BINA", 4) != 0) 
        return 0;
    
    BINA_header_fix_endian(header);
    return 1;
}

void BINA_header_fix_endian(BINA_Header* header)
{	
    const uint8_t is_be = ed_is_BE();
    
    /* Only need to check for ('B' && !is_be) and ('L' && is_be) */
    if(((header->endianness == 'L') && is_be)
        || ((header->endianness == 'B') && (is_be == 0)))
    {
        swap_endianness = 1;
    }
    
    if(swap_endianness)
    {
        header->file_size = ed_swap_endian_32(header->file_size);
        header->node_count = ed_swap_endian_16(header->node_count);
        header->unk_01 = ed_swap_endian_16(header->unk_01);
    }
}

uint8_t BINA_read_DATA(uint8_t* data, BINA_DATA_Chunk* data_chunk)
{
    tr_read_array(&data[0x10], sizeof(BINA_DATA_Chunk), (uint8_t*)data_chunk);
    if(strncmp((const char*)data_chunk->magic, "DATA", 4) != 0) 
        return 0;

    BINA_DATA_fix_endian(data_chunk);

    data_chunk->string_table = &data[0x40 + data_chunk->string_table_offset];
    data_chunk->offsets = (BINA_Offset*)calloc(data_chunk->offset_table_size, sizeof(BINA_Offset));
    
    const uint32_t offtabpos = 0x40 + data_chunk->string_table_offset + data_chunk->string_table_size;
    uint32_t it = 0;
    uint32_t pos = 0;
    uint32_t pos_abs = 0x40;
    
    while(1)
    {
        BINA_Offset* cur_off = &data_chunk->offsets[it];
        
        const uint8_t size = BINA_parse_offset(&data[offtabpos+pos], cur_off);
        if(size == 0) 
        {
            data_chunk->offsets_size = it;
            break;
        }
        pos_abs += cur_off->off_value;
        cur_off->pos = pos_abs;
        cur_off->value = tr_read_u32(&data[cur_off->pos]) + 0x40;
        if(swap_endianness) cur_off->value = cur_off->value;
        
        pos += size;
        it += 1;
    }
    
    return 1;
}

void BINA_DATA_fix_endian(BINA_DATA_Chunk* data_chunk)
{
    if(swap_endianness)
    {
        data_chunk->string_table_offset = ed_swap_endian_32(data_chunk->string_table_offset);
        data_chunk->string_table_size = ed_swap_endian_32(data_chunk->string_table_size);
        data_chunk->offset_table_size = ed_swap_endian_32(data_chunk->offset_table_size);
        data_chunk->additional_data_size = ed_swap_endian_32(data_chunk->additional_data_size);
    }
}

uint8_t BINA_read_CPIC(uint8_t* data, BINA_CPIC_Chunk* cpic_chunk)
{
    tr_read_array(&data[0x40], sizeof(BINA_CPIC_Chunk), (uint8_t*)cpic_chunk);
    if(strncmp((const char*)cpic_chunk->magic, "CPIC", 4) != 0) 
        return 0;

    BINA_CPIC_fix_endian(cpic_chunk);
    
    const uint32_t inst_size = sizeof(BINA_CPIC_Instance);
    printf("%u %u\n", inst_size, cpic_chunk->instances_count);
    cpic_chunk->instances = (BINA_CPIC_Instance*)calloc(cpic_chunk->instances_count, inst_size);
    
    uint32_t pos = 0x40 + cpic_chunk->instances_offset;
    for(uint32_t i = 0; i < cpic_chunk->instances_count; ++i)
    {
        tr_read_array(&data[pos], inst_size, (uint8_t*)&cpic_chunk->instances[i]);
        BINA_CPIC_Instance_fix_endian(&cpic_chunk->instances[i]);
        pos += inst_size;
    }
    
    return 1;
}

void BINA_CPIC_fix_endian(BINA_CPIC_Chunk* cpic_chunk)
{
    if(swap_endianness)
    {
        cpic_chunk->ver_number = ed_swap_endian_32(cpic_chunk->ver_number);
        cpic_chunk->instances_offset = ed_swap_endian_32(cpic_chunk->instances_offset);
        cpic_chunk->unk_01 = ed_swap_endian_32(cpic_chunk->unk_01);
        cpic_chunk->instances_count = ed_swap_endian_32(cpic_chunk->instances_count);
        cpic_chunk->unk_02 = ed_swap_endian_32(cpic_chunk->unk_02);
    }
}

void BINA_CPIC_Instance_fix_endian(BINA_CPIC_Instance* cpic_instance)
{
    if(swap_endianness)
    {
        cpic_instance->name_offset = ed_swap_endian_32(cpic_instance->name_offset);
        cpic_instance->name2_offset = ed_swap_endian_32(cpic_instance->name2_offset);
        cpic_instance->pos_x = ed_swap_endian_32(cpic_instance->pos_x);
        cpic_instance->pos_y = ed_swap_endian_32(cpic_instance->pos_y);
        cpic_instance->pos_z = ed_swap_endian_32(cpic_instance->pos_z);
        cpic_instance->rot_x = ed_swap_endian_32(cpic_instance->rot_x);
        cpic_instance->rot_y = ed_swap_endian_32(cpic_instance->rot_y);
        cpic_instance->rot_z = ed_swap_endian_32(cpic_instance->rot_z);
        cpic_instance->scale_x = ed_swap_endian_32(cpic_instance->scale_x);
        cpic_instance->scale_y = ed_swap_endian_32(cpic_instance->scale_y);
        cpic_instance->scale_z = ed_swap_endian_32(cpic_instance->scale_z);
    }
}

uint8_t BINA_parse_offset(uint8_t* data, BINA_Offset* offset)
{
    offset->off_flag = tr_read_u8(data)>>6;
    offset->off_value = 0;
    
    switch(offset->off_flag)
    {
        case BINA_OFFSET_FLAG_STOP:
            return 0;
            break;
        case BINA_OFFSET_FLAG_6:
            offset->off_value = (uint8_t)(tr_read_u8(data)<<2);
            return 1;
            break;
        case BINA_OFFSET_FLAG_14:
            offset->off_value = (uint16_t)(tr_read_u16(data)<<2);
            if(swap_endianness) offset->off_value = ed_swap_endian_16(offset->off_value);
            return 2;
            break;
        case BINA_OFFSET_FLAG_30:
            offset->off_value = (uint32_t)(tr_read_u32(data)<<2);
            if(swap_endianness) offset->off_value = ed_swap_endian_32(offset->off_value);
            return 4;
            break;
    }
    
    return 0;
}