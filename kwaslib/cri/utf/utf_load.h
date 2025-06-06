#pragma once

#include <stdint.h>

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/data/cvector.h>

#include "utf_defines.h"
#include "utf_common.h"
#include "utf_table.h"

UTF_TABLE* utf_load_from_data(const uint8_t* data);

const UTF_HEADER utf_read_header(const uint8_t* data);
const UTF_TABLE_HEADER utf_read_table_header(const uint8_t* data);

/*
    Reads schema to a cvector of UTF_SCHEMA_ENTRY.
*/
CVEC utf_read_schema(const uint8_t* data, const uint16_t columns_count);

UTF_RECORD utf_read_record_by_type(const uint8_t* data, const uint8_t type);

void utf_record_to_table_row(UTF_RECORD* record, UTF_ROW* row, const uint8_t type,
                             const char* strtbl_ptr, const uint8_t* data_ptr);
                             
