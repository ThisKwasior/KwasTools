/*
    https://github.com/Radfordhound/HedgeLib/wiki/BINA-Format
*/

#pragma once

#include <stdint.h>

#define BINA_OFFSET_FLAG_STOP 0
#define BINA_OFFSET_FLAG_6 1
#define BINA_OFFSET_FLAG_14 2
#define BINA_OFFSET_FLAG_30 3

typedef struct BINA_Header
{
    uint8_t magic[4]; /* BINA */
    uint8_t ver_major;
    uint8_t ver_minor;
    uint8_t ver_rev;
    uint8_t endianness;
    uint32_t file_size;
    uint16_t node_count;
    uint16_t unk_01;
} BINA_Header;

typedef struct BINA_Offset
{
    uint8_t off_flag;
    uint32_t off_value;
    uint32_t pos;
    uint32_t value;
} BINA_Offset;

typedef struct BINA_DATA_Chunk
{
    uint8_t magic[4]; /* DATA */
    uint32_t data_size;
    uint32_t string_table_offset;
    uint32_t string_table_size;
    uint32_t offset_table_size;
    uint32_t additional_data_size; /* Always 0x18 */
    uint8_t additional_data[0x18];
    
    uint8_t* string_table;
    BINA_Offset* offsets;
    uint32_t offsets_size;
} BINA_DATA_Chunk;

/*
    CPIC - exclusive to pcmodel and pccol files from what i see
*/
typedef struct BINA_CPIC_Instance
{
    uint32_t name_offset;
    uint32_t unk_01; /* 0 */
    uint32_t name2_offset;
    uint32_t unk_02; /* 0 */
    float pos_x;
    float pos_y;
    float pos_z;
    float rot_x;
    float rot_y;
    float rot_z;
    uint32_t unk_03; /* 0 */
    float scale_x;
    float scale_y;
    float scale_z;
    uint32_t unk_04; /* 0 */
    uint32_t unk_05; /* 0 - last instance will read 4 bytes from string table */
    
} BINA_CPIC_Instance;

typedef struct BINA_CPIC_Chunk
{
    uint8_t magic[4]; /* CPIC */
    uint32_t ver_number; /* Always 2 apparently */
    uint32_t instances_offset; /* Always 0x18 */
    uint32_t unk_01; /* 0 */
    uint32_t instances_count;
    uint32_t unk_02; /* 0 */
    
    BINA_CPIC_Instance* instances;
} BINA_CPIC_Chunk;

/*
    Functions
*/
uint8_t BINA_read_header(uint8_t* data, BINA_Header* header);
void BINA_header_fix_endian(BINA_Header* header);

uint8_t BINA_read_DATA(uint8_t* data, BINA_DATA_Chunk* data_chunk);
void BINA_DATA_fix_endian(BINA_DATA_Chunk* data_chunk);

uint8_t BINA_read_CPIC(uint8_t* data, BINA_CPIC_Chunk* cpic_chunk);
void BINA_CPIC_fix_endian(BINA_CPIC_Chunk* cpic_chunk);
void BINA_CPIC_Instance_fix_endian(BINA_CPIC_Instance* cpic_instance);

uint8_t BINA_parse_offset(uint8_t* data, BINA_Offset* offset);