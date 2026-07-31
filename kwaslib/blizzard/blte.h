#pragma once

/*
    Based on
    https://wowdev.wiki/BLTE
    
    Big Endian.
*/

#include <stdint.h>
#include <stdlib.h>

#include <kwaslib/core/data/cvector.h>

/*
    Defines
*/

#define BLTE_MAGIC                  "BLTE"

#define BLTE_TABLE_FMT_DEFAULT      (0x0F)
#define BLTE_TABLE_FMT_DEFAULT_SIZE (24)
#define BLTE_TABLE_FMT_AVOWED       (0x10)
#define BLTE_TABLE_FMT_AVOWED_SIZE  (40)

#define BLTE_ENCODING_PLAIN         ('N')   /* Plain data       */
#define BLTE_ENCODING_ZLIB          ('Z')   /* Zlib compressed  */
#define BLTE_ENCODING_LZ4           ('4')   /* LZ4 compressed   */
#define BLTE_ENCODING_RECURSIVE     ('R')   /* BLTE within BLTE */
#define BLTE_ENCODING_CRYPT         ('E')   /* Encrypted data   */

/*
    Structures
*/
typedef struct BLTE_HEADER      BLTE_HEADER;
typedef struct BLTE_BLOCK_0F    BLTE_BLOCK_0F;
typedef struct BLTE_BLOCK_10    BLTE_BLOCK_10;
typedef union  BLTE_BLOCK       BLTE_BLOCK;
typedef struct BLTE_CHUNKINFO   BLTE_CHUNKINFO;
typedef struct BLTE_FILE        BLTE_FILE;

struct BLTE_HEADER
{
    char magic[4];
    uint32_t header_size;
};

struct BLTE_BLOCK_0F
{
    uint32_t raw_size;          /* Compressed size */
    uint32_t logical_size;      /* Decompressed size */
    uint8_t hash[16];           /* MD5 checksum of the compressed block */
};

struct BLTE_BLOCK_10
{
    uint32_t raw_size;          /* Compressed size */
    uint32_t logical_size;      /* Decompressed size */
    uint8_t hash[16];           /* MD5 checksum of the compressed block */
    uint8_t logical_hash[16];   /* MD5 checksum of the uncompressed block */
};

union BLTE_BLOCK
{
    BLTE_BLOCK_0F   block_0f;
    BLTE_BLOCK_10   block_10;
};

struct BLTE_CHUNKINFO
{
    uint32_t table_fmt : 8;     /* 0xF except in Avowed which is 0x10 */
    uint32_t num_blocks : 24;
    
    CVEC blocks;                /* Vector of BLTE_BLOCK */
};

struct BLTE_FILE
{
    BLTE_HEADER header;
    BLTE_CHUNKINFO chunkinfo;
    
    uint8_t* data;
};

/*
    Functions
*/

/*

*/
BLTE_FILE* blte_read_file(const uint8_t* data);

/*
    Returns NULL
*/
BLTE_FILE* blte_free(BLTE_FILE* blte);


/*
    Reads the header.
    If the header is wrong, header_size will be -1.
*/
BLTE_HEADER blte_read_header(const uint8_t* data);

/*
    Reads the chunk info.
*/
BLTE_CHUNKINFO blte_read_chunkinfo(const uint8_t* data);

/*
    Reads the data into an array and returns a pointer to it.
    `data` needs to point to data blocks right after the block descriptors
*/
uint8_t* blte_read_data(const BLTE_CHUNKINFO* const chunkinfo, const uint8_t* data);

/*
    Just copies the raw data from blocks.
*/
uint8_t* blte_data_to_raw(const BLTE_FILE* const blte);


/*
    Converts the data from blocks into its raw representation.
    
    TODO: For now, only BLTE_ENCODING_PLAIN is supported,
          other types return without any conversion.
*/
uint8_t* blte_data_to_logical(const BLTE_FILE* const blte, uint64_t* out_size);


/*
    Returns the sum of sizes of raw blocks.
*/
static inline uint64_t blte_get_raw_data_size(const BLTE_CHUNKINFO* const chunkinfo)
{
    uint64_t sum = 0;
    
    for(uint32_t i = 0; i != chunkinfo->num_blocks; ++i)
    {
        BLTE_BLOCK* cur_block = (BLTE_BLOCK*)cvec_at(chunkinfo->blocks, i);
        sum += cur_block->block_0f.raw_size;
    }
    
    return sum;
}

/*
    Returns the sum of sizes of logical blocks.
*/
static inline uint64_t blte_get_logical_data_size(const BLTE_CHUNKINFO* const chunkinfo)
{
    uint64_t sum = 0;
    
    for(uint32_t i = 0; i != chunkinfo->num_blocks; ++i)
    {
        BLTE_BLOCK* cur_block = (BLTE_BLOCK*)cvec_at(chunkinfo->blocks, i);
        sum += cur_block->block_0f.logical_size;
    }
    
    return sum;
}