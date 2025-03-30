#include "dir_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dl_parse_directory(const char* path, DL_DIR_LIST* list)
{
	DIR* pdir = opendir(path);
	const uint32_t entries_count = dl_count_entries(path);
	
	list->path = pu_split_path(path, strlen(path));
	list->entries = (DL_DIR_ENTRY*)calloc(entries_count, sizeof(DL_DIR_ENTRY));
	list->size = entries_count;
	
	/* Needed for checking if the entry is a file or directory */
	SU_STRING* base_path = pu_path_to_string(list->path);
	
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
				list->entries[it].path = pu_split_path(pent->d_name, strlen(pent->d_name));
				
				SU_STRING* file_path = su_copy(base_path);
				su_insert_char(file_path, -1, "/", 1);
				su_insert_char(file_path, -1, pent->d_name, strlen(pent->d_name));
				
				if(pu_is_dir(file_path->ptr))
				{
					list->entries[it].type = DL_TYPE_DIR;
					list->dir_count += 1;
				}
				else
				{
					list->entries[it].type = DL_TYPE_FILE;
					list->file_count += 1;
				}

				it += 1;
				
				su_free(file_path);
			}
		}
	}

	su_free(base_path);

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

SU_STRING* dl_get_full_entry_path(DL_DIR_LIST* list, uint32_t entry_id)
{
	if(entry_id > list->size) return su_create_string("", 0);
	
    SU_STRING* str = pu_path_to_string(list->path);
    su_insert_char(str, -1, "/", 1);
    
	SU_STRING* temp_file_name = pu_path_to_string(list->entries[entry_id].path);
	su_insert_string(str, -1, temp_file_name);
    su_free(temp_file_name);
    
	return str;
}

void dl_free_list(DL_DIR_LIST* list)
{
	list->path = pu_free_path(list->path);
	
	for(uint32_t i = 0; i != list->size; ++i)
	{
		list->entries[i].path = pu_free_path(list->entries[i].path);
	}
	
	free(list->entries);
	
	list->size = 0;
}

void dl_print_list(const DL_DIR_LIST* list)
{
	SU_STRING* path = pu_path_to_string(list->path);
	printf("Dir path - %*s\n", path->size, path->ptr);
    su_free(path);
	
	for(uint32_t i = 0; i != list->size; ++i)
	{
		SU_STRING* str = pu_path_to_string(list->entries[i].path);
		
		printf("%6s %.*s\n",
			   (list->entries[i].type == DL_TYPE_DIR ? "<DIR>" : "<FILE>"),
			   str->size, str->ptr);
			   
		su_free(str);
	}
}