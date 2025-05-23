#pragma once

#include <stdint.h>

#include "sexml.h"
#include "sexml_defines.h"

void sexml_text_to_entity_references(SU_STRING* text);
void sexml_text_from_entity_references(SU_STRING* text);

const uint64_t sexml_get_element_depth(SEXML_ELEMENT* element);
const uint64_t sexml_get_child_count(SEXML_ELEMENT* element, const char* name);
const uint8_t sexml_does_child_exists(SEXML_ELEMENT* element, const char* name);

/*
    Getters by id
*/
SEXML_ELEMENT* sexml_get_element_by_id(SEXML_ELEMENT* element, const uint64_t id);
SEXML_ATTRIBUTE* sexml_get_attribute_by_id(SEXML_ELEMENT* element, const uint64_t id);
const uint8_t sexml_get_attribute_bool_by_id(SEXML_ELEMENT* element, const uint64_t id);
const uint64_t sexml_get_attribute_uint_by_id(SEXML_ELEMENT* element, const uint64_t id);
const int64_t sexml_get_attribute_int_by_id(SEXML_ELEMENT* element, const uint64_t id);
const double sexml_get_attribute_double_by_id(SEXML_ELEMENT* element, const uint64_t id);
SU_STRING* sexml_get_attribute_vl_by_id(SEXML_ELEMENT* element, const uint64_t id);

/*
    Getters by name
*/
SEXML_ELEMENT* sexml_get_element_by_name(SEXML_ELEMENT* element, const char* name);
SEXML_ATTRIBUTE* sexml_get_attribute_by_name(SEXML_ELEMENT* element, const char* name);
const uint8_t sexml_get_attribute_bool_by_name(SEXML_ELEMENT* element, const char* name);
const uint64_t sexml_get_attribute_uint_by_name(SEXML_ELEMENT* element, const char* name);
const int64_t sexml_get_attribute_int_by_name(SEXML_ELEMENT* element, const char* name);
const double sexml_get_attribute_double_by_name(SEXML_ELEMENT* element, const char* name);
SU_STRING* sexml_get_attribute_vl_by_name(SEXML_ELEMENT* element, const char* name);

/*
    Removers
*/
void sexml_remove_element_by_id(SEXML_ELEMENT* element, const uint64_t id);
void sexml_remove_attribute_by_id(SEXML_ELEMENT* element, const uint64_t id);

/*
    Setters
*/
void sexml_set_element_name(SEXML_ELEMENT* element, const char* name);
void sexml_set_element_text(SEXML_ELEMENT* element, const char* text);
void sexml_set_element_text_bool(SEXML_ELEMENT* element, const uint8_t value);
void sexml_set_element_text_uint(SEXML_ELEMENT* element, const uint64_t value);
void sexml_set_element_text_int(SEXML_ELEMENT* element, const int64_t value);
void sexml_set_element_text_double(SEXML_ELEMENT* element, const double value, const uint8_t precision);
void sexml_set_element_text_vl(SEXML_ELEMENT* element, const char* data, const uint32_t size);

void sexml_set_attribute_name(SEXML_ATTRIBUTE* attribute, const char* name);
void sexml_set_attribute_value(SEXML_ATTRIBUTE* attribute, const char* value);
void sexml_set_attribute_value_bool(SEXML_ATTRIBUTE* attribute, const uint8_t value);
void sexml_set_attribute_value_uint(SEXML_ATTRIBUTE* attribute, const uint64_t value);
void sexml_set_attribute_value_int(SEXML_ATTRIBUTE* attribute, const int64_t value);
void sexml_set_attribute_value_double(SEXML_ATTRIBUTE* attribute, const double value, const uint8_t precision);
void sexml_set_attribute_value_vl(SEXML_ATTRIBUTE* attribute, const char* data, const uint32_t size);

/*
    Appends element and allocates all fields to default values.
    `name` can be NULL, though not recommended.
    
    Returns a pointer to the allocated element.
*/
SEXML_ELEMENT* sexml_append_element(SEXML_ELEMENT* element, const char* name);

/*
    Appends an attribute.
    `name` is mandatory, `value` is optional.
    
    Returns a pointer to the allocated attribute.
*/
SEXML_ATTRIBUTE* sexml_append_attribute(SEXML_ELEMENT* element, const char* name, const char* value);

/* Anything other than 0 will be true */
SEXML_ATTRIBUTE* sexml_append_attribute_bool(SEXML_ELEMENT* element, const char* name, const uint8_t value);
SEXML_ATTRIBUTE* sexml_append_attribute_uint(SEXML_ELEMENT* element, const char* name, const uint64_t value);
SEXML_ATTRIBUTE* sexml_append_attribute_int(SEXML_ELEMENT* element, const char* name, const int64_t value);
SEXML_ATTRIBUTE* sexml_append_attribute_double(SEXML_ELEMENT* element, const char* name, const double value, const uint8_t precision);
SEXML_ATTRIBUTE* sexml_append_attribute_vl(SEXML_ELEMENT* element, const char* name, const char* data, const uint32_t size);

/*
    Attribute converters
*/

/* Checks for 1TtYy as first character */
const uint8_t sexml_get_attribute_bool(SEXML_ATTRIBUTE* attribute);
const uint64_t sexml_get_attribute_uint(SEXML_ATTRIBUTE* attribute);
const int64_t sexml_get_attribute_int(SEXML_ATTRIBUTE* attribute);
const double sexml_get_attribute_double(SEXML_ATTRIBUTE* attribute);
SU_STRING* sexml_get_attribute_vl(SEXML_ATTRIBUTE* attribute);