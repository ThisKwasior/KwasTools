#pragma once

#include <stdint.h>
#include <string.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>
#include <kwaslib/core/crypto/crc32.h>

/*
    Defines
*/
#define AP_STAT_ERROR       (uint8_t)(0)
#define AP_STAT_SUCCESS     (uint8_t)(1)
#define AP_STAT_UNK_TYPE    (uint8_t)(2)
#define AP_STAT_ARG_NO_VAL  (uint8_t)(3)    /* Last arg has no passed value */
#define AP_STAT_ARGC_0      (uint8_t)(4)    /* No arguments to parse */

#define AP_TYPE_ILL         (uint8_t)(-1)   /* Illegal type */
#define AP_TYPE_U64	        (uint8_t)(10)
#define AP_TYPE_S64	        (uint8_t)(11)
#define AP_TYPE_F64	        (uint8_t)(12)
#define AP_TYPE_STR	        (uint8_t)(13)
#define AP_TYPE_BOOL	    (uint8_t)(14)
#define AP_TYPE_NOV         (uint8_t)(15)   /* Arg isn't followed by a value. Internally a bool.
                                               Every parsed arg of this type will be
                                               a negation of default value. 
                                               i.e. 0 -> 1, 1 -> 0 */
                                            

/*
    Macros for getting individual values from arguments and descriptors.
*/
#define AP_GET_ARG_UINT(arg)    (arg->value.u64)
#define AP_GET_ARG_INT(arg)     (arg->value.s64)
#define AP_GET_ARG_FLOAT(arg)   (arg->value.f64)
#define AP_GET_ARG_STR(arg)     (const char*)(arg->value.str)
#define AP_GET_ARG_BOOL(arg)    (uint8_t)(arg->value.b)
#define AP_GET_ARG_NOV(arg)     (uint8_t)(arg->value.nov)

/*
    Misc macros
*/
#define AP_VEC_ELEM_BY_ID(v,i)          (cvec_at(v,i))
#define AP_DESC_FROM_VEC_BY_ID(v,i)     (AP_ARG_DESC*)(AP_VEC_ELEM_BY_ID(v,i))
#define AP_ARG_FROM_VEC_BY_ID(v,i)      (AP_ARG*)(AP_VEC_ELEM_BY_ID(v,i))

/*
    Types
*/
typedef union AP_VALUE AP_VALUE;
union AP_VALUE
{
	uint64_t    u64;
	int64_t     s64;
	double      f64;
	char*       str;
	uint8_t     b : 1;      /* bool, everything other than 0/n/N/f/F will be true */
	uint8_t     nov : 1;    /* noval, parsed arg will flip default value */
};

/* Argument descriptor */
typedef struct AP_ARG_DESC AP_ARG_DESC;
struct AP_ARG_DESC
{
    AP_VALUE value;
    char* name;
    char* description;
    uint32_t name_hash;     /* CRC-32 hash */
    uint8_t type;
};

typedef CVEC AP_ARG_DESC_VEC;

/* Parsed argument */
typedef struct AP_ARG AP_ARG;
struct AP_ARG
{
    AP_VALUE value;
    uint32_t desc_hash;
    uint8_t type;
};

typedef CVEC AP_ARG_VEC;

/* Parser object */
typedef struct AP_DESC AP_DESC;
struct AP_DESC
{
    AP_ARG_DESC_VEC available_args;     /* Vector of AP_ARG_DESC */
    AP_ARG_VEC parsed_args;             /* Vector of AP_ARG */
};

/*
    Functions
*/

/*
    Allocates the parser descriptor.
    
    Returns a pointer to AP_DESC.
*/
AP_DESC* ap_create();

/*
    Frees descriptor's content and its pointer.
    
    Returns NULL.
*/
AP_DESC* ap_free(AP_DESC* ap);

/*
    Appends an argument descriptor.
    
    Returns AP_STAT_SUCCESS on success, AP_STAT_* otherwise.
*/
const uint8_t ap_append_arg_desc(AP_DESC* ap,
                                 const char* name, const char* description,
                                 uint8_t value_type, AP_VALUE default_value);

/*
    Parses arguments and populates ap->parsed_args.
    
    Returns AP_SUCCESS on success, AP_ERROR otherwise.
*/
const uint8_t ap_parse(AP_DESC* ap, int argc, char** argv);

/*
    Appends a parsed argument.
    
    Returns AP_STAT_SUCCESS on success, AP_STAT_* otherwise.
*/
const uint8_t ap_append_arg(AP_DESC* ap, uint32_t desc_hash,
                            uint8_t value_type, AP_VALUE value);

/*
    Functions for appending individual types of argument descriptors.
*/
const uint8_t ap_append_desc_uint(AP_DESC* ap, const uint64_t default_value,
                                  const char* name, const char* description);
const uint8_t ap_append_desc_int(AP_DESC* ap, const int64_t default_value,
                                 const char* name, const char* description);
const uint8_t ap_append_desc_float(AP_DESC* ap, const double default_value,
                                   const char* name, const char* description);
const uint8_t ap_append_desc_str(AP_DESC* ap, const char* default_value,
                                 const char* name, const char* description);
const uint8_t ap_append_desc_bool(AP_DESC* ap, const uint8_t default_value,
                                  const char* name, const char* description);
const uint8_t ap_append_desc_noval(AP_DESC* ap, const uint8_t default_value,
                                   const char* name, const char* description);

/*
    Misc utils
*/
AP_ARG_DESC* ap_get_desc_by_hash(AP_DESC* ap, const uint32_t name_hash);
AP_ARG_VEC ap_get_arg_vec_by_hash(AP_DESC* ap, const uint32_t desc_hash);

static inline AP_ARG_DESC* ap_get_desc_by_name(AP_DESC* ap, const char* name)
{
    const uint32_t hash = crc32_encode((const uint8_t*)name, strlen(name));
    return ap_get_desc_by_hash(ap, hash);
}

static inline AP_ARG_VEC ap_get_arg_vec_by_name(AP_DESC* ap, const char* desc_name)
{
    const uint32_t hash = crc32_encode((const uint8_t*)desc_name, strlen(desc_name));
    return ap_get_arg_vec_by_hash(ap, hash);
}

static inline AP_ARG_DESC* ap_get_desc_by_id(AP_DESC* ap, const uint64_t index)
{
    return AP_DESC_FROM_VEC_BY_ID(ap->available_args, index);
}

static inline AP_ARG* ap_get_arg_by_id(AP_DESC* ap, const uint64_t index)
{
    return AP_ARG_FROM_VEC_BY_ID(ap->parsed_args, index);
}

static inline AP_ARG* ap_get_arg_from_vec_by_id(AP_ARG_VEC args, const uint64_t index)
{
    return AP_ARG_FROM_VEC_BY_ID(args, index);
}

static inline const uint64_t ap_get_desc_count(AP_DESC* ap)
{
    return cvec_size(ap->available_args);
}

static inline const uint64_t ap_get_arg_count(AP_DESC* ap)
{
    return cvec_size(ap->parsed_args);
}

static inline const uint64_t ap_get_arg_vec_count(AP_ARG_VEC args)
{
    return cvec_size(args);
}

static inline AP_ARG_VEC ap_free_arg_vec(AP_ARG_VEC args)
{
    return cvec_destroy(args);
}
