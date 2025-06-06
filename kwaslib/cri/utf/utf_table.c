#include "utf_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utf_load.h"

UTF_TABLE* utf_table_create(const char* name)
{
    UTF_TABLE* utf = (UTF_TABLE*)calloc(1, sizeof(UTF_TABLE));
    
    if(utf)
    {
        utf->name = su_create_string(name, strlen(name));
        utf->columns = cvec_create(sizeof(UTF_COLUMN));
    }
    return utf;
}

UTF_TABLE* utf_table_destroy(UTF_TABLE* utf)
{
    if(utf)
    {
        const uint32_t columns_count = cvec_size(utf->columns);
        for(uint32_t i = 0; i != columns_count; ++i)
        {
            utf_table_remove_column_by_id(utf, 0);
        }
        
        utf->name = su_free(utf->name);
        utf->columns = cvec_destroy(utf->columns);
        free(utf);
    }
    
    return NULL;
}

UTF_TABLE* utf_table_create_by_size(const char* name, const uint32_t columns, const uint32_t rows)
{
    UTF_TABLE* utf = utf_table_create(name);
    
    /* Allocate columns */
    cvec_resize(utf->columns, columns);
    
    for(uint32_t i = 0; i != columns; ++i)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        col->name = su_create_string("", 0);
        col->type = UTF_COLUMN_TYPE_UINT8;
        
        /* and rows */
        col->rows = cvec_create(sizeof(UTF_ROW));
        cvec_resize(col->rows, rows);
    }
    
    return utf;
}

UTF_COLUMN* utf_table_append_column(UTF_TABLE* utf, const char* name, const uint8_t type)
{
    UTF_COLUMN* new_col = NULL;
    const uint32_t id = cvec_size(utf->columns);
    cvec_resize(utf->columns, id+1);
    
    if(id) /* Create a column with the same amount of rows as first column */
    {
        UTF_COLUMN* first_col = utf_table_get_column_by_id(utf, 0);
        const uint32_t rows_size = cvec_size(first_col->rows);
        new_col = utf_table_get_column_by_id(utf, id);
        new_col->rows = cvec_create(sizeof(UTF_ROW));
        cvec_resize(new_col->rows, rows_size);
    }
    else
    {
        new_col = utf_table_get_column_by_id(utf, 0);
        new_col->rows = cvec_create(sizeof(UTF_ROW));
    }
    
    new_col->name = su_create_string(name, strlen(name));
    new_col->type = type;

    return new_col;
}

const uint32_t utf_table_append_row(UTF_TABLE* utf)
{
    UTF_COLUMN* first_col = utf_table_get_column_by_id(utf, 0);
    const uint32_t id = cvec_size(first_col->rows);
    
    for(uint32_t i = 0; i != cvec_size(utf->columns); ++i)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        cvec_resize(col->rows, id+1);
    }
    
    return id;
}

void utf_table_remove_column_by_id(UTF_TABLE* utf, const uint32_t id)
{
    UTF_COLUMN* col = utf_table_get_column_by_id(utf, id);
    const uint32_t rows_count = cvec_size(col->rows);
    
    for(uint32_t i = 0; i != rows_count; ++i)
    {
        UTF_ROW* row = utf_table_get_row_from_col_by_id(col, i);
        utf_table_free_row_data(row, col->type);
    }
    
    col->name = su_free(col->name);
    col->type = 0;
    col->rows = cvec_destroy(col->rows);
    
    cvec_erase(utf->columns, id);
}

void utf_table_remove_row_by_id(UTF_TABLE* utf, const uint32_t id)
{
    for(uint32_t i = 0; i != cvec_size(utf->columns); ++i)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        utf_table_remove_row_from_col_by_id(col, id);
    }
}

void utf_table_remove_row_from_col_by_id(UTF_COLUMN* col, const uint32_t id)
{
    UTF_ROW* row = utf_table_get_row_from_col_by_id(col, id);
    utf_table_free_row_data(row, col->type);
    cvec_erase(col->rows, id);
}

void utf_table_free_row_data(UTF_ROW* row, const uint8_t column_type)
{
    /* Only free the row data when it's a str or vl */
    switch(column_type)
    {
        case UTF_COLUMN_TYPE_STRING:
            row->data.str = su_free(row->data.str);
            break;
        case UTF_COLUMN_TYPE_VLDATA:
            /* Check for embedded file types */
            if(row->embed_type == UTF_TABLE_VL_UTF)
            {
                row->embed.utf = utf_table_destroy(row->embed.utf);
            }
            else if(row->embed_type == UTF_TABLE_VL_AFS2)
            {
                row->embed.afs2 = awb_free(row->embed.afs2);
            }
            else if(row->embed_type == UTF_TABLE_VL_ACBCMD)
            {
                row->embed.acbcmd = acb_cmd_free(row->embed.acbcmd);
            }
            else
            {
                row->data.vl = su_free(row->data.vl);
            }
            break;
    }
}

UTF_COLUMN* utf_table_get_column_by_id(UTF_TABLE* utf, const uint32_t id)
{
    return (UTF_COLUMN*)cvec_at(utf->columns, id);
}

UTF_ROW* utf_table_get_row_xy(UTF_TABLE* utf, const uint32_t x, const uint32_t y)
{
    UTF_COLUMN* col = utf_table_get_column_by_id(utf, x);
    return utf_table_get_row_from_col_by_id(col, y);
}

UTF_ROW* utf_table_get_row_from_col_by_id(UTF_COLUMN* col, const uint32_t id)
{
    return (UTF_ROW*)cvec_at(col->rows, id);
}

void utf_check_for_embedded(UTF_TABLE* utf)
{
    const uint32_t columns_count = utf_table_get_column_count(utf);
    const uint32_t rows_count = utf_table_get_row_count(utf);
    
    for(uint32_t i = 0; i != columns_count; ++i)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        
        if(col->type == UTF_COLUMN_TYPE_VLDATA)
        {
            for(uint32_t j = 0; j != rows_count; ++j)
            {
                UTF_ROW* row = utf_table_get_row_from_col_by_id(col, j);
                const char* vl_ptr = &row->data.vl->ptr[0];
                const uint32_t vl_magic_size = (row->data.vl->size > 4) ? 4 : row->data.vl->size;
                
                /* No parsing if there's no data */
                if(row->data.vl->size == 0)
                    continue;
                
                if(su_cmp_char(vl_ptr, vl_magic_size, UTF_MAGIC, 4) == SU_STRINGS_MATCH)
                {
                    //printf("Found UTF at %p\n", &row->data.vl->ptr[0]);
                    row->embed.utf = utf_load_from_data((const uint8_t*)row->data.vl->ptr);
                    row->data.vl = su_free(row->data.vl);
                    row->embed_type = UTF_TABLE_VL_UTF;
                }
                else if(su_cmp_string_char(col->name, "AwbFile", 7) == SU_STRINGS_MATCH)
                {
                    //printf("Found AwbFile, not processing.\n");
                    row->embed.afs2 = awb_load_from_data((const uint8_t*)row->data.vl->ptr, row->data.vl->size);
                    row->data.vl = su_free(row->data.vl);
                    row->embed_type = UTF_TABLE_VL_AFS2;
                }
                else if(su_cmp_string_char(col->name, "Command", 7) == SU_STRINGS_MATCH)
                {
                    //printf("Found ACB Command, not processing.\n");
                    row->embed.acbcmd = acb_cmd_load_from_data((const uint8_t*)row->data.vl->ptr, row->data.vl->size);
                    row->data.vl = su_free(row->data.vl);
                    row->embed_type = UTF_TABLE_VL_ACBCMD;
                }
            }
        }
    }
}

const uint32_t utf_table_get_column_count(UTF_TABLE* utf)
{
    return cvec_size(utf->columns);
}

const uint32_t utf_table_get_row_count(UTF_TABLE* utf)
{
    UTF_COLUMN* col = utf_table_get_column_by_id(utf, 0);
    return cvec_size(col->rows);
}