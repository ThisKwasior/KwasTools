#pragma once

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

#define SEXML_GOOD              3   /* All good (default) */
#define SEXML_ERROR             4   /* Generic error */
#define SEXML_ERR_END_NO_MATCH  5   /* Start Tag couldn't be matched with End Tag */
#define SEXML_ERR_SPACE_START   6   /* Space after Start Tag and before the name */
#define SEXML_ERR_SPACE_END     7   /* Space after End Tag slash and before End Tag */
#define SEXML_ERR_MALFORMED_XML 8   /* XML doc is not written properly. Oops. */
#define SEXML_ERR_ATTR_MALFORM  9   /* Attribute inside of a Tag is malformed */
#define SEXML_ERR_ATTR_CLOSING  10  /* Attribute in closing tag */
#define SEXML_ERR_ATTR_NOSPACE  11  /* No space or end tag after the attribute value */

#define SEXML_ENT_REF_LT        (const char)'<'
#define SEXML_ENT_REF_LT_STR    (const char*)"&lt;"
#define SEXML_ENT_REF_LT_SIZE   4

#define SEXML_ENT_REF_GT        (const char)'>'
#define SEXML_ENT_REF_GT_STR    (const char*)"&gt;"
#define SEXML_ENT_REF_GT_SIZE   4

#define SEXML_ENT_REF_AMP       (const char)'&'
#define SEXML_ENT_REF_AMP_STR   (const char*)"&amp;"
#define SEXML_ENT_REF_AMP_SIZE  5

#define SEXML_ENT_REF_APOS      (const char)'\''
#define SEXML_ENT_REF_APOS_STR  (const char*)"&apos;"
#define SEXML_ENT_REF_APOS_SIZE 6

#define SEXML_ENT_REF_QUOT      (const char)'\"'
#define SEXML_ENT_REF_QUOT_STR  (const char*)"&quot;"
#define SEXML_ENT_REF_QUOT_SIZE 6

typedef struct SEXML_ELEMENT SEXML_ELEMENT;

struct SEXML_ELEMENT
{
    SEXML_ELEMENT* parent;
    
    SU_STRING* name;
    SU_STRING* text;
    
    CVEC attributes;
    CVEC elements;
};

typedef struct
{
    SU_STRING* name;
    SU_STRING* value;
} SEXML_ATTRIBUTE;

typedef struct SEXML_PARSER_CONTEXT
{
    uint64_t text_it;
    uint64_t text_size;
    char* text;
    SEXML_ELEMENT* cur_elem;
    uint8_t inside_tag;
    uint8_t self_closing_tag;
    uint8_t closing_tag;
    uint8_t last_error;
    uint8_t root_read;
} SEXML_PARSER_CTX;