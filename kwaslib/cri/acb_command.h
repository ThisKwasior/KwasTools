#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>

/*
	ACB Commands
	
	All opcodes and values are in big endian.
	Opcode is 2 bytes long.
	Values have variable length described in one byte after the opcode.
	Some opcodes don't have size though.
	
	Most of the opcodes taken from vgmstream, thanks!
	https://github.com/vgmstream/vgmstream/blob/master/src/meta/acb.c#L495
*/

typedef enum 
{
	OPCODE_NOP								= 0,
	OPCODE_CATEGORY							= 65,
	OPCODE_POS_3D_DISTANCE_MIN				= 68,	/* float/double */
	OPCODE_POS_3D_DISTANCE_MAX				= 69,	/* float/double */
	OPCODE_VOLUME_GAIN_RESOLUTION100		= 87,
	OPCODE_UNK_006F							= 111,
	OPCODE_NOTE_ON							= 2000,
	OPCODE_DELAY							= 2001,
	OPCODE_UNK_07D5							= 2005,
	OPCODE_BLOCK_END						= 4050,
} ACB_COMMAND_OPCODES;

typedef enum {
    OPCODE_TYPE_UINT8			= 0x00,
    OPCODE_TYPE_UINT16			= 0x02,
    OPCODE_TYPE_UINT32			= 0x04,
    OPCODE_TYPE_UINT64			= 0x06,
    OPCODE_TYPE_FLOAT			= 0x08,
    OPCODE_TYPE_DOUBLE			= 0x09,
    OPCODE_TYPE_NOVAL			= 0x0f,
	OPCODE_TYPE_UINT24			= 0xf3,
	OPCODE_TYPE_UINT40			= 0xf5,
	OPCODE_TYPE_UINT48			= 0xf6,
	OPCODE_TYPE_UINT56			= 0xf7,
} ACB_COMMAND_TYPE;

typedef struct
{
	uint16_t op;
	uint8_t size;
	
	ACB_COMMAND_TYPE type;
	
	union
	{
		uint8_t u8;			/* 1 */
		uint16_t u16;		/* 2 */
		uint32_t u24 : 24;	/* 3 */
		uint32_t u32;		/* 4 */
		uint64_t u40 : 40;	/* 5 */
		uint64_t u48 : 48;	/* 6 */
		uint64_t u56 : 56;	/* 7 */
		uint64_t u64;		/* 8 */
		float f32;
		double f64;
	} data;
	
} ACB_COMMAND_OPCODE;

typedef struct
{
	ACB_COMMAND_OPCODE* opcodes;
	uint32_t opcodes_count;
	
} ACB_COMMAND;

/*
	Functions
*/

ACB_COMMAND* acb_command_parse_data(const uint8_t* data, const uint32_t data_size);
FU_FILE* acb_command_to_fu(ACB_COMMAND* cmd);

ACB_COMMAND* acb_command_check_data(FU_FILE* data_fu);

ACB_COMMAND* acb_command_alloc_command();
ACB_COMMAND_OPCODE* acb_command_alloc_opcodes(const uint32_t amount);

/* Won't free `cmd` itself*/
void acb_command_free(ACB_COMMAND* cmd);

void acb_command_print(ACB_COMMAND* cmd);