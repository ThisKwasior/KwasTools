#pragma once

#include <stdio.h>

#include "sexml_defines.h"

/*
    Implementation
*/

void sexml_exporter_save_to_file(const char* path, SEXML_ELEMENT* xml);
void sexml_exporter_save_to_file_formatted(const char* path, SEXML_ELEMENT* xml, const uint32_t indent);

void sexml_exporter_write_element(FILE* f, SEXML_ELEMENT* xml);
void sexml_exporter_write_element_formatted(FILE* f, SEXML_ELEMENT* xml, const uint32_t indent);