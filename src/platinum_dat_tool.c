#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/cpu/endianness.h>
#include <kwaslib/platinum/dat.h>

/*
	Arguments
*/
const AP_ARG_DESC arg_list[] =
{
	{"--x360", AP_TYPE_NOV},
	{"--skip_ext_check", AP_TYPE_NOV},
	{"--block_size", AP_TYPE_U32},
};
const uint32_t arg_list_size = 3;

AP_VALUE_NODE* arg_node = NULL;

/* Flags */
uint8_t dat_tool_endian = FU_HOST_ENDIAN;
uint8_t flag_skip_ext_check = 0;

uint32_t val_block_size = DAT_DEFAULT_BLOCK_SIZE;

void dat_tool_parse_arguments(int argc, char** argv);

/*
	Common
*/
void dat_tool_print_usage(char* program_name);
void dat_tool_print_dat(DAT_FILE* dat);

/* 
	Unpacker
*/
DAT_FILE* dat_tool_parse_file(const char* filepath);
void dat_tool_extract_to_folder(DAT_FILE* dat, PU_PATH* folder);

/* 
	Packer
*/
DBL_LIST_NODE* dat_tool_load_files_from_dir(const char* dir);

/*
	Entry point
*/
int main(int argc, char** argv)
{
	if(argc < 2)
	{
		dat_tool_print_usage(argv[0]);
		return 0;
	}

	dat_tool_parse_arguments(argc, argv);
	
	/* It's a file so let's process a DAT file */
	if(pu_is_file(argv[1]))
	{
		DAT_FILE* dat = dat_tool_parse_file(argv[1]);
		
		if(dat == NULL)
		{
			printf("Couldn't process the DAT file.\n");
		}
		else
		{			
			/* Print dat info */
			dat_tool_print_dat(dat);
			
			PU_PATH dat_path = {0};
			pu_split_path(argv[1], strlen(argv[1]), &dat_path);
			
			/* I love hacking my own code */
			pu_insert_char("_", 1, -1, &dat_path.name);
			pu_insert_char(dat_path.ext.p, 3, -1, &dat_path.name);
			pu_free_string(&dat_path.ext);
			dat_path.type = PU_PATH_TYPE_DIR;
			
			PU_STRING str = {0};
			pu_path_to_string(&dat_path, &str);
			printf("Dir path: %s\n", str.p);
			
			/* Saving to a folder */
			dat_tool_extract_to_folder(dat, &dat_path);
			
			/* Free the dat structure */
			dat_free(dat);
			
			printf("\nUnpacking done without issues (I hope)\n");
		}
	}
	else if(pu_is_dir(argv[1])) /* It's a directory so let's create a DAT file */
	{
		DAT_FILE dat = {0};
		
		/* Load files from folder to the entries list */
		dat.entries = dat_tool_load_files_from_dir(argv[1]);
		
		/* Update DAT structures with entries */
		dat_update(&dat, val_block_size);
		
		/* Print the created DAT */
		dat_tool_print_dat(&dat);
		
		/* Convert DAT to FU_FILE */
		printf("Preparing the file. This might take a moment.\n");
		FU_FILE* file = dat_save_to_fu_file(&dat, dat_tool_endian);
		
		/* Save generated DAT to file on disk */
		PU_STRING output_str = {0};
		pu_create_string(argv[1], strlen(argv[1]), &output_str);
		
		/* converting the suffix in the folder name to the 3-letter extension */
		if(flag_skip_ext_check)
		{
			pu_insert_char(".dat", 5, -1, &output_str);
		}
		else
		{
			if(output_str.p[output_str.s-4] == '_')
			{
				output_str.p[output_str.s-4] = '.';
			}
		}
		
		/* Saving to the file and overwriting */
		fu_to_file(output_str.p, file, 1);
		
		/* Free everything */
		dat_free(&dat);
		fu_close(file);
		free(file);
		
		printf("\nPacking done without issues (I hope)\n");
	}
	else /* Everything failed oops */
	{
		printf("Couldn't do anything.\n");
	}
	
	return 0;
}

/*
	Common
*/

void dat_tool_parse_arguments(int argc, char** argv)
{
	arg_node = ap_parse_argv(argv, argc, arg_list, arg_list_size);

	AP_VALUE_NODE* arg_x360 = ap_get_node_by_arg(arg_node, "--x360");
	AP_VALUE_NODE* arg_skip_ext_check = ap_get_node_by_arg(arg_node, "--skip_ext_check");
	AP_VALUE_NODE* arg_block_size = ap_get_node_by_arg(arg_node, "--block_size");
	
	if(arg_x360 != NULL) dat_tool_endian = FU_BIG_ENDIAN;
	if(arg_skip_ext_check != NULL) flag_skip_ext_check = 1;
	if(arg_block_size != NULL) val_block_size = arg_block_size->data.value.u32;
}

void dat_tool_print_usage(char* program_name)
{
	printf("Usage:\n");
	printf("\tTo unpack:\t\t%s <dat file> <options>\n", program_name);
	printf("\tTo pack:\t\t%s <directory> <options>\n", program_name);
	printf("\n");
	printf("Options:\n");
	printf("\tPacking and unpacking:\n");
	printf("\t\t%24s\t%s\n", "--x360", "Process the DAT with Xbox 360 in mind (big endian)");
	printf("\tPacking:\n");
	printf("\t\t%24s\t%s\n", "--skip_ext_check", "Do not get the extension from the folder suffix");
	printf("\t\t%24s\t%s\n", "--block_size", "Block size to be used in the resulting DAT (default is 2048)");
}

void dat_tool_print_dat(DAT_FILE* dat)
{	
	printf("File count: %u\n", dat->header.file_count);
	printf("Positions offset: 0x%x\n", dat->header.positions_offset);
	printf("Extensions offset: 0x%x\n", dat->header.extensions_offset);
	printf("Names offset: 0x%x\n", dat->header.names_offset);
	printf("Sizes offset: 0x%x\n", dat->header.sizes_offset);
	printf("Hashes offset: 0x%x\n", dat->header.hashes_offset);
	printf("\n");
	printf("Entry name size: %u\n", dat->entry_name_size);
	printf("\n");
	printf("Prehash shift: %u\n", dat->hashes.prehash_shift);
	printf("Bucket offsets offset: 0x%x\n", dat->hashes.bucket_offset);
	printf("Hashes offset: 0x%x\n", dat->hashes.hashes_offset);
	printf("Indices offset: 0x%x\n", dat->hashes.indices_offset);
	printf("\n");

	DBL_LIST_NODE* node = dat->entries;
	while(node != NULL)
	{
		DAT_FILE_ENTRY* entry = (DAT_FILE_ENTRY*)node->data;
		printf("\tPosition: %u\n", entry->position);
		printf("\tSize: %u\n", entry->size);
		printf("\tExtension: %.4s\n", entry->extension);
		printf("\tName: %.*s\n", dat->entry_name_size, entry->name);
		printf("\tData: %.*s\n\n", 4, &entry->data[0]);
		node = node->next;
	}
	
	for(uint32_t i = 0; i != dat->hashes.bucket_size; ++i)
	{
		printf("\t[%u] Bucket: %u\n", i, dat->hashes.bucket[i]);
	}
	
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		printf("[%u] Hash: %08x %u %u\n", i, dat->hashes.hashes[i].hash, dat->hashes.hashes[i].pos, dat->hashes.hashes[i].index);
	}
}

/* 
	Unpacker
*/
DAT_FILE* dat_tool_parse_file(const char* filepath)
{
	FU_FILE dat_file = {0};
	fu_open_file(filepath, 1, &dat_file);
	
	DAT_FILE* dat = dat_parse(&dat_file, dat_tool_endian);
	
	fu_close(&dat_file);
	
	return dat;
}

void dat_tool_extract_to_folder(DAT_FILE* dat, PU_PATH* folder)
{
	pu_create_dir(folder);
	
	for(uint32_t i = 0; i != dat->header.file_count; ++i)
	{
		DAT_FILE_ENTRY* entry = dat_get_entry_node(dat->entries, i);
		
		PU_STRING temp_file_str = {0};
		pu_path_to_string(folder, &temp_file_str);
		pu_insert_char("/", 1, -1, &temp_file_str);
		pu_insert_char((const char*)entry->name, strlen((const char*)entry->name),
					   -1, &temp_file_str);
		
		FU_FILE temp_file = {0};
		fu_create_mem_file(&temp_file);
		fu_write_data(&temp_file, entry->data, entry->size);
		fu_to_file(temp_file_str.p, &temp_file, 1);
		
		fu_close(&temp_file);
		pu_free_string(&temp_file_str);
	}
}

/* 
	Packer
*/
DBL_LIST_NODE* dat_tool_load_files_from_dir(const char* dir)
{
	DBL_LIST_NODE* head = NULL;
	DL_DIR_LIST dirlist = {0};
	dl_parse_directory(dir, &dirlist);
	
	for(uint32_t i = 0; i != dirlist.size; ++i)
	{
		if(dirlist.entries[i].type == DL_TYPE_FILE)
		{
			uint32_t size = 0;
			uint8_t extension[4] = {0};
			uint8_t* name = NULL;
			uint32_t name_size = 0;
			uint8_t* data = NULL;
			
			PU_STRING file_name = {0};
			pu_path_to_string(&dirlist.entries[i].path, &file_name);
			
			PU_STRING file_dir_path = {0};
			pu_path_to_string(&dirlist.path, &file_dir_path);
			pu_insert_char("/", 1, -1, &file_dir_path);
			pu_insert_char(file_name.p, file_name.s, -1, &file_dir_path);
			
			/* Loading the file */
			FU_FILE dat_file = {0};
			fu_open_file(file_dir_path.p, 1, &dat_file);

			data = (uint8_t*)dat_file.buf;
			size = dat_file.size;
			
			/* Name and extension */
			name = (uint8_t*)file_name.p;
			name_size = file_name.s;
			
			if(dirlist.entries[i].path.ext.s >= 3)
			{
				memcpy(&extension[0],
					   dirlist.entries[i].path.ext.p,
					   3);
			}
			else
			{
				memcpy(&extension[0],
					   dirlist.entries[i].path.ext.p,
					   dirlist.entries[i].path.ext.s);
			}
			
			/* Append new entry to the list */
			DAT_FILE_ENTRY* entry = dat_entry_from_data(0, extension, name_size, name, size, data);
			head = dat_append_entry(head, entry);
			
			/* Cleanup */
			fu_close(&dat_file);
			pu_free_string(&file_name);
			pu_free_string(&file_dir_path);
		}
	}
	
	dl_free_list(&dirlist);
	
	return head;
}