#pragma once

/*
	ACB Commands
	
	All opcodes and values are in big endian.
	Opcode is 2 bytes long.
	Values have variable length described in one byte after the opcode.
	Some opcodes don't have size though.
    There's also no way to know if 4 byte value is an uint or float.
	
	Most of the opcodes taken from vgmstream, thanks!
	https://github.com/vgmstream/vgmstream/blob/master/src/meta/acb.c#L495
*/

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

/*
    Defines
*/

#define ACB_CMD_OPCODE_TYPE_NOVAL   0
#define ACB_CMD_OPCODE_TYPE_U8      1
#define ACB_CMD_OPCODE_TYPE_U16     2
#define ACB_CMD_OPCODE_TYPE_U32     3
#define ACB_CMD_OPCODE_TYPE_U64     4
#define ACB_CMD_OPCODE_TYPE_F32     5
#define ACB_CMD_OPCODE_TYPE_F64     6
#define ACB_CMD_OPCODE_TYPE_U24     7
#define ACB_CMD_OPCODE_TYPE_U40     8
#define ACB_CMD_OPCODE_TYPE_U48     9
#define ACB_CMD_OPCODE_TYPE_U56     10
#define ACB_CMD_OPCODE_TYPE_VL      11

typedef struct
{
	uint16_t op;
	uint8_t size;
	uint8_t type;
	
	union
	{
		uint8_t u8;			/* 1 */
		uint16_t u16;		/* 2 */
		uint32_t u32;		/* 3, 4 */
		uint64_t u64;		/* 5, 6, 7, 8 */
		float f32;
		double f64;
        uint8_t vl[255];    /* 8< */
	} data;
} ACB_COMMAND_OPCODE;

typedef CVEC ACB_COMMAND;

/*
    Implementation
*/

/*
    Allocates the command structure.
    
    Returns ACB_COMMAND vector pointer.
*/
ACB_COMMAND acb_cmd_alloc();

/*
    Loads all acb commands from provided data.
    
    Returns ACB_COMMAND cvector.
*/
ACB_COMMAND acb_cmd_load_from_data(const uint8_t* data, const uint32_t size);

/*
    Parses an opcode pointed to by data.
    If an opcode of size 4 or 8 isn't know, it will default to uint.
    
    Returns ACB_COMMAND_OPCODE
*/
const ACB_COMMAND_OPCODE acb_cmd_parse_opcode(const uint8_t* data);

/*
    Appends an opcode to the command vector.
*/
void acb_cmd_append_opcode(ACB_COMMAND acbcmd, const ACB_COMMAND_OPCODE op);

/*
    Returns an opcode structure pointer specified by id.
*/
ACB_COMMAND_OPCODE* acb_cmd_get_opcode_by_id(ACB_COMMAND acbcmd, const uint32_t id);

/*
    Frees all data in the vector.
    
    Returns NULL.
*/
ACB_COMMAND acb_cmd_free(ACB_COMMAND acbcmd);

/*
    Writes the acb command into a data buffer.
    
    Returns an SU_STRING pointer with data.
*/
SU_STRING* acb_cmd_to_data(ACB_COMMAND acbcmd);

/*
    Returns the size of output buffer in acb_cmd_to_data()
*/
const uint32_t acb_cmd_calc_buffer_size(ACB_COMMAND acbcmd);

/*
    Returns the amount of opcodes.
*/
const uint32_t acb_cmd_get_opcode_count(ACB_COMMAND acbcmd);

/*

*/
const char* acb_cmd_type_to_str(const uint8_t type);
const uint8_t acb_cmd_str_to_type(const char* str, const uint8_t size);
const uint8_t acb_cmd_size_by_type(const uint8_t type);