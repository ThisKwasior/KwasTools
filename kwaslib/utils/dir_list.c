#include "dir_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dl_parse_directory(const char* path, DL_DIR_LIST* list)
{
	DIR* pdir = opendir(path);
	const uint32_t entries_count = dl_count_entries(path);
	
	pu_split_path(path, strlen(path), &list->path);
	list->entries = (DL_DIR_ENTRY*)calloc(entries_count, sizeof(DL_DIR_ENTRY));
	list->size = entries_count;
	
	/* Needed for checking if the entry is a file or directory */
	PU_STRING base_path = {0};
	pu_path_to_string(&list->path, &base_path);
	
	if(pdir)
	{
		uint32_t it = 0;
		while(1)
		{
			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;
			
			if((strncmp(pent->d_name, ".", 1) != 0)
				&& (strncmp(pent->d_name, "..", 2) != 0))
			{
				pu_split_path(pent->d_name, strlen(pent->d_name), &list->entries[it].path);
				
				PU_STRING file_path = {0};
				pu_create_string(base_path.p, base_path.s, &file_path);
				pu_insert_char("/", 1, -1, &file_path);
				pu_insert_char(pent->d_name, strlen(pent->d_name), -1, &file_path);
				
				if(pu_is_dir(file_path.p)) list->entries[it].type = DL_TYPE_DIR;
				else list->entries[it].type = DL_TYPE_FILE;

				it += 1;
				
				pu_free_string(&file_path);
			}
		}
	}

	pu_free_string(&base_path);

	closedir(pdir);
}

uint32_t dl_count_entries(const char* path)
{
	DIR* pdir = opendir(path);
	uint32_t entries = 0;
	
	if(pdir)
	{
		while(1)
		{
			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;
			
			if((strncmp(pent->d_name, ".", 1) != 0)
				&& (strncmp(pent->d_name, "..", 2) != 0))
			{
				entries += 1;
			}
		}
	}

	closedir(pdir);
	
	return entries;
}

void dl_free_list(DL_DIR_LIST* list)
{
	pu_free_path(&list->path);
	memset(&list->path, 0, sizeof(PU_PATH));
	
	for(uint32_t i = 0; i != list->size; ++i)
	{
		pu_free_path(&list->entries[i].path);
	}
	
	free(list->entries);
	
	list->size = 0;
}

void dl_print_list(const DL_DIR_LIST* list)
{
	PU_STRING path = {0};
	pu_path_to_string(&list->path, &path);
	printf("Dir path - %*s\n", path.s, path.p);
	pu_free_string(&path);
	
	for(uint32_t i = 0; i != list->size; ++i)
	{
		PU_STRING str = {0};
		pu_path_to_string(&list->entries[i].path, &str);
		
		printf("%6s %.*s\n",
			   (list->entries[i].type == DL_TYPE_DIR ? "<DIR>" : "<FILE>"),
			   str.s, str.p);
			   
		pu_free_string(&str);
	}
}