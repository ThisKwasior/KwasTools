#pragma once

#include <stdint.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/dir_list.h>
#include <kwaslib/core/data/cvector.h>

/* 
    Code here has been "borrowed" from NierDocs and xxk-i.
    Thank you very much!
    https://github.com/ArthurHeitmann/NierDocs/blob/master/tools/datRepacker/datRepacker.py
    https://github.com/xxk-i/DATrepacker
*/

#define DAT_MAGIC                   (const char*)"DAT\0"
#define DAT_DEFAULT_BLOCK_SIZE      16

typedef struct DAT_HEADER DAT_HEADER;
typedef struct DAT_FILE_ENTRY DAT_FILE_ENTRY;
typedef struct DAT_FILE DAT_FILE;

#include "dat_hashtable.h"

struct DAT_HEADER
{
    uint8_t magic[4];               /* {'D','A','T',0} */
    uint32_t file_count;            /* Amount of files in the container */
    uint32_t positions_offset;
    uint32_t extensions_offset;
    uint32_t names_offset;
    uint32_t sizes_offset;
    uint32_t hashtable_offset;
    uint32_t unk1C;                 /* Zero */
};

struct DAT_FILE_ENTRY
{
    uint32_t position;              /* Position in the file */
    uint8_t extension[4];           /* File extension */
    SU_STRING* name;                /* File name */
    uint32_t size;                  /* File size */
    uint8_t* data;                  /* File data */
};

struct DAT_FILE
{
    DAT_HEADER header;              /* Fixed-length header */
    
    uint32_t entry_name_size;       /* Size for file names
                                       First value in names section */
                         
    CVEC entries;
    DAT_HASHTABLE* hashtable;
};

/*
    Implementation
*/

DAT_FILE* dat_parse_file(FU_FILE* file);
FU_FILE* dat_to_fu_file(DAT_FILE* dat, const uint32_t block_size, const uint8_t endian);

void dat_update(DAT_FILE* dat, const uint32_t block_size);

void dat_append_entry(CVEC entries,
                      const char* extension,
                      const char* name,
                      const uint32_t size,
                      const uint8_t* data);

DAT_FILE* dat_alloc_dat();
DAT_FILE* dat_destroy(DAT_FILE* dat);

DAT_FILE_ENTRY* dat_get_entry_by_id(CVEC entries, const uint64_t id);

/*
    Check is simple, we read the file_count as little endian.
    If the value:
        - is greater or equal to 16777216, we can say it's BE
        - if it's less, then it's LE
    
    Check should work because hashtable, and by extension DAT itself,
    can only support up to 65535 files.
        
    Returns endianness as defined in file_utils.h
*/
uint8_t dat_check_endian(const uint32_t file_count);

const uint32_t dat_max_name_len(CVEC entries);