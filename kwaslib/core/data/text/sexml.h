#pragma once

#include <stdint.h>

#include "sexml_defines.h"
#include "sexml_io.h"

/* Implementation */
SEXML_ELEMENT* sexml_load_from_file(const char* path);
void sexml_save_to_file(const char* path, SEXML_ELEMENT* xml);
void sexml_save_to_file_formatted(const char* path, SEXML_ELEMENT* xml, const uint32_t indent);

SEXML_ELEMENT* sexml_parse_text(const char* text, const uint32_t size);

SEXML_ELEMENT* sexml_create_root(const char* name);
SEXML_ELEMENT* sexml_alloc_element();
void sexml_alloc_element_fields(SEXML_ELEMENT* element);

SEXML_ATTRIBUTE* sexml_alloc_attribute();

SEXML_ELEMENT* sexml_destroy(SEXML_ELEMENT* root);
SEXML_ELEMENT* sexml_destroy_element(SEXML_ELEMENT* element);