#include "utf.h"

#include "utf_load.h"
#include "utf_save.h"

UTF_TABLE* utf_load_file(FU_FILE* utf_file)
{
    return utf_load_from_data((const uint8_t*)&utf_file->buf[0]);
}

FU_FILE* utf_save_file(UTF_TABLE* utf)
{
    return utf_save_to_fu(utf);
}

