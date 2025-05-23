#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kwaslib/core/io/file_utils.h>

#include "sexml.h"
#include "sexml_defines.h"
#include "sexml_parser.h"
#include "sexml_exporter.h"

SEXML_ELEMENT* sexml_load_from_file(const char* path)
{
    SEXML_ELEMENT* xml = NULL;
    FU_FILE xmlf = {0};
    const uint8_t status = fu_open_file(path, 1, &xmlf);
    
    if(status == FU_SUCCESS)
    {
        xml = sexml_parse_text(xmlf.buf, xmlf.size);
        fu_close(&xmlf);
    }
    
    return xml;
}

SEXML_ELEMENT* sexml_parse_text(const char* text, const uint32_t size)
{
    SEXML_ELEMENT* xml = sexml_alloc_element();
    
    if(xml)
    {
        SEXML_PARSER_CTX ctx = {0};
        ctx.last_error = SEXML_GOOD;
        ctx.text = (char*)text;
        ctx.text_size = size;
        ctx.text_it = 0;

        /* Check for <?xml tag */
        sexml_parser_check_for_prolog(&ctx);

        /* Main loop of the parser */
        while(ctx.text_it != ctx.text_size)
        {
            /*
                Just break out if any error occured.
                The state of the memory XML is unknown.
            */
            if(ctx.last_error != SEXML_GOOD)
            {
                break;
            }
            
            const char cur_char = ctx.text[ctx.text_it];
            
            switch(cur_char)
            {
                case '<': /* Start Tag */
                    if(ctx.text[ctx.text_it+1] != '/')
                    {
                        if(ctx.root_read == 0)
                        {
                            ctx.cur_elem = xml;
                            ctx.root_read = 1;
                        }
                        else
                        {
                            ctx.cur_elem = sexml_append_element(ctx.cur_elem, NULL);
                        }
                    }
                    
                    sexml_parser_read_start_tag(&ctx);
                    
                    break;
                case '>': /* Leaving the tag */
                    if(ctx.closing_tag)
                    {
                        ctx.cur_elem = ctx.cur_elem->parent;
                        ctx.closing_tag = 0;
                    }
                    
                    ctx.inside_tag = 0;
                    ctx.text_it += 1;
                    break;
                case '/': /* We're leaving the self-closing tag */
                    ctx.inside_tag = 0;
                    ctx.text_it += 1;
                    
                    switch(ctx.text[ctx.text_it])
                    {
                        case '>':
                            ctx.text_it += 1;
                            ctx.cur_elem = ctx.cur_elem->parent;
                            break;
                        default: /* Space after the end tag slash is invalid */
                            ctx.last_error = SEXML_ERR_SPACE_END;
                    }
                    
                    break;
                case ' ':   /* Characters to skip */
                case '\n':
                case '\r':
                    ctx.text_it += 1;
                    break;
                case '\0': /* End of file. Could be premature. */
                    printf("NULL spotted. Terminating parser.\n");
                    ctx.text_it = size;
                    break;
                default:
                    /*
                        Reading attributes.
                        Otherwise read text.
                    */
                    if(ctx.inside_tag)
                    {
                        if(ctx.closing_tag)
                        {
                            ctx.last_error = SEXML_ERR_ATTR_CLOSING;
                        }
                        else
                        {
                            sexml_parser_read_attribute(&ctx);
                        }
                    }
                    else
                    {
                        sexml_parser_read_text(&ctx);
                    }
            }
        }
        
        if(ctx.last_error != SEXML_GOOD)
        {
            printf("Error occured at %llu | %u\n", ctx.text_it, ctx.last_error);
        }
    }
    
    return xml;
}

void sexml_save_to_file(const char* path, SEXML_ELEMENT* xml)
{
    sexml_exporter_save_to_file(path, xml);
}

void sexml_save_to_file_formatted(const char* path, SEXML_ELEMENT* xml, const uint32_t indent)
{
    sexml_exporter_save_to_file_formatted(path, xml, indent);
}

SEXML_ELEMENT* sexml_create_root(const char* name)
{
    SEXML_ELEMENT* xml = sexml_alloc_element();
    sexml_set_element_name(xml, name);
    return xml;
}

SEXML_ELEMENT* sexml_alloc_element()
{
    SEXML_ELEMENT* elem = (SEXML_ELEMENT*)calloc(1, sizeof(SEXML_ELEMENT));
    sexml_alloc_element_fields(elem);
    return elem;
}

void sexml_alloc_element_fields(SEXML_ELEMENT* element)
{
    element->parent = NULL;
    element->name = su_create_string("", 0);
    element->text = su_create_string("", 0);
    element->elements = cvec_create(sizeof(SEXML_ELEMENT*));
    element->attributes = cvec_create(sizeof(SEXML_ATTRIBUTE*));
}

SEXML_ATTRIBUTE* sexml_alloc_attribute()
{
    return (SEXML_ATTRIBUTE*)calloc(1, sizeof(SEXML_ATTRIBUTE));
}

SEXML_ELEMENT* sexml_destroy(SEXML_ELEMENT* root)
{
    return sexml_destroy_element(root);
}

SEXML_ELEMENT* sexml_destroy_element(SEXML_ELEMENT* element)
{    
    /* Freeing strings */
    element->text = su_free(element->text);
    element->name = su_free(element->name);

    /* Freeing attributes */
    for(uint64_t i = 0; i != cvec_size(element->attributes); ++i)
    {
        SEXML_ATTRIBUTE* attrib = sexml_get_attribute_by_id(element, i);
        attrib->name = su_free(attrib->name);
        attrib->value = su_free(attrib->value);
        free(attrib);
    }
    
    element->attributes = cvec_destroy(element->attributes);
    
    /* Freeing elements */
    for(uint64_t i = 0; i != cvec_size(element->elements); ++i)
    {
        SEXML_ELEMENT* cur_elem = sexml_get_element_by_id(element, i);
        sexml_destroy_element(cur_elem);
    }
    
    element->elements = cvec_destroy(element->elements);
    
    /* Final cleanup in element */
    free(element);
    
    return NULL;
}
