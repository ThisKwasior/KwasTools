#include "sexml_exporter.h"

#include <stdint.h>

#include "sexml.h"

#define WRITE_DEPTH(file,x,indent)              \
    for(uint32_t i = 0; i !=(x) ;++i)           \
        for(uint32_t j = 0; j != (indent); ++j) \
            fprintf((file)," ");

/*
    Implementation
*/

void sexml_exporter_save_to_file(const char* path, SEXML_ELEMENT* xml)
{
    FILE* fout = fopen(path, "wb");
    sexml_exporter_write_element(fout, xml);
    fclose(fout);
}

void sexml_exporter_save_to_file_formatted(const char* path, SEXML_ELEMENT* xml, const uint32_t indent)
{
    FILE* fout = fopen(path, "wb");
    sexml_exporter_write_element_formatted(fout, xml, indent);
    fclose(fout);
}

void sexml_exporter_write_element(FILE* f, SEXML_ELEMENT* xml)
{
    uint8_t write_elements = 0;
    uint8_t write_text = 0;

    fprintf(f, "<%*s", xml->name->size, xml->name->ptr);
    
    for(uint64_t i = 0; i != cvec_size(xml->attributes); ++i)
    {
        SEXML_ATTRIBUTE* attr = sexml_get_attribute_by_id(xml, i);
        sexml_text_to_entity_references(attr->value);
        fprintf(f, " %*s=\"%*s\"", attr->name->size, attr->name->ptr,
                                   attr->value->size, attr->value->ptr);
        sexml_text_from_entity_references(attr->value);
    }
    
    if(!cvec_empty(xml->elements))
    {
        write_elements = 1;
    }
    
    if(xml->text->size != 0)
    {
        write_text = 1;
    }
    
    if((write_elements == 0) && (write_text == 0))
    {
        fprintf(f, "/");
    }

    fprintf(f, ">");
    
    if(write_text && (write_elements == 0))
    {
        sexml_text_to_entity_references(xml->text);
        fprintf(f, "%*s", xml->text->size, xml->text->ptr);
        sexml_text_from_entity_references(xml->text);
        fprintf(f, "</%*s>", xml->name->size, xml->name->ptr);
    }
    
    if(write_text && write_elements)
    {
        sexml_text_to_entity_references(xml->text);
        fprintf(f, "%*s", xml->text->size, xml->text->ptr);
        sexml_text_from_entity_references(xml->text);
    }
    
    if(write_elements)
    {
        for(uint64_t i = 0; i != cvec_size(xml->elements); ++i)
        {
            SEXML_ELEMENT* elem = sexml_get_element_by_id(xml, i);
            sexml_exporter_write_element(f, elem);
        }

        fprintf(f, "</%*s>", xml->name->size, xml->name->ptr);
    }
}

void sexml_exporter_write_element_formatted(FILE* f, SEXML_ELEMENT* xml, const uint32_t indent)
{
    const uint64_t depth = sexml_get_element_depth(xml);
    uint8_t write_elements = 0;
    uint8_t write_text = 0;
    
    WRITE_DEPTH(f, depth, indent);
    
    fprintf(f, "<%*s", xml->name->size, xml->name->ptr);
    
    for(uint64_t i = 0; i != cvec_size(xml->attributes); ++i)
    {
        SEXML_ATTRIBUTE* attr = sexml_get_attribute_by_id(xml, i);
        sexml_text_to_entity_references(attr->value);
        fprintf(f, " %*s=\"%*s\"", attr->name->size, attr->name->ptr,
                                   attr->value->size, attr->value->ptr);
        sexml_text_from_entity_references(attr->value);
    }
    
    if(!cvec_empty(xml->elements))
    {
        write_elements = 1;
    }
    
    if(xml->text->size != 0)
    {
        write_text = 1;
    }
    
    if((write_elements == 0) && (write_text == 0))
    {
        fprintf(f, "/");
    }

    fprintf(f, ">");
    
    if(write_text && (write_elements == 0))
    {
        sexml_text_to_entity_references(xml->text);
        fprintf(f, "%*s", xml->text->size, xml->text->ptr);
        sexml_text_from_entity_references(xml->text);
        fprintf(f, "</%*s>\n", xml->name->size, xml->name->ptr);
    }
    
    if(write_text && write_elements)
    {
        fprintf(f, "\n");
        WRITE_DEPTH(f, (depth+1), indent);
        sexml_text_to_entity_references(xml->text);
        fprintf(f, "%*s\n", xml->text->size, xml->text->ptr);
        sexml_text_from_entity_references(xml->text);
    }
    
    if((write_text == 0) && write_elements)
    {
        fprintf(f, "\n");
    }
    
    if(write_elements)
    {
        for(uint64_t i = 0; i != cvec_size(xml->elements); ++i)
        {
            SEXML_ELEMENT* elem = sexml_get_element_by_id(xml, i);
            sexml_exporter_write_element_formatted(f, elem, indent);
        }
        
        WRITE_DEPTH(f, depth, indent);
        fprintf(f, "</%*s>\n", xml->name->size, xml->name->ptr);
    }
    
    if((write_text == 0) && (write_elements == 0))
    {
        fprintf(f, "\n");
    }
}