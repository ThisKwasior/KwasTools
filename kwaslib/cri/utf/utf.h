#pragma once

#include <kwaslib/core/io/file_utils.h>

#include "utf_table.h"

UTF_TABLE* utf_load_file(FU_FILE* utf_file);

FU_FILE* utf_save_file(UTF_TABLE* utf);

