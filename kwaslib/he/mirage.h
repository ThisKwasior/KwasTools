#pragma once

/*
	BIGGGG thanks to @ik-01 for the help with
	anim formats and working together on cam-anim format.
    
    Special thanks to @blueskythlikesclouds as well
*/

/*
	Files are in big endian on all platforms.
	Sections are aligned to 4 bytes.
    
    Data could be anything - anims, material, etc.
    
    Offset table points to pointers in data section.
    For *-anim files, offsets are bogus and aren't used by games.
    
    File name is just a NULL terminated string
    at the end of the file, also aligned and padded to 4 bytes.
*/

/*
    Implementation here i wanted to keep as straight-forward as possible.
    
    It doesn't care what's inside the data section,
    it requires that you fill the data_version, data_size, data buffer, offset_table and, optionally, file name string.
    
    Then call mirage_update() and all remaining 
    header fields will be neatly updated.
*/

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/io/string_utils.h>

#include "mirage_keyframe.h"
#include "mirage_string_table.h"

#define MIRAGE_HEADER_SIZE  0x18

typedef struct
{
    uint32_t file_size;
    uint32_t data_version; /* Version of the structure in data section. Important for uv-anim for example. */
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t offset_table_offset;
    uint32_t file_name_offset; /* At the end of file */
} MIRAGE_HEADER;

typedef struct
{
    MIRAGE_HEADER header;
    uint8_t* data;
    CVEC offset_table;
    SU_STRING* file_name;
} MIRAGE_FILE;

/*
    Implementation
*/

/*
    Allocates and initializes all fields to default values.
    
    Returns a pointer to MIRAGE_FILE
*/
MIRAGE_FILE* mirage_alloc();

/*
    Loads the file from a data buffer.
    
    Returns a pointer to a populated structure, otherwise NULL.
*/
MIRAGE_FILE* mirage_load_from_data(const uint8_t* data);

/*
    Exports a MIRAGE_FILE to FU_FILE ready to be saved.
    
    Returns FU_FILE pointer with exported mirage file.
*/
FU_FILE* mirage_export_to_fu(MIRAGE_FILE* mirage);

/*
    Updates header offsets and file size
    based on contents of the MIRAGE_FILE.
*/
void mirage_update(MIRAGE_FILE* mirage);

/*
    Replaces the data section in the MIRAGE_FILE.
*/
void mirage_set_data(MIRAGE_FILE* mirage, const uint8_t* data, const uint32_t size);

/*
    Frees all contents of the structure and the structure itself.
    
    Returns NULL.
*/
MIRAGE_FILE* mirage_free(MIRAGE_FILE* mirage);