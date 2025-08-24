#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define SU_STRINGS_MATCH        0
#define SU_ERROR_STR_NO_MATCH   1
#define SU_ERROR_LEN_NO_MATCH   2

typedef struct
{
	char* ptr;
	uint32_t size;
} SU_STRING;

/*
	Allocates and populates an SU_STRING*.
    If str is NULL, then it will create a zero-initalized string.
    Returns NULL on error.
*/
SU_STRING* su_create_string(const char* str, const uint32_t size);

/*
	Frees string and its contents.
    Returns NULL.
*/
SU_STRING* su_free(SU_STRING* str);

/*
	Inserts a char* string into an existing SU_STRING
*/
void su_insert_char(SU_STRING* sustr, const uint32_t pos,
                    const char* str, const uint32_t size);

/*
	Inserts an SU_STRING into an existing SU_STRING
*/
void su_insert_string(SU_STRING* sustr, const uint32_t pos, SU_STRING* to_insert);

/*
	Makes a copy of an SU_STRING
*/
SU_STRING* su_copy(SU_STRING* sustr);
                 
/*
	Removes a portion of an existing SU_STRING
*/
void su_remove(SU_STRING* sustr, const uint32_t pos, uint32_t len);

/*
	Makes a copy of a portion of an SU_STRING
*/
SU_STRING* su_cut(SU_STRING* sustr, const uint32_t pos, uint32_t len);

/*
    Compares strings.
    
    Returns:
        0 - Strings are the same
        1 - Strings differ in contents
        2 - Strings differ in size
*/
const uint8_t su_cmp_char(const char* s1, const uint64_t s1s,
                          const char* s2, const uint64_t s2s);
                        
const uint8_t su_cmp_string(SU_STRING* s1, SU_STRING* s2);
const uint8_t su_cmp_string_char(SU_STRING* s1, const char* s2, const uint64_t s2s);