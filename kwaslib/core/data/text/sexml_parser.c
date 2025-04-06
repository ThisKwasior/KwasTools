#include "sexml_parser.h"

#include <stdio.h>
#include <string.h>

#include "sexml.h"

void sexml_parser_read_start_tag(SEXML_PARSER_CTX* ctx)
{
    ctx->inside_tag = 1;
    ctx->text_it += 1;
    char cur_char = ctx->text[ctx->text_it];

    switch(cur_char)
    {                     
        case ' ': /* Space after the start tag is invalid */
            ctx->last_error = SEXML_ERR_SPACE_START;
            break;
        case '/': /* Closing tag */
            ctx->closing_tag = 1;
            ctx->text_it += 1;
            cur_char = ctx->text[ctx->text_it];
            
            if(cur_char == ' ') /* Space after the slash is also invalid */
            {
                ctx->last_error = SEXML_ERR_SPACE_START;
            }
            break;
    }
    
    /* Throw an error if not good after the checks */
    if(ctx->last_error != SEXML_GOOD)
    {
        return;
    }
    
    /* Finally reading the name */
    SU_STRING* str = sexml_parser_get_name(ctx);
    
    /*
        If closing, check if we're leaving the correct tag.
        Otherwise just apply the name to current element.
    */
    if(ctx->closing_tag)
    {
        /* Check for name itself */
        if(strncmp(str->ptr, ctx->cur_elem->name->ptr, str->size) != 0)
            ctx->last_error = SEXML_ERR_END_NO_MATCH;  
      
        /* Second check, do the string sizes match */
        if(str->size != ctx->cur_elem->name->size)
            ctx->last_error = SEXML_ERR_END_NO_MATCH;
    }
    else
    {
        ctx->cur_elem->name = su_copy(str);
    }
    
    str = su_free(str);
}

SU_STRING* sexml_parser_get_name(SEXML_PARSER_CTX* ctx)
{
    uint64_t it = 0;
    char* text = &ctx->text[ctx->text_it];
    
    for(;;++it)
    {
        if((text[it] == ' ')
        || (text[it] == '>')
        || (text[it] == '=')
        || (text[it] == '/'))
        {
            break;
        }
    }
    
    ctx->text_it += it;
    
    return su_create_string(text, it);
}


void sexml_parser_read_attribute(SEXML_PARSER_CTX* ctx)
{
    SEXML_ATTRIBUTE* attr = sexml_alloc_attribute();
    attr->name = sexml_parser_get_name(ctx);

    /* Checking if the attribute is even an attribute */
    if(ctx->text[ctx->text_it] != '=')
    {
        ctx->last_error = SEXML_ERR_ATTR_MALFORM;
    }
    else
    {
        ctx->text_it += 1;
        
        if(ctx->text[ctx->text_it] == '"')
        {
            attr->value = sexml_parser_read_attribute_value(ctx);
            sexml_text_from_entity_references(attr->value);
            cvec_push_back(ctx->cur_elem->attributes, &attr);
        }
        else
        {
            ctx->last_error = SEXML_ERR_ATTR_MALFORM;  
        }
    }
}

SU_STRING* sexml_parser_read_attribute_value(SEXML_PARSER_CTX* ctx)
{
    if(ctx->text[ctx->text_it] == '"')
    {
        ctx->text_it += 1;
    }
    
    uint64_t it = 0;
    char* text = &ctx->text[ctx->text_it];
    
    for(;;++it)
    {
        if(text[it] == '"')
        {
            break;
        }
    }
    
    ctx->text_it += it + 1;
    
    /* Last check if area after the value is correct */
    if((ctx->text[ctx->text_it] != ' ')
    && (ctx->text[ctx->text_it] != '>')
    && (ctx->text[ctx->text_it] != '/'))
    {
        ctx->last_error = SEXML_ERR_ATTR_NOSPACE;
    }
    
    return su_create_string(text, it);
}

void sexml_parser_read_text(SEXML_PARSER_CTX* ctx)
{
    int64_t it = 0;
    char* text = &ctx->text[ctx->text_it];
    
    for(;;it+=1)
    {
        if(text[it] == '<')
            break;
    }
    
    ctx->text_it += it;
    it -= 1;
    
    /* Extract text without spaces at the end */
    for(;;it-=1)
    {
        if((text[it] != ' ')
        && (text[it] != '\n')
        && (text[it] != '\r')
        && (text[it] != '\t'))
        {
            it += 1;
            break;
        }
    }
    
    if(it > 0)
    {
        su_insert_char(ctx->cur_elem->text, -1, text, it);
        sexml_text_from_entity_references(ctx->cur_elem->text);
    }
}

void sexml_parser_check_for_prolog(SEXML_PARSER_CTX* ctx)
{
    uint64_t it = 0;
    char* text = &ctx->text[ctx->text_it];
    
    if(strncmp("<?xml", &ctx->text[ctx->text_it], 5) == 0)
    {
        it = 5;
        
        while(1)
        {
            if((text[it] == '\0') || (text[it] == '>'))
            {
                it += 1;
                break;
            }
            
            it += 1;
        }
    }
    
    if(ctx->text_it >= ctx->text_size)
    {
        ctx->last_error = SEXML_ERR_MALFORMED_XML;
    }
    
    ctx->text_it += it;
}