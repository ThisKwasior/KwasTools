#pragma once

#if defined(__unix__) || defined(__linux__)
#define _FILE_OFFSET_BITS 64
#endif

#include <stdint.h>
#include <stdio.h>

#include "path_utils.h"

#define FU_SUCCESS	0
#define FU_ERROR	1
#define FU_EOF		2
#define FU_ENDDATA	3	/* Last data read */
#define FU_FEXISTS	4	/* File exists */
#define FU_NOTMEMF	5	/* File is not in memory */
#define FU_REQ0		6	/* Requested size is 0 */
#define FU_REQBEL0	7	/* Requested size is less than 0 */

#define FU_SEEK_SET	0
#define FU_SEEK_CUR	1
#define FU_SEEK_END	2

#define FU_HOST_ENDIAN 0
#define FU_LITTLE_ENDIAN 1
#define FU_BIG_ENDIAN 2

typedef struct
{
	/* File buffer */
	char* buf;
	uint8_t is_buf;  /* If true, the file is in char buffer */
	uint8_t do_free; /* If true, the buffer will be freed by fu_close() */
	uint8_t writeable; /* You can write to the buffer */
	
	/* File streamed */
	FILE* f;
	
	/* Shared by both */
	uint64_t size;
	uint64_t rem;
	uint64_t pos;
} FU_FILE;

/*
	Functions
*/

/* Return FU_SUCCESS on success, FU_ERROR otherwise. */
uint8_t fu_open_file(const char* path, const uint8_t to_memory, FU_FILE* f);
uint8_t fu_open_file_pu(PU_PATH* path, const uint8_t to_memory, FU_FILE* f);

/* Just allocates the structure and returns a pointer */
FU_FILE* fu_alloc_file();

uint64_t fu_get_file_size(const char* path);
uint64_t fu_get_file_size_pu(PU_PATH* path);

uint8_t fu_seek(FU_FILE* f, int64_t offset, uint8_t whence);

uint64_t fu_tell(FU_FILE* f);

void fu_close(FU_FILE* f);

const char* fu_status_str(const uint8_t status);

/* 
	Memory file
*/
uint8_t fu_create_mem_file(FU_FILE* f);

/* Changes the size of buf in the file */
uint8_t fu_change_buf_size(FU_FILE* f, const int64_t desired_size);

/* Adds bytes_req to file's size and allocates new buffer */
uint8_t fu_add_to_buf_size(FU_FILE* f, const int64_t bytes_req);

/* Checks and expands buf in FU_FILE if needed */
uint8_t fu_check_buf_rem(FU_FILE* f, const uint64_t bytes_req);

/* Create memory file from data provided */
FU_FILE* fu_create_mem_file_data(const uint8_t* data, const uint64_t size);

/*
	Saving
*/
uint8_t fu_to_file(const char* path, FU_FILE* f, const uint8_t overwrite);
uint8_t fu_to_file_pu(PU_PATH* path, FU_FILE* f, const uint8_t overwrite);

uint8_t fu_buffer_to_file(const char* path, const char* buffer, const uint64_t size, const uint8_t overwrite);
uint8_t fu_buffer_to_file_pu(PU_PATH* path, const char* buffer, const uint64_t size, const uint8_t overwrite);

/*
	Readers
*/

uint8_t fu_check_read_req(FU_FILE* f, const uint32_t req);

/* 
	Tries to read the amount of data requested to `buf`.
	Amount of bytes read is returned with `bytes`. Can be NULL.
	Returns status.	
*/
uint8_t fu_read_data(FU_FILE* f, uint8_t* buf, const uint64_t size, uint64_t* read);
uint8_t fu_read_u8(FU_FILE* f, uint8_t* status);

/*
	Endian param is:
	0 - Read as host endian (FU_HOST_ENDIAN)
	1 - little endian (FU_LITTLE_ENDIAN)
	2 - big endian (FU_BIG_ENDIAN)
	
	Made it like that because I wanted the file_utils not be a giant
	mess of functions like type_readers/writers
*/
uint16_t fu_read_u16(FU_FILE* f, uint8_t* status, const uint8_t endian);
uint32_t fu_read_u32(FU_FILE* f, uint8_t* status, const uint8_t endian);
uint64_t fu_read_u64(FU_FILE* f, uint8_t* status, const uint8_t endian);
float fu_read_f32(FU_FILE* f, uint8_t* status, const uint8_t endian);
double fu_read_f64(FU_FILE* f, uint8_t* status, const uint8_t endian);

/*
	Writers
*/
uint8_t fu_write_data(FU_FILE* f, const uint8_t* data, const uint64_t size);
uint8_t fu_write_u8(FU_FILE* f, const uint8_t data);

/*
	Endian param is:
	0 - Read as host endian (FU_HOST_ENDIAN)
	1 - little endian (FU_LITTLE_ENDIAN)
	2 - big endian (FU_BIG_ENDIAN)
	
	Made it like that because I wanted the file_utils not be a giant
	mess of functions like type_readers/writers
*/
uint8_t fu_write_u16(FU_FILE* f, const uint16_t data, const uint8_t endian);
uint8_t fu_write_u32(FU_FILE* f, const uint32_t data, const uint8_t endian);
uint8_t fu_write_u64(FU_FILE* f, const uint64_t data, const uint8_t endian);
uint8_t fu_write_f32(FU_FILE* f, const float data, const uint8_t endian);
uint8_t fu_write_f64(FU_FILE* f, const double data, const uint8_t endian);
