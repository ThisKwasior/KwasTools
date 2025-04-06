#pragma once

#include <stdint.h>

#include <kwaslib/core/io/string_utils.h>

#include "sexml_defines.h"

/*
    Implementation
*/

/*
    Reads the name of the tag and checks if it's the closing tag
*/
void sexml_parser_read_start_tag(SEXML_PARSER_CTX* ctx);

/*
    Gets the name of a tag or parameter 
*/
SU_STRING* sexml_parser_get_name(SEXML_PARSER_CTX* ctx);

/*
    Reads an attribute
*/
void sexml_parser_read_attribute(SEXML_PARSER_CTX* ctx);
SU_STRING* sexml_parser_read_attribute_value(SEXML_PARSER_CTX* ctx);

/*
    Appends the text between tags to current element
*/
void sexml_parser_read_text(SEXML_PARSER_CTX* ctx);

/*
    Skipping <?xml
*/
void sexml_parser_check_for_prolog(SEXML_PARSER_CTX* ctx);