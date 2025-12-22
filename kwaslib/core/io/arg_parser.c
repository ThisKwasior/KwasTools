#include "arg_parser.h"

#include <stdlib.h>

AP_DESC* ap_create()
{
    AP_DESC* desc = (AP_DESC*)calloc(1, sizeof(AP_DESC));
    desc->available_args = cvec_create(sizeof(AP_ARG_DESC));
    desc->parsed_args = cvec_create(sizeof(AP_ARG));
    return desc;
}

AP_DESC* ap_free(AP_DESC* ap)
{
    if(ap)
    {
        const uint64_t desc_count = ap_get_desc_count(ap);
        const uint64_t arg_count = ap_get_arg_count(ap);
        
        /* Argument descriptors first */
        for(uint64_t i = 0; i != desc_count; ++i)
        {
            AP_ARG_DESC* desc = ap_get_desc_by_id(ap, i);
            
            free(desc->name);
            free(desc->description);
            
            if(desc->type == AP_TYPE_STR)
            {
                free(desc->value.str);
            }
        }
        
        /* Parsed arguments second */
        for(uint64_t i = 0; i != arg_count; ++i)
        {
            AP_ARG* arg = ap_get_arg_by_id(ap, i);

            if(arg->type == AP_TYPE_STR)
            {
                free(arg->value.str);
            }
        }
        
        /* Vectors third */
        ap->available_args = cvec_destroy(ap->available_args);
        ap->parsed_args = cvec_destroy(ap->parsed_args);
        
        /* Descriptor fourth */
        free(ap);
    }
    
    return NULL;
}

const uint8_t ap_append_arg_desc(AP_DESC* ap,
                                 const char* name, const char* description,
                                 uint8_t value_type, AP_VALUE default_value)
{
    AP_ARG_DESC desc = {0};
    
    switch(value_type)
    {
        case AP_TYPE_NOV:
        case AP_TYPE_U64:
        case AP_TYPE_S64:
        case AP_TYPE_F64:
        case AP_TYPE_STR:
        case AP_TYPE_BOOL:
            desc.type = value_type;
            desc.value.u64 = default_value.u64;
            break;
        default:
            return AP_STAT_UNK_TYPE;
    }
    
    const uint64_t name_len = strlen(name);
    const uint64_t description_len = strlen(description);
    desc.name = (char*)calloc(1, name_len+1);
    desc.description = (char*)calloc(1, description_len+1);
    memcpy(desc.name, name, name_len);
    memcpy(desc.description, description, description_len);
    
    desc.name_hash = crc32_encode((const uint8_t*)name, name_len);
    
    cvec_push_back(ap->available_args, &desc);
    
    return AP_STAT_SUCCESS;
}

const uint8_t ap_parse(AP_DESC* ap, int argc, char** argv)
{
    if(argc == 0)
    {
        return AP_STAT_ARGC_0;
    }
    
    for(int i = 0; i != argc; ++i)
    {
        const uint64_t len = strlen(argv[i]);
        const uint32_t hash = crc32_encode((const uint8_t*)argv[i], len);
        
        AP_ARG_DESC* desc = ap_get_desc_by_hash(ap, hash);
        
        if(desc)
        {
            const uint8_t is_nov = (desc->type == AP_TYPE_NOV);
            const int value_index = i+1;
            char* value_str = argv[value_index];
            uint64_t value_len = 0;
            char* endptr = NULL;
            AP_VALUE val = {0};

            if(is_nov == 0)
            {
                if(value_index >= argc)
                {
                    return AP_STAT_ARG_NO_VAL;
                }
                
                value_len = strlen(value_str);
            }
            
            switch(desc->type)
            {
                case AP_TYPE_U64:
                    val.u64 = strtoull(value_str, &endptr, 10);
                    break;
                case AP_TYPE_S64:
                    val.s64 = strtoll(value_str, &endptr, 10);
                    break;
                case AP_TYPE_F64:
                    val.f64 = strtod(value_str, &endptr);
                    break;
                case AP_TYPE_STR:
                    val.str = (char*)calloc(1, value_len+1);
                    memcpy(val.str, value_str, value_len);
                    break;
                case AP_TYPE_BOOL:
                    switch(value_str[0])
                    {
                        case '0':
                        case 'n':
                        case 'N':
                        case 'f':
                        case 'F':
                            val.b = 0;
                            break;
                        default:
                            val.b = 1;
                    }
                    break;
                case AP_TYPE_NOV:
                    val.nov = (!desc->value.nov);
                    break;
                default:
                    return AP_STAT_UNK_TYPE;
            }
            
            ap_append_arg(ap, desc->name_hash, desc->type, val);
        }
        else
        {
            continue;
        }
    }
    
    return AP_STAT_SUCCESS;
}

const uint8_t ap_append_arg(AP_DESC* ap, uint32_t desc_hash,
                            uint8_t value_type, AP_VALUE value)
{
    AP_ARG arg = {0};

    switch(value_type)
    {
        case AP_TYPE_NOV:
        case AP_TYPE_U64:
        case AP_TYPE_S64:
        case AP_TYPE_F64:
        case AP_TYPE_STR:
        case AP_TYPE_BOOL:
            arg.value.u64 = value.u64;
            arg.desc_hash = desc_hash;
            arg.type = value_type;
            break;
        default:
            return AP_STAT_UNK_TYPE;
    }

    cvec_push_back(ap->parsed_args, &arg);

    return AP_STAT_SUCCESS;
}

/*
    Functions for appending individual types of argument descriptors.
*/
const uint8_t ap_append_desc_uint(AP_DESC* ap, const uint64_t default_value,
                                  const char* name, const char* description)
{
    AP_VALUE def;
    def.u64 = default_value;
    return ap_append_arg_desc(ap, name, description, AP_TYPE_U64, def);
}
                                      
const uint8_t ap_append_desc_int(AP_DESC* ap, const int64_t default_value,
                                 const char* name, const char* description)
{
    AP_VALUE def;
    def.s64 = default_value;
    return ap_append_arg_desc(ap, name, description, AP_TYPE_S64, def);
}
                                     
const uint8_t ap_append_desc_float(AP_DESC* ap, const double default_value,
                                   const char* name, const char* description)
{
    AP_VALUE def;
    def.f64 = default_value;
    return ap_append_arg_desc(ap, name, description, AP_TYPE_F64, def);
}
                                       
const uint8_t ap_append_desc_str(AP_DESC* ap, const char* default_value,
                                 const char* name, const char* description)
{
    AP_VALUE def;
    const uint64_t len = strlen(default_value);
    def.str = (char*)calloc(1, len+1);
    memcpy(def.str, default_value, len);
    const uint8_t stat = ap_append_arg_desc(ap, name, description, AP_TYPE_STR, def);
    
    if(stat != AP_STAT_SUCCESS)
    {
        free(def.str);
    }
    
    return stat;
}
                                     
const uint8_t ap_append_desc_bool(AP_DESC* ap, const uint8_t default_value,
                                      const char* name, const char* description)
{
    AP_VALUE def;
    def.b = default_value;
    return ap_append_arg_desc(ap, name, description, AP_TYPE_BOOL, def);
}

const uint8_t ap_append_desc_noval(AP_DESC* ap, const uint8_t default_value,
                                       const char* name, const char* description)
{
    AP_VALUE def;
    def.nov = default_value;
    return ap_append_arg_desc(ap, name, description, AP_TYPE_NOV, def);
}

/*
    Misc utils
*/
AP_ARG_DESC* ap_get_desc_by_hash(AP_DESC* ap, const uint32_t name_hash)
{
    const uint64_t desc_count = ap_get_desc_count(ap);
    
    /* Argument descriptors first */
    for(uint64_t i = 0; i != desc_count; ++i)
    {
        AP_ARG_DESC* desc = ap_get_desc_by_id(ap, i);
        
        if(desc->name_hash == name_hash)
        {
            return desc;
        }
    }
    
    return NULL;
}

AP_ARG_VEC ap_get_arg_vec_by_hash(AP_DESC* ap, const uint32_t desc_hash)
{
    AP_ARG_DESC* desc = ap_get_desc_by_hash(ap, desc_hash);
    
    if(desc == NULL)
    {
        return NULL;
    }
    
    AP_ARG_VEC args = cvec_create(sizeof(AP_ARG));
    
    const uint64_t arg_count = ap_get_arg_count(ap);

    for(uint64_t i = 0; i != arg_count; ++i)
    {
        AP_ARG* arg = ap_get_arg_by_id(ap, i);
        
        if(arg->desc_hash == desc_hash)
        {
            cvec_push_back(args, arg);
        }
    }
    
    if(ap_get_arg_vec_count(args) == 0)
    {
        args = ap_free_arg_vec(args);
    }
    
    return args;
}
