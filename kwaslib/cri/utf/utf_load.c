#include "utf_load.h"

#include <string.h>
#include <stdio.h>

#include "utf_common.h"

UTF_TABLE* utf_load_from_data(const uint8_t* data)
{
    const uint8_t* th_ptr = &data[8];
    UTF_TABLE* utf = NULL;
    const UTF_HEADER header = utf_read_header(data);
    
    /* It isn't the @UTF table */
    if(su_cmp_char(&header.id[0], 4, UTF_MAGIC, 4) != SU_STRINGS_MATCH)
    {
        return NULL;
    }
    
    const UTF_TABLE_HEADER th = utf_read_table_header(th_ptr);
    const uint8_t* schema_ptr = &th_ptr[0x18];
    const uint8_t* rows_ptr = &th_ptr[th.rows_offset];
    const char* strtbl_ptr = (const char*)&th_ptr[th.string_table_offset];
    const uint8_t* data_ptr = &th_ptr[th.data_offset];
    CVEC schema = utf_read_schema(schema_ptr, th.columns_count);
    
    utf = utf_table_create_by_size(&strtbl_ptr[th.name_offset],
                                   th.columns_count, th.rows_count);

    /* Insert all data from schema */
    uint32_t rows_iter_offset = 0;
    
    for(uint32_t i = 0; i != th.columns_count; ++i)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        UTF_SCHEMA_ENTRY* se = (UTF_SCHEMA_ENTRY*)cvec_at(schema, i);
        col->type = se->desc.type;
        
        /* Name of the column */
        if(se->desc.name)
        {
            const char* name_str = &strtbl_ptr[se->name_offset];
            su_insert_char(col->name, -1, name_str, strlen(name_str));
        }
        
        /* All rows have the value from schema */
        if(se->desc.schema)
        {
            for(uint32_t j = 0; j != th.rows_count; ++j)
            {
                UTF_ROW* row = utf_table_get_row_from_col_by_id(col, j);
                utf_record_to_table_row(&se->record, row, se->desc.type, strtbl_ptr, data_ptr);
            }
        }
        
        /* Row has data in rows */
        if(se->desc.row)
        {
            for(uint32_t j = 0; j != th.rows_count; ++j)
            {
                const uint8_t* row_item_ptr = &rows_ptr[th.rows_width * j + rows_iter_offset];
                UTF_ROW* row = utf_table_get_row_from_col_by_id(col, j);
                UTF_RECORD record = utf_read_record_by_type(row_item_ptr, se->desc.type);
                
                utf_record_to_table_row(&record, row, se->desc.type,
                                        strtbl_ptr, data_ptr);
            }
            
            rows_iter_offset += utf_get_type_size(se->desc.type);
        }
    }
    
    utf_check_for_embedded(utf);
    
    return utf;
}

const UTF_HEADER utf_read_header(const uint8_t* data)
{
    UTF_HEADER header = {0};
    tr_read_array(&data[0], 4, (uint8_t*)&header.id[0]);
    header.table_size = tr_read_u32be(&data[4]);
    return header;
}

const UTF_TABLE_HEADER utf_read_table_header(const uint8_t* data)
{
    UTF_TABLE_HEADER header = {0};
    header.version = tr_read_u16be(&data[0]);
    header.rows_offset = tr_read_u16be(&data[2]);
    header.string_table_offset = tr_read_u32be(&data[4]);
    header.data_offset = tr_read_u32be(&data[8]);
    header.name_offset = tr_read_u32be(&data[12]);
    header.columns_count = tr_read_u16be(&data[16]);
    header.rows_width = tr_read_u16be(&data[18]);
    header.rows_count = tr_read_u32be(&data[20]);
    return header;
}

CVEC utf_read_schema(const uint8_t* data, const uint16_t columns_count)
{
    uint8_t* schema_ptr = (uint8_t*)&data[0];
    CVEC schema = cvec_create(sizeof(UTF_SCHEMA_ENTRY));
    cvec_resize(schema, columns_count);
    
    for(uint32_t i = 0 ; i != columns_count; ++i)
    {
        UTF_SCHEMA_ENTRY* se = (UTF_SCHEMA_ENTRY*)cvec_at(schema, i);
        
        tr_read_array(schema_ptr, 1, (uint8_t*)&se->desc);
        schema_ptr += 1;
        
        if(se->desc.name)
        {
            se->name_offset = tr_read_u32be(schema_ptr);
            schema_ptr += 4;
        }
        
        if(se->desc.schema)
        {
            se->record = utf_read_record_by_type(schema_ptr, se->desc.type);
            schema_ptr += utf_get_type_size(se->desc.type);
        }
    }
    
    return schema;
}

UTF_RECORD utf_read_record_by_type(const uint8_t* data, const uint8_t type)
{
    UTF_RECORD record;
    memset(&record, 0, sizeof(UTF_RECORD));
    
    switch(type)
    {
        case UTF_COLUMN_TYPE_UINT8:
        case UTF_COLUMN_TYPE_SINT8:
            record.u8 = tr_read_u8(data);
            break;    
        case UTF_COLUMN_TYPE_UINT16:
        case UTF_COLUMN_TYPE_SINT16:
            record.u16 = tr_read_u16be(data);
            break;
        case UTF_COLUMN_TYPE_UINT32:
        case UTF_COLUMN_TYPE_SINT32:
        case UTF_COLUMN_TYPE_STRING:
            record.u32 = tr_read_u32be(data);
            break;
        case UTF_COLUMN_TYPE_UINT64:
        case UTF_COLUMN_TYPE_SINT64:
            record.u64 = tr_read_u64be(data);
            break;
        case UTF_COLUMN_TYPE_FLOAT:
            record.f32 = tr_read_f32be(data);
            break;
        case UTF_COLUMN_TYPE_DOUBLE:
            record.f64 = tr_read_f64be(data);
            break;
        case UTF_COLUMN_TYPE_VLDATA:
            record.vl.offset = tr_read_u32be(data);
            record.vl.size = tr_read_u32be(&data[4]);
            break;
        case UTF_COLUMN_TYPE_UINT128:
            memcpy(&record.u128[0], data, 16);
            break;
    }
    
    return record;
}

void utf_record_to_table_row(UTF_RECORD* record, UTF_ROW* row, const uint8_t type,
                             const char* strtbl_ptr, const uint8_t* data_ptr)
{
    switch(type)
    {
        case UTF_COLUMN_TYPE_UINT8:
        case UTF_COLUMN_TYPE_SINT8:
            row->data.u8 = record->u8;
            break;    
        case UTF_COLUMN_TYPE_UINT16:
        case UTF_COLUMN_TYPE_SINT16:
            row->data.u16 = record->u16;
            break;
        case UTF_COLUMN_TYPE_UINT32:
        case UTF_COLUMN_TYPE_SINT32:
            row->data.u32 = record->u32;
            break;
        case UTF_COLUMN_TYPE_UINT64:
        case UTF_COLUMN_TYPE_SINT64:
            row->data.u64 = record->u64;
            break;
        case UTF_COLUMN_TYPE_FLOAT:
            row->data.f32 = record->f32;
            break;
        case UTF_COLUMN_TYPE_DOUBLE:
            row->data.f64 = record->f64;
            break;
        case UTF_COLUMN_TYPE_STRING:
            row->data.str = su_create_string(&strtbl_ptr[record->str_offset],
                                             strlen(&strtbl_ptr[record->str_offset]));
            break;
        case UTF_COLUMN_TYPE_VLDATA:
            row->data.vl = su_create_string((const char*)&data_ptr[record->vl.offset],
                                            record->vl.size);
            break;
        case UTF_COLUMN_TYPE_UINT128:
            memcpy(&row->data.u128[0], &record->u128[0], 16);
            break;
    }
}
