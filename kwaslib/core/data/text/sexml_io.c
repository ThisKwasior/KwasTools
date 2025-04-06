#include "sexml_io.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void sexml_text_to_entity_references(SU_STRING* text)
{
    uint64_t it = 0;
    
    while(it != text->size)
    {
        switch(text->ptr[it])
        {
            case SEXML_ENT_REF_LT:
                su_remove(text, it, 1);
                su_insert_char(text, it, SEXML_ENT_REF_LT_STR, SEXML_ENT_REF_LT_SIZE);
                it += SEXML_ENT_REF_LT_SIZE;
                break;
            case SEXML_ENT_REF_GT:
                su_remove(text, it, 1);
                su_insert_char(text, it, SEXML_ENT_REF_GT_STR, SEXML_ENT_REF_GT_SIZE);
                it += SEXML_ENT_REF_GT_SIZE;
                break;
            case SEXML_ENT_REF_AMP:
                su_remove(text, it, 1);
                su_insert_char(text, it, SEXML_ENT_REF_AMP_STR, SEXML_ENT_REF_AMP_SIZE);
                it += SEXML_ENT_REF_AMP_SIZE;
                break;
            case SEXML_ENT_REF_APOS:
                su_remove(text, it, 1);
                su_insert_char(text, it, SEXML_ENT_REF_APOS_STR, SEXML_ENT_REF_APOS_SIZE);
                it += SEXML_ENT_REF_APOS_SIZE;
                break;
            case SEXML_ENT_REF_QUOT:
                su_remove(text, it, 1);
                su_insert_char(text, it, SEXML_ENT_REF_QUOT_STR, SEXML_ENT_REF_QUOT_SIZE);
                it += SEXML_ENT_REF_QUOT_SIZE;
                break;
            default:
                it += 1;
        }
    }
}

void sexml_text_from_entity_references(SU_STRING* text)
{
    uint64_t it = 0;
    
    while(it != text->size)
    {
        if(text->ptr[it] == '&')
        {
            const uint8_t lt = strncmp(&text->ptr[it], SEXML_ENT_REF_LT_STR, SEXML_ENT_REF_LT_SIZE);
            const uint8_t gt = strncmp(&text->ptr[it], SEXML_ENT_REF_GT_STR, SEXML_ENT_REF_GT_SIZE);
            const uint8_t am = strncmp(&text->ptr[it], SEXML_ENT_REF_AMP_STR, SEXML_ENT_REF_AMP_SIZE);
            const uint8_t ap = strncmp(&text->ptr[it], SEXML_ENT_REF_APOS_STR, SEXML_ENT_REF_APOS_SIZE);
            const uint8_t qu = strncmp(&text->ptr[it], SEXML_ENT_REF_QUOT_STR, SEXML_ENT_REF_QUOT_SIZE);

            if(lt == 0)
            {
                su_remove(text, it, SEXML_ENT_REF_LT_SIZE);
                su_insert_char(text, it, "<", 1);
            }
            else if(gt == 0)
            {
                su_remove(text, it, SEXML_ENT_REF_GT_SIZE);
                su_insert_char(text, it, ">", 1);
            }
            else if(am == 0)
            {
                su_remove(text, it, SEXML_ENT_REF_AMP_SIZE);
                su_insert_char(text, it, "&", 1);
            }
            else if(ap == 0)
            {
                su_remove(text, it, SEXML_ENT_REF_APOS_SIZE);
                su_insert_char(text, it, "\'", 1);
            }
            else if(qu == 0)
            {
                su_remove(text, it, SEXML_ENT_REF_QUOT_SIZE);
                su_insert_char(text, it, "\"", 1);
            }
        }
        
        it += 1;
    }
}

const uint64_t sexml_get_element_depth(SEXML_ELEMENT* xml)
{
    uint64_t depth = 0;
    
    while(xml->parent != NULL)
    {
        depth += 1;
        xml = xml->parent;
    }
    
    return depth;
}

const uint64_t sexml_get_child_count(SEXML_ELEMENT* element, const char* name)
{
    if(element == NULL) return 0;
    
    const uint64_t name_len = strlen(name);
    uint64_t count = 0;
    
    for(uint64_t i = 0; i != cvec_size(element->elements); ++i)
    {
        SEXML_ELEMENT* elem = sexml_get_element_by_id(element, i);
        
        if(elem->name->size == name_len)
        {
            if(strncmp(elem->name->ptr, name, name_len) == 0)
            {
                count += 1;
            }
        }
    }
    
    return count;
}

const uint8_t sexml_does_child_exists(SEXML_ELEMENT* element, const char* name)
{
    if(element == NULL) return 0;
    
    const uint64_t name_len = strlen(name);
    uint8_t exists = 0;
    
    for(uint64_t i = 0; i != cvec_size(element->elements); ++i)
    {
        SEXML_ELEMENT* elem = sexml_get_element_by_id(element, i);
        
        if(elem->name->size == name_len)
        {
            if(strncmp(elem->name->ptr, name, name_len) == 0)
            {
                exists = 1;
                break;
            }
        }
    }
    
    return exists;
}

SEXML_ELEMENT* sexml_get_element_by_id(SEXML_ELEMENT* element, const uint64_t id)
{
    return *(SEXML_ELEMENT**)cvec_at(element->elements, id);
}

SEXML_ATTRIBUTE* sexml_get_attribute_by_id(SEXML_ELEMENT* element, const uint64_t id)
{
    return *(SEXML_ATTRIBUTE**)cvec_at(element->attributes, id);
}

SEXML_ELEMENT* sexml_get_element_by_name(SEXML_ELEMENT* element, const char* name)
{
    if(element == NULL) return NULL;
    
    const uint64_t name_len = strlen(name);
    SEXML_ELEMENT* elem = NULL;
    
    for(uint64_t i = 0; i != cvec_size(element->elements); ++i)
    {
        elem = sexml_get_element_by_id(element, i);
        
        if(elem->name->size == name_len)
        {
            if(strncmp(elem->name->ptr, name, name_len) == 0)
            {
                break;
            }
        }
    }
    
    return elem;
}

SEXML_ATTRIBUTE* sexml_get_attribute_by_name(SEXML_ELEMENT* element, const char* name)
{
    if(element == NULL) return NULL;
    
    const uint64_t name_len = strlen(name);
    SEXML_ATTRIBUTE* attrib = NULL;
    
    for(uint64_t i = 0; i != cvec_size(element->attributes); ++i)
    {
        attrib = sexml_get_attribute_by_id(element, i);
        
        if(attrib->name->size == name_len)
        {
            if(strncmp(attrib->name->ptr, name, name_len) == 0)
            {
                break;
            }
        }
    }
    
    return attrib;
}

void sexml_remove_element_by_id(SEXML_ELEMENT* element, const uint64_t id)
{
    SEXML_ELEMENT* elem = sexml_get_element_by_id(element, id);
    
    if(elem)
    {
        sexml_destroy_element(elem);
        cvec_erase(element->elements, id);
    }
}

void sexml_remove_attribute_by_id(SEXML_ELEMENT* element, const uint64_t id)
{
    SEXML_ATTRIBUTE* attrib = sexml_get_attribute_by_id(element, id);
    
    if(attrib)
    {
        attrib->name = su_free(attrib->name);
        attrib->value = su_free(attrib->value);
        free(attrib);
        cvec_erase(element->attributes, id);
    }
}

void sexml_set_element_name(SEXML_ELEMENT* element, const char* name)
{
    if(name)
    {
        const uint64_t name_len = strlen(name);
        element->name = su_free(element->name);
        element->name = su_create_string(name, name_len);
    }
}

void sexml_set_element_text(SEXML_ELEMENT* element, const char* text)
{
    if(text)
    {
        const uint64_t name_len = strlen(text);
        element->text = su_free(element->text);
        element->text = su_create_string(text, name_len);
    }
}

void sexml_set_element_text_bool(SEXML_ELEMENT* element, const uint8_t value)
{
    char value_str[2] = {0};
    if(value) value_str[0] = '1';
    else value_str[0] = '0';
    sexml_set_element_text(element, value_str);
}

void sexml_set_element_text_uint(SEXML_ELEMENT* element, const uint64_t value)
{
    char value_str[21] = {0};
    snprintf(value_str, 21, "%llu", value);
    sexml_set_element_text(element, value_str);
}

void sexml_set_element_text_int(SEXML_ELEMENT* element, const int64_t value)
{
    char value_str[21] = {0};
    snprintf(value_str, 21, "%lld", value);
    sexml_set_element_text(element, value_str);
}

void sexml_set_element_text_double(SEXML_ELEMENT* element, const double value, const uint8_t precision)
{
    char value_str[64] = {0};
    snprintf(value_str, 64, "%.*lf", precision, value);
    sexml_set_element_text(element, value_str);
}

SEXML_ELEMENT* sexml_append_element(SEXML_ELEMENT* element, const char* name)
{
    SEXML_ELEMENT* elem = sexml_alloc_element();
    sexml_alloc_element_fields(elem);
    
    elem->parent = element;
    
    if(name != NULL)
    {
        su_insert_char(elem->name, 0, name, strlen(name));
    }
    
    cvec_push_back(element->elements, &elem);
    
    return elem;
}

void sexml_set_attribute_name(SEXML_ATTRIBUTE* attribute, const char* name)
{
    if(name)
    {
        const uint64_t name_len = strlen(name);
        attribute->name = su_free(attribute->name);
        attribute->name = su_create_string(name, name_len);
    }
}

void sexml_set_attribute_value(SEXML_ATTRIBUTE* attribute, const char* value)
{
    if(value)
    {
        const uint64_t value_len = strlen(value);
        attribute->value = su_free(attribute->value);
        attribute->value = su_create_string(value, value_len);
    }
}

void sexml_set_attribute_value_bool(SEXML_ATTRIBUTE* attribute, const uint8_t value)
{
    char value_str[2] = {0};
    if(value) value_str[0] = '1';
    else value_str[0] = '0';
    sexml_set_attribute_value(attribute, value_str);
}

void sexml_set_attribute_value_uint(SEXML_ATTRIBUTE* attribute, const uint64_t value)
{
    char value_str[21] = {0};
    snprintf(value_str, 21, "%llu", value);
    sexml_set_attribute_value(attribute, value_str);
}

void sexml_set_attribute_value_int(SEXML_ATTRIBUTE* attribute, const int64_t value)
{
    char value_str[21] = {0};
    snprintf(value_str, 21, "%lld", value);
    sexml_set_attribute_value(attribute, value_str);
}

void sexml_set_attribute_value_double(SEXML_ATTRIBUTE* attribute, const double value, const uint8_t precision)
{
    char value_str[64] = {0};
    snprintf(value_str, 64, "%.*lf", precision, value);
    sexml_set_attribute_value(attribute, value_str);
}

SEXML_ATTRIBUTE* sexml_append_attribute(SEXML_ELEMENT* element, const char* name, const char* value)
{
    SEXML_ATTRIBUTE* attr = sexml_alloc_attribute();
    
    if(name == NULL)
    {
        free(attr);
        return NULL;
    }
    
    attr->name = su_create_string(name, strlen(name));
    
    if(value != NULL)
    {
        attr->value = su_create_string(value, strlen(value));
    }
    else
    {
        attr->value = su_create_string("", 0);
    }
    
    cvec_push_back(element->attributes, &attr);
    
    return attr;
}

SEXML_ATTRIBUTE* sexml_append_attribute_bool(SEXML_ELEMENT* element, const char* name, const uint8_t value)
{
    char value_str[2] = {0};
    if(value) value_str[0] = '1';
    else value_str[0] = '0';
    return sexml_append_attribute(element, name, value_str);
}

SEXML_ATTRIBUTE* sexml_append_attribute_uint(SEXML_ELEMENT* element, const char* name, const uint64_t value)
{
    char value_str[21] = {0};
    snprintf(value_str, 21, "%llu", value);
    return sexml_append_attribute(element, name, value_str);
}

SEXML_ATTRIBUTE* sexml_append_attribute_int(SEXML_ELEMENT* element, const char* name, const int64_t value)
{
    char value_str[21] = {0};
    snprintf(value_str, 21, "%lld", value);
    return sexml_append_attribute(element, name, value_str);
}

SEXML_ATTRIBUTE* sexml_append_attribute_double(SEXML_ELEMENT* element, const char* name, const double value, const uint8_t precision)
{
    char value_str[64] = {0};
    snprintf(value_str, 64, "%.*lf", precision, value);
    return sexml_append_attribute(element, name, value_str);
}

const uint8_t sexml_get_attribute_bool(SEXML_ATTRIBUTE* attribute)
{
    switch(attribute->value->ptr[0])
    {
        case '1':
        case 'T':
        case 't':
        case 'Y':
        case 'y':
            return 1;
    }
    
    return 0;
}

const uint64_t sexml_get_attribute_uint(SEXML_ATTRIBUTE* attribute)
{
    uint64_t value = 0;
    sscanf(attribute->value->ptr, "%llu", &value);
    return value;
}

const int64_t sexml_get_attribute_int(SEXML_ATTRIBUTE* attribute)
{
    int64_t value = 0;
    sscanf(attribute->value->ptr, "%lld", &value);
    return value;
}

const double sexml_get_attribute_double(SEXML_ATTRIBUTE* attribute)
{
    double value = 0;
    sscanf(attribute->value->ptr, "%lf", &value);
    return value;
}