#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/cri/awb.h>

#include "acb_command.h"

/*
    Vgmstream source code we trust (and, uh, borrow from).
    Thank you very much for your work, vgmstream contributors!
    
    https://github.com/vgmstream/vgmstream/blob/master/src/util/cri_utf.c
*/

/*
	Added the "cri_" prefix to everything because of UTF-8 ambiguity
*/

#define CRI_UTF_MAGIC				"@UTF"
#define CRI_UTF_MAX_SCHEMA_SIZE		0x8000    /* arbitrary max */
#define CRI_UTF_SCHEMA_OFFSET		0x20
#define CRI_COLUMN_BITMASK_FLAG		0xf0
#define CRI_COLUMN_BITMASK_TYPE		0x0f

typedef enum {
    COLUMN_FLAG_NAME            = 0x01,		/* column has name (may be empty) */
    COLUMN_FLAG_DEFAULT         = 0x02,		/* data is found relative to schema start (typically constant value for all rows) */
    COLUMN_FLAG_ROW             = 0x04,		/* data is found relative to row start */
    COLUMN_FLAG_UNDEFINED       = 0x08,		/* shouldn't exist */
	
	/*
		For the sake of making this easier to digest
		I'm making these combined flags
	*/
	COLUMN_FLAG_NONE			= 0x00, /* Actually I don't know */
	COLUMN_FLAG_NAME_DEFAULT	= 0x03,
	COLUMN_FLAG_NAME_ROW		= 0x05,
	COLUMN_FLAG_ROW_DEFAULT		= 0x06, /* Shouldn't exist? */
	COLUMN_FLAG_ALL				= 0x07, /* Shouldn't exist? */
} CRI_UTF_FLAG;

typedef enum {
    COLUMN_TYPE_UINT8           = 0x00,
    COLUMN_TYPE_SINT8           = 0x01,
    COLUMN_TYPE_UINT16          = 0x02,
    COLUMN_TYPE_SINT16          = 0x03,
    COLUMN_TYPE_UINT32          = 0x04,
    COLUMN_TYPE_SINT32          = 0x05,
    COLUMN_TYPE_UINT64          = 0x06,
    COLUMN_TYPE_SINT64          = 0x07,
    COLUMN_TYPE_FLOAT           = 0x08,
    COLUMN_TYPE_DOUBLE          = 0x09,
    COLUMN_TYPE_STRING          = 0x0a,
    COLUMN_TYPE_VLDATA          = 0x0b,
    COLUMN_TYPE_UINT128         = 0x0c, /* for GUIDs */
    COLUMN_TYPE_UNDEFINED       = 0x0f
} CRI_UTF_TYPE;

typedef enum {
    STREAMING_MEMORY			= 0x00, /* Audio is in ACB */
    STREAMING_STREAM			= 0x01, /* Audio is in AWB */
    STREAMING_PREFETCH			= 0x02, /* Memory + Stream */
} CRI_UTF_STREAMING;

typedef struct CRI_UTF_FILE CRI_UTF_FILE;

typedef struct
{
    char magic[4]; /* @UTF */
    uint32_t table_size; /* recalc on write */
    uint16_t version;
    uint16_t rows_offset; /* recalc on write */
    uint32_t string_table_offset; /* recalc on write */ 
    uint32_t data_offset; /* recalc on write */
    uint32_t name_offset; /* In string table */
    uint16_t columns;
    uint16_t row_width; /* recalc on write */
    uint32_t rows;
	
	/* Pointers in string table */
	char* name;
} CRI_UTF_HEADER;

typedef struct
{
    CRI_UTF_TYPE type : 4;
	CRI_UTF_FLAG flag : 4;
} CRI_UTF_FLAG_TYPE;

typedef struct
{
	union
	{
		uint8_t u8;
		int8_t s8;
		uint16_t u16;
		int16_t s16;
		uint32_t u32;
		int32_t s32;
		uint64_t u64;
		int64_t s64;
		float f32;
		double f64;
		char* str;
		uint8_t* vl;
	} data;
	uint32_t offset;
	uint32_t size;
	
	/* UTF Table can hold these as VLDATA */
	CRI_UTF_FILE* utf;
	AWB_FILE* awb;
	ACB_COMMAND* acbcmd;
} CRI_UTF_RECORD;

typedef struct
{
	CRI_UTF_FLAG_TYPE desc;
	uint32_t name_offset;
	
	CRI_UTF_RECORD* records;
	
	char* name;
} CRI_UTF_COLUMN;

typedef struct CRI_UTF_FILE
{	
    CRI_UTF_HEADER header;
    
	CRI_UTF_COLUMN* columns;
	
	uint32_t rows_size;
    char* string_table;
    uint32_t string_table_size;
	uint8_t* data;
	uint32_t data_size;
	
	uint64_t file_offset;
    
} CRI_UTF_FILE;

/*
	Functions
*/
/* Reading */
CRI_UTF_FILE* cri_utf_read_file(FU_FILE* cri);

void cri_utf_read_header(FU_FILE* cri, CRI_UTF_FILE* utf);

void cri_utf_calc_sizes(CRI_UTF_FILE* utf);
void cri_utf_read_string_data_table(FU_FILE* cri, CRI_UTF_FILE* utf);
void cri_utf_parse_schema(FU_FILE* cri, CRI_UTF_FILE* utf);
void cri_utf_read_rows(FU_FILE* cri, CRI_UTF_FILE* utf);

CRI_UTF_RECORD cri_utf_read_by_type(FU_FILE* cri, CRI_UTF_FILE* utf, CRI_UTF_COLUMN* col);

/* Writing */
FU_FILE* cri_utf_write_file(CRI_UTF_FILE* utf);

void cri_utf_write_header(FU_FILE* cri, CRI_UTF_FILE* utf);
void cri_utf_write_schema(FU_FILE* cri, CRI_UTF_FILE* utf);
void cri_utf_write_rows(FU_FILE* cri, CRI_UTF_FILE* utf);

void cri_utf_recalc_for_write(CRI_UTF_FILE* utf);

/* Other */
uint32_t cri_utf_calc_row_width(CRI_UTF_FILE* utf);

const char* cri_utf_flag_to_str(const CRI_UTF_FLAG flag);
const char* cri_utf_type_to_str(const CRI_UTF_TYPE type);
const char* cri_utf_streaming_to_str(const uint8_t streaming);

const CRI_UTF_FLAG cri_utf_str_to_flag(const char* str);
const CRI_UTF_TYPE cri_utf_str_to_type(const char* str);

CRI_UTF_FILE* cri_utf_alloc_file();
CRI_UTF_COLUMN* cri_utf_alloc_columns(const uint32_t column_count);
CRI_UTF_RECORD* cri_utf_alloc_records(const uint32_t record_count);

void cri_utf_free(CRI_UTF_FILE* utf);

void cri_utf_print(CRI_UTF_FILE* utf);