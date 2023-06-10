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
} DL_DIR_LIST;

/*
	Functions
*/

void dl_parse_directory(const char* path, DL_DIR_LIST* list);

uint32_t dl_count_entries(const char* path);

/* Won't free "list" */
void dl_free_list(DL_DIR_LIST* list);

void dl_print_list(const DL_DIR_LIST* list);