#pragma once

#include <stdint.h>

typedef struct
{
	char* ptr;
	uint32_t size;
} SU_STRING;

/*
	Allocates and populates an SU_STRING.
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