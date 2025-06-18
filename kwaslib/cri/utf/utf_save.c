#include "utf_save.h"

#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/math/boundary.h>

#include "utf_common.h"
#include "utf_string_table.h"
#include "utf_data_table.h"

#include <kwaslib/cri/acb/acb_command.h>

FU_FILE* utf_save_to_fu(UTF_TABLE* utf)
{
    FU_FILE* utf_fu = fu_alloc_file();
    fu_create_mem_file(utf_fu);
    
    UTF_HEADER header = {0};
    UTF_TABLE_HEADER table_header = {0};
    SU_STRING* data_table = su_create_string("", 0);
    SU_STRING* string_table = su_create_string("", 0);
    
    /* Name for the table is first in the string_table */
    table_header.name_offset = utf_add_str_to_table(string_table, utf->name->ptr, utf->name->size);
    
    const uint8_t utf_present = utf_check_for_utf_tables(utf);
    const uint32_t columns_count = utf_table_get_column_count(utf);
    const uint32_t rows_count = utf_table_get_row_count(utf);
    CVEC schema = utf_generate_schema(utf, data_table, string_table, utf_present);
    const uint32_t rows_width = utf_get_row_size(schema);
    
    FU_FILE* schema_fu = utf_schema_to_fu(schema);
    FU_FILE* rows_fu = utf_rows_to_fu(utf, schema, string_table, data_table, utf_present);

    /*const uint32_t data_shift = bound_calc_leftover(16,
                                8 + UTF_TABLE_HEADER_SIZE +
                                schema_fu->size +
                                rows_width*rows_count +
                                string_table->size);*/
    
    /* Update remaining table header fields */
    table_header.version = 1;
    table_header.rows_offset = UTF_TABLE_HEADER_SIZE + schema_fu->size;
    table_header.string_table_offset = table_header.rows_offset + rows_width*rows_count;
    table_header.columns_count = columns_count;
    table_header.data_offset = table_header.string_table_offset + string_table->size;
    if(utf_present) table_header.data_offset += bound_calc_leftover(32, 8+table_header.data_offset);
    table_header.rows_width = rows_width;
    table_header.rows_count = rows_count;
    
    /* Update the header */
    memcpy(&header.id[0], UTF_MAGIC, 4);
    header.table_size = table_header.data_offset + data_table->size;         
    header.table_size += bound_calc_leftover(4, header.table_size);
    fu_change_buf_size(utf_fu, header.table_size+8);
    
    /* Merge everything */
    fu_write_data(utf_fu, (const uint8_t*)&header.id[0], 4);
    fu_write_u32(utf_fu, header.table_size, FU_BIG_ENDIAN);
    
    fu_write_u16(utf_fu, table_header.version, FU_BIG_ENDIAN);
    fu_write_u16(utf_fu, table_header.rows_offset, FU_BIG_ENDIAN);
    fu_write_u32(utf_fu, table_header.string_table_offset, FU_BIG_ENDIAN);
    fu_write_u32(utf_fu, table_header.data_offset, FU_BIG_ENDIAN);
    fu_write_u32(utf_fu, table_header.name_offset, FU_BIG_ENDIAN);
    fu_write_u16(utf_fu, table_header.columns_count, FU_BIG_ENDIAN);
    fu_write_u16(utf_fu, table_header.rows_width, FU_BIG_ENDIAN);
    fu_write_u32(utf_fu, table_header.rows_count, FU_BIG_ENDIAN);
    
    fu_write_data(utf_fu, (const uint8_t*)schema_fu->buf, schema_fu->size);
    if(rows_width) fu_write_data(utf_fu, (const uint8_t*)rows_fu->buf, rows_fu->size);
    
    fu_seek(utf_fu, table_header.string_table_offset+8, FU_SEEK_SET);
    fu_write_data(utf_fu, (const uint8_t*)string_table->ptr, string_table->size);
    
    fu_seek(utf_fu, table_header.data_offset+8, FU_SEEK_SET);
    fu_write_data(utf_fu, (const uint8_t*)data_table->ptr, data_table->size);
    
    /* Cleanup */
    fu_close(schema_fu);
    free(schema_fu);
    fu_close(rows_fu);
    free(rows_fu);
    schema = cvec_destroy(schema);
    data_table = su_free(data_table);
    string_table = su_free(string_table);
    
    return utf_fu;
}

CVEC utf_generate_schema(UTF_TABLE* utf,
                         SU_STRING* data_table,
                         SU_STRING* string_table,
                         const uint8_t utf_present)
{
    const uint32_t columns_count = utf_table_get_column_count(utf);
    CVEC schema = cvec_create(sizeof(UTF_SCHEMA_ENTRY));
    cvec_resize(schema, columns_count);
    
    for(uint32_t i = 0; i != columns_count; ++i)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        UTF_SCHEMA_ENTRY* se = (UTF_SCHEMA_ENTRY*)cvec_at(schema, i);
        
        /*
            Crashes Sonic Frontiers??
            TODO: Figure out why it crashes
        */
        /*const uint8_t are_rows_the_same = utf_column_rows_the_same(col);*/
        const uint8_t are_rows_the_same = 0;
        
        se->desc.type = col->type;
        se->desc.name = 1;
        se->desc.schema = are_rows_the_same ? 1 : 0;
        se->desc.row = are_rows_the_same ? 0 : 1;
        
        se->name_offset = utf_add_str_to_table(string_table, col->name->ptr, col->name->size);
    
        /* Data is in schema */
        if(se->desc.schema)
        {
            UTF_ROW* first_row = utf_table_get_row_from_col_by_id(col, 0);
            utf_table_row_to_record(first_row, &se->record, se->desc.type,
                                    string_table, data_table, utf_present);
        }
    }
    
    return schema;
}

FU_FILE* utf_schema_to_fu(CVEC schema)
{
    FU_FILE* schema_fu = fu_alloc_file();
    fu_create_mem_file(schema_fu);
    
    for(uint32_t i = 0; i != cvec_size(schema); ++i)
    {
        UTF_SCHEMA_ENTRY* se = (UTF_SCHEMA_ENTRY*)cvec_at(schema, i);
        
        fu_write_u8(schema_fu, *(const uint8_t*)&se->desc);
        
        if(se->desc.name)
        {
            fu_write_u32(schema_fu, se->name_offset, FU_BIG_ENDIAN);
        }
        
        if(se->desc.schema)
        {
            utf_write_record_to_fu(schema_fu, &se->record, se->desc.type);
        }
    }
    
    return schema_fu;
}

FU_FILE* utf_rows_to_fu(UTF_TABLE* utf, CVEC schema,
                        SU_STRING* string_table, SU_STRING* data_table,
                        const uint8_t utf_present)
{
    FU_FILE* rows_fu = fu_alloc_file();
    fu_create_mem_file(rows_fu);

    const uint32_t columns_count = utf_table_get_column_count(utf);
    const uint32_t rows_count = utf_table_get_row_count(utf);
    
    for(uint32_t row_it = 0; row_it != rows_count; ++row_it)
    {
        for(uint32_t column_it = 0; column_it != columns_count; ++column_it)
        {
            UTF_SCHEMA_ENTRY* se = (UTF_SCHEMA_ENTRY*)cvec_at(schema, column_it);
            const uint8_t type = se->desc.type;

            if(se->desc.row)
            {
                UTF_ROW* row = utf_table_get_row_xy(utf, column_it, row_it);
                UTF_RECORD record = {0};
                utf_table_row_to_record(row, &record, type, string_table, data_table, utf_present);
                utf_write_record_to_fu(rows_fu, &record, type);
            }
        }
    }
    
    return rows_fu;
}

const uint8_t utf_column_rows_the_same(UTF_COLUMN* col)
{
    const uint32_t rows_count = cvec_size(col->rows);
    
    /* 
        If rows_count is 1, all data will be in the rows section,
        even though all rows are "the same".
        Same goes for rows_count == 0.
    */
    if((rows_count == 0) || (rows_count == 1))
    {
        return 0;
    }
    
    UTF_ROW* first_row = utf_table_get_row_from_col_by_id(col, 0);
    uint8_t the_same = 1;
    
    for(uint32_t i = 1; i != rows_count; ++i)
    {
        UTF_ROW* cur_row = utf_table_get_row_from_col_by_id(col, i);
        
        switch(col->type)
        {
            case UTF_COLUMN_TYPE_UINT8:
            case UTF_COLUMN_TYPE_SINT8:
                if(first_row->data.s8 != cur_row->data.s8)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_UINT16:
            case UTF_COLUMN_TYPE_SINT16:
                if(first_row->data.s16 != cur_row->data.s16)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_UINT32:
            case UTF_COLUMN_TYPE_SINT32:
                if(first_row->data.s32 != cur_row->data.s32)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_UINT64:
            case UTF_COLUMN_TYPE_SINT64:
                if(first_row->data.s64 != cur_row->data.s64)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_FLOAT:
                if(first_row->data.f32 != cur_row->data.f32)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_DOUBLE:
                if(first_row->data.f64 != cur_row->data.f64)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_STRING:
                if(su_cmp_string(first_row->data.str, cur_row->data.str) != SU_STRINGS_MATCH)
                    the_same = 0;
                break;
            case UTF_COLUMN_TYPE_VLDATA:
                if(first_row->embed_type == UTF_TABLE_VL_NONE)
                {
                    if(first_row->data.vl->size != cur_row->data.vl->size)
                    {
                        the_same = 0;
                    }
                    else if((first_row->data.vl->size > 1) && (first_row->data.vl->size < 16))
                    {
                        the_same = 0;
                    }
                    if(su_cmp_string(first_row->data.vl, cur_row->data.vl) != SU_STRINGS_MATCH)
                    {
                        the_same = 0;
                    }
                }
                else
                {
                    the_same = 0;
                }
                break;
            case UTF_COLUMN_TYPE_UINT128:
                if(su_cmp_char((const char*)first_row->data.u128, 16,
                               (const char*)cur_row->data.u128, 16)
                               == SU_STRINGS_MATCH)
                    the_same = 0;
                break;
        }
        
        if(the_same == 0)
        {
            break;
        }
    }
    
    return the_same;
}

void utf_table_row_to_record(UTF_ROW* row, UTF_RECORD* record, const uint8_t type,
                             SU_STRING* string_table, SU_STRING* data_table,
                             const uint8_t utf_present)
{
    switch(type)
    {
        case UTF_COLUMN_TYPE_UINT8:
        case UTF_COLUMN_TYPE_SINT8:
            record->u8 = row->data.u8;
            break;    
        case UTF_COLUMN_TYPE_UINT16:
        case UTF_COLUMN_TYPE_SINT16:
            record->u16 = row->data.u16;
            break;
        case UTF_COLUMN_TYPE_UINT32:
        case UTF_COLUMN_TYPE_SINT32:
            record->u32 = row->data.u32;
            break;
        case UTF_COLUMN_TYPE_UINT64:
        case UTF_COLUMN_TYPE_SINT64:
            record->u64 = row->data.u64;
            break;
        case UTF_COLUMN_TYPE_FLOAT:
            record->f32 = row->data.f32;
            break;
        case UTF_COLUMN_TYPE_DOUBLE:
            record->f64 = row->data.f64;
            break;
        case UTF_COLUMN_TYPE_STRING:
            record->str_offset = utf_add_str_to_table(string_table,
                                                      &row->data.str->ptr[0],
                                                      row->data.str->size);
            break;
        case UTF_COLUMN_TYPE_VLDATA:
            switch(row->embed_type)
            {
                case UTF_TABLE_VL_NONE:
                    record->vl.offset = utf_add_data_to_table(data_table,
                                        (const uint8_t*)&row->data.vl->ptr[0],
                                        row->data.vl->size, utf_present);
                    record->vl.size = row->data.vl->size;
                    break;
                case UTF_TABLE_VL_UTF:
                    FU_FILE* utf_fu = utf_save_to_fu(row->embed.utf);
                    record->vl.offset = utf_add_data_to_table(data_table,
                                        (const uint8_t*)utf_fu->buf,
                                        utf_fu->size, utf_present);
                    record->vl.size = utf_fu->size;
                    fu_close(utf_fu);
                    free(utf_fu);
                    break;
                case UTF_TABLE_VL_AFS2:
                    SU_STRING* afs2_str = awb_to_data(row->embed.afs2);
                    record->vl.offset = utf_add_data_to_table(data_table,
                                        (const uint8_t*)afs2_str->ptr,
                                        afs2_str->size, utf_present);
                    record->vl.size = afs2_str->size;
                    afs2_str = su_free(afs2_str);
                    break;
                case UTF_TABLE_VL_ACBCMD:
                    SU_STRING* acbcmd_str = acb_cmd_to_data(row->embed.acbcmd);
                    record->vl.offset = utf_add_data_to_table(data_table,
                                        (const uint8_t*)acbcmd_str->ptr,
                                        acbcmd_str->size, utf_present);
                    record->vl.size = acbcmd_str->size;
                    acbcmd_str = su_free(acbcmd_str);
                    break;
            }

            break;
        case UTF_COLUMN_TYPE_UINT128:
            memcpy(&record->u128[0], &row->data.u128[0], 16);
            break;
    }
}

void utf_write_record_to_fu(FU_FILE* fu, UTF_RECORD* record, const uint8_t type)
{
    switch(type)
    {
        case UTF_COLUMN_TYPE_UINT8:
        case UTF_COLUMN_TYPE_SINT8:
            fu_write_u8(fu, record->u8);
            break;    
        case UTF_COLUMN_TYPE_UINT16:
        case UTF_COLUMN_TYPE_SINT16:
            fu_write_u16(fu, record->u16, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_UINT32:
        case UTF_COLUMN_TYPE_SINT32:
            fu_write_u32(fu, record->u32, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_UINT64:
        case UTF_COLUMN_TYPE_SINT64:
            fu_write_u64(fu, record->u64, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_FLOAT:
            fu_write_f32(fu, record->f32, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_DOUBLE:
            fu_write_f64(fu, record->f64, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_STRING:
            fu_write_u32(fu, record->str_offset, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_VLDATA:
            fu_write_u32(fu, record->vl.offset, FU_BIG_ENDIAN);
            fu_write_u32(fu, record->vl.size, FU_BIG_ENDIAN);
            break;
        case UTF_COLUMN_TYPE_UINT128:
            fu_write_data(fu, &record->u128[0], 16);
            break;
    }
}

const uint32_t utf_get_row_size(CVEC schema)
{
    uint32_t rows_width = 0;
    const uint32_t schema_size = cvec_size(schema);
    
    for(uint32_t i = 0; i != schema_size; ++i)
    {
        UTF_SCHEMA_ENTRY* se = (UTF_SCHEMA_ENTRY*)cvec_at(schema, i);

        if(se->desc.row)
        {
            rows_width += utf_get_type_size(se->desc.type);
        }
    }
    
    return rows_width;
}

const uint8_t utf_check_for_utf_tables(UTF_TABLE* utf)
{
    const uint32_t columns_count = utf_table_get_column_count(utf);
    const uint32_t rows_count = utf_table_get_row_count(utf);
    
    for(uint32_t row_it = 0; row_it != rows_count; ++row_it)
    {
        for(uint32_t column_it = 0; column_it != columns_count; ++column_it)
        {
            UTF_ROW* row = utf_table_get_row_xy(utf, column_it, row_it);

            if(row->embed_type == UTF_TABLE_VL_UTF)
            {
                return 1;
            }
        }
    }
    
    return 0;
}