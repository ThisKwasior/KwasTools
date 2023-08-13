#pragma once

#include <stdint.h>
#include <dirent.h>

#include "path_utils.h"

#define DL_TYPE_FILE 1
#define DL_TYPE_DIR 2

typedef struct
{
	PU_PATH path;
	uint8_t type;
} DL_DIR_ENTRY;

typedef struct
{
	PU_PATH path;
	DL_DIR_ENTRY* entries;
	uint32_t size;
	
	uint32_t file_count;
	uint32_t dir_count;
} DL_DIR_LIST;

/*
	Functions
*/

void dl_parse_directory(const char* path, DL_DIR_LIST* list);

uint32_t dl_count_entries(const char* path);

PU_STRING* dl_get_full_entry_path(DL_DIR_LIST* list, uint32_t entry_id);

/* Won't free "list" */
void dl_free_list(DL_DIR_LIST* list);

void dl_print_list(const DL_DIR_LIST* list);