#pragma once

/*
    CRI-Middleware/CRIWARE file archive (Dreamcast/PS2/Xbox/GC/PC)
    Mostly used for storing ADX files, but can hold any kind of data.
    
    Like AFS2 (AWB), file is in little endian.
    Sections are aligned to 2048 bytes by default.
    
    Has an optional metadata section, which stores
    a file name, date and its size.
*/

/*
    AFS file can have both null entries AND somehow 65535 files
    with last one being null entry.
    It can also have more files in the header than files in metadata.
    File count can also LIE.
    File size in the metadata can also LIE.
    It can also not have the metadata section.
    Couldn't reproduce this fuckery with afslnk and AfsLink.
*/

#include <stdint.h>
#include <time.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/cvector.h>

/*
    Defines
*/
#define AFS_MAGIC                       (const char*)"AFS\0"
#define AFS_HEADER_SIZE                 (uint8_t)(8)
#define AFS_ENTRY_SIZE                  (uint8_t)(8)
#define AFS_ENTRY_METADATA_SIZE         (uint8_t)(48)
#define AFS_ENTRY_METADATA_NAME_SIZE    (uint8_t)(32)
#define AFS_BLOCK_SIZE_DEFAULT          (uint32_t)(2048)
#define AFS_FILE_MIN_SIZE               AFS_BLOCK_SIZE_DEFAULT
#define AFS_MAX_FILES                   (65535)

#define AFS_NO_METADATA                 (uint8_t)(0)
#define AFS_HAS_METADATA                (uint8_t)(1)

#define AFS_GOOD                        (uint8_t)(1)
#define AFS_ERROR                       (uint8_t)(-1)

#define AFS_DATA_BIN                    (uint8_t)(0)
#define AFS_DATA_ADX                    (uint8_t)(1)
#define AFS_DATA_AHX                    (uint8_t)(2)
#define AFS_DATA_AFS                    (uint8_t)(3)

/*
    Structures
*/
typedef struct AFS_ENTRY_METADATA AFS_ENTRY_METADATA;
struct AFS_ENTRY_METADATA
{
    char name[AFS_ENTRY_METADATA_NAME_SIZE];
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t minute;
    uint16_t second;
    uint32_t file_size;     /* But not always */
};

typedef struct AFS_ENTRY AFS_ENTRY;
struct AFS_ENTRY
{
    AFS_ENTRY_METADATA metadata;
    uint8_t* data;
    /*uint32_t id;*/    /* Index in CVEC entries */
    uint32_t size;
    uint8_t data_type;  /* ADX, AFS or BIN */
};

typedef struct AFS_HEADER AFS_HEADER;
struct AFS_HEADER
{
    char magic[4];
    uint32_t file_count;    /* But not always */
};

typedef struct AFS_FILE AFS_FILE;
struct AFS_FILE
{
    CVEC entries;           /* Vector of AFS_ENTRY */
    uint8_t has_metadata;
};

/*
    Functions
*/

/*
    Allocates an empty AFS structure to fill with data.
    
    Returns a pointer to AFS_FILE.
*/
AFS_FILE* afs_alloc();

/*
    Parses the entire AFS archive.
    
    Returns a pointer to AFS_FILE; NULL on error.
*/
AFS_FILE* afs_read_from_fu(FU_FILE* fafs);

/*
    Parses the entire AFS archive.
    
    Returns a pointer to AFS_FILE; NULL on error.
*/
AFS_FILE* afs_read_from_data(const uint8_t* data, const uint32_t size);

/*
    Writes the AFS file ready to be saved to disk.
*/
FU_FILE* afs_write_to_fu(AFS_FILE* afs, const uint32_t block_size);

/*
    Frees the entire structure and the pointer.
    
    Returns NULL.
*/
AFS_FILE* afs_free(AFS_FILE* afs);

/*
    Returns a pointer to specified entry.
    If entry doesn't exist, returns NULL.
*/
AFS_ENTRY* afs_get_entry_by_id(AFS_FILE* afs, const uint64_t id);

/*
    Will insert data to entry specified by id.
    If entry exists, it will replace its data.
    If the name is NULL, it will use the id as filename.
    
    Returns a pointer to specified entry.
*/
AFS_ENTRY* afs_set_entry_data(AFS_FILE* afs, const uint32_t id,
                              const uint8_t* data, const uint32_t size,
                              const char* name, const time_t timestamp);

/*
    Frees and zeroes the entry.
*/
void afs_remove_entry(AFS_FILE* afs, const uint32_t id);

/*
    Returns the amount of files present in entries vector.
*/
const uint32_t afs_get_count(AFS_FILE* afs);

/*
    Returns the offset to specified ID's entry in AFS file.
*/
const uint32_t afs_id_to_entry_offset(const uint32_t id);

/*
    Returns the offset to specified ID's metadata in AFS file.
    Or AFS_ERROR if you go over the range.
*/
const uint32_t afs_id_to_metadata_offset(const uint32_t meta_pos, const uint32_t size, const uint32_t id);

/*
    Returns the type of file, ADX, AHX, AFS or raw binary.
*/
const uint8_t afs_get_file_type(const uint8_t* data, const uint32_t size);
