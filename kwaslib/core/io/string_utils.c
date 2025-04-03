#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

SU_STRING* su_create_string(const char* str, const uint32_t size)
{
    SU_STRING* pustr = (SU_STRING*)calloc(1, sizeof(SU_STRING));
    
    if(pustr)
    {
        pustr->ptr = (char*)calloc(size+1, 1);
        pustr->size = size;
        memcpy(&pustr->ptr[0], str, size);
    }
    
    return pustr;
}

SU_STRING* su_free(SU_STRING* str)
{
    free(str->ptr);
    str->ptr = NULL;
    str->size = 0;
    free(str);
    
    return NULL;
}

void su_insert_char(SU_STRING* sustr, const uint32_t pos,
                    const char* str, const uint32_t size)
{
    const uint32_t new_size = sustr->size + size;
    char* buf = (char*)calloc(1, new_size+1);
    
    if(buf)
    {
        if(pos == 0)
        {
            memcpy(&buf[0], str, size);
            memcpy(&buf[size], sustr->ptr, sustr->size);
        }
        else if(pos > sustr->size)
        {
            memcpy(&buf[0], sustr->ptr, sustr->size);
            memcpy(&buf[sustr->size], str, size);
        }
        else
        {
            memcpy(&buf[0], sustr->ptr, pos);
            memcpy(&buf[pos], str, size);
            memcpy(&buf[pos+size], &sustr->ptr[pos], sustr->size-pos);
        }
    }
    
    free(sustr->ptr);
    
    sustr->ptr = buf;
    sustr->size = new_size;
}

void su_insert_string(SU_STRING* sustr, const uint32_t pos, SU_STRING* to_insert)
{
    su_insert_char(sustr, pos, to_insert->ptr, to_insert->size);
}

SU_STRING* su_copy(SU_STRING* sustr)
{
    return su_cut(sustr, 0, sustr->size);
}

void su_remove(SU_STRING* sustr, const uint32_t pos, uint32_t len)
{
    if(pos >= sustr->size)
    {
        return;
    }
    
    if(len >= sustr->size)
    {
        len = sustr->size;
    }
    
    SU_STRING* new_str = su_create_string("", 0);
    
    const uint32_t c1_pos = 0;
    const uint32_t c1_size = pos;
    const uint32_t c2_pos = pos+len;
    const uint32_t c2_size = (c2_pos >= sustr->size) ? 0 : (sustr->size-c2_pos);
    
    su_insert_char(new_str, -1, &sustr->ptr[c1_pos], c1_size);
    su_insert_char(new_str, -1, &sustr->ptr[c2_pos], c2_size);
    
    free(sustr->ptr);
    sustr->ptr = new_str->ptr;
    sustr->size = new_str->size;
    free(new_str);
}

SU_STRING* su_cut(SU_STRING* sustr, const uint32_t pos, uint32_t len)
{
    SU_STRING* new_str = su_create_string("", 0);
    
    if(pos >= sustr->size)
    {
        return new_str;
    }
    
    const uint32_t rem_bytes = sustr->size-pos;
    const uint32_t new_size = (rem_bytes <= len) ? rem_bytes : len;
    
    su_insert_char(new_str, -1, &sustr->ptr[pos], new_size);
    
    return new_str;
}

const uint8_t su_cmp_char(const char* s1, const uint64_t s1s,
                          const char* s2, const uint64_t s2s)
{
    if(s1s != s2s)
    {
        return SU_ERROR_LEN_NO_MATCH;
    }
    
    if(strncmp(s1, s2, s1s) != 0)
    {
        return SU_ERROR_STR_NO_MATCH;
    }
    
    return SU_STRINGS_MATCH;
}

const uint8_t su_cmp_string(SU_STRING* s1, SU_STRING* s2)
{
    return su_cmp_char(s1->ptr, s1->size, s2->ptr, s2->size);
}

const uint8_t su_cmp_string_char(SU_STRING* s1, const char* s2, const uint64_t s2s)
{
    return su_cmp_char(s1->ptr, s1->size, s2, s2s);
}