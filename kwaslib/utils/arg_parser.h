#pragma once

#include <stdint.h>

#define AP_TYPE_NOV	0 /* Argument isn't followed by a value. A switch perhaps */
#define AP_TYPE_U8	1
#define AP_TYPE_S8	2
#define AP_TYPE_U16	3
#define AP_TYPE_S16	4
#define AP_TYPE_U32	5
#define AP_TYPE_S32	6
#define AP_TYPE_U64	7
#define AP_TYPE_S64	8
#define AP_TYPE_FLT	9
#define AP_TYPE_DBL	10
#define AP_TYPE_STR	11

#define AP_SUCCESS 0
#define AP_ERROR 1

#define AP_ARG_SIZE	64

typedef union 
{
	uint8_t u8;
	int8_t s8;
	uint16_t u16;
	int16_t s16;
	uint32_t u32;
	int32_t s32;
	uint64_t u64;
	int64_t s64;
	float f;
	double d;
	char* str;
} AP_TYPE;

typedef struct
{
	char arg[AP_ARG_SIZE];
	uint8_t type;
} AP_ARG_DESC;

typedef struct
{
	AP_ARG_DESC desc;
	AP_TYPE value;
} AP_ARG_VALUE;

/* Linked list of values */
typedef struct AP_VALUE_NODE AP_VALUE_NODE;

struct AP_VALUE_NODE
{
	AP_ARG_VALUE data;
	AP_VALUE_NODE* next;
};

/* 
	Functions
*/

AP_VALUE_NODE* ap_parse_argv(char** argv, int argc, const AP_ARG_DESC* descs, const uint32_t descs_size);

AP_VALUE_NODE* ap_create_empty_node();

void ap_append_new_elem(AP_VALUE_NODE* node);

void ap_free_node(AP_VALUE_NODE* node);

void ap_print_node(AP_VALUE_NODE* node);

const char* ap_type_str(const uint8_t type);