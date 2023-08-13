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
};
const uint32_t arg_list_size = 2;

AP_VALUE_NODE* arg_node = NULL;

/* Flags */
uint8_t dat_tool_endian = FU_HOST_ENDIAN;
uint8_t flag_skip_ext_check = 0;

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
			
			/* Free this guy */
			dat_free_dat(dat);
			
			printf("\nUnpacking done without issues (I hope)\n");
		}
	}
	else if(pu_is_dir(argv[1])) /* It's a directory so let's create a DAT file */
	{
		/* Create DAT from a directory of files */
		DAT_FILE* dat = dat_parse_directory(argv[1]);
		
		/* Print the created DAT */
		dat_tool_print_dat(dat);
		
		/* Convert DAT to FU_FILE */
		printf("Preparing the file. This might take a moment.\n");
		FU_FILE* file = dat_save_to_fu_file(dat, dat_tool_endian);
		
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
		dat_free_dat(dat);
		
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
	
	if(arg_x360 != NULL)
	{
		dat_tool_endian = FU_BIG_ENDIAN;
	}
	
	if(arg_skip_ext_check != NULL)
	{
		flag_skip_ext_check = 1;
	}
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
}

void dat_tool_print_dat(DAT_FILE* dat)
{	
	printf("Files amount: %u\n", dat->header.files_amount);
	printf("Positions offset: 0x%x\n", dat->header.positions_offset);
	printf("Extensions offset: 0x%x\n", dat->header.extensions_offset);
	printf("Names offset: 0x%x\n", dat->header.names_offset);
	printf("Sizes offset: 0x%x\n", dat->header.sizes_offset);
	printf("Hashes offset: 0x%x\n", dat->header.hashes_offset);
	printf("\n");
	printf("Entry name size: %u\n", dat->entry_name_size);
	printf("\n");
	printf("Prehash shift: %u\n", dat->prehash_shift);
	printf("Bucket offsets offset: 0x%x\n", dat->bucket_offsets_offset);
	printf("Hashes offset: 0x%x\n", dat->hashes_offset);
	printf("Indices offset: 0x%x\n", dat->indices_offset);
	printf("\n");

	
	for(uint32_t i = 0; i != dat->bucket_offsets_size; ++i)
	{
		printf("Bucket offset[%u]: 0x%x\n", i, dat->bucket_offsets[i]);
	}

	printf("\n");

	/*
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		printf("[%04u]: %.*s\n", i, dat->entry_name_size, dat->entries[i].name);
		printf("\tExtension: %.4s\n", dat->entries[i].extension);
		printf("\tPosition: %u\n", dat->entries[i].position);
		printf("\tSize: %u\n", dat->entries[i].size);
		printf("\tData ptr: %p\n", dat->entries[i].data);
		printf("\tHash: %08x\n", dat->hashes[i]);
		printf("\tIndex: %u\n", dat->indices[i]);
	}
	*/
}

/* 
	Unpacker
*/
DAT_FILE* dat_tool_parse_file(const char* filepath)
{
	FU_FILE dat_file = {0};
	fu_open_file(filepath, 1, &dat_file);
	
	DAT_FILE* dat = dat_parse_dat(&dat_file, dat_tool_endian);
	
	fu_close(&dat_file);
	
	return dat;
}

void dat_tool_extract_to_folder(DAT_FILE* dat, PU_PATH* folder)
{
	pu_create_dir(folder);
	
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		PU_STRING temp_file_str = {0};
		pu_path_to_string(folder, &temp_file_str);
		pu_insert_char("/", 1, -1, &temp_file_str);
		pu_insert_char((const char*)dat->entries[i].name, dat->entry_name_size, -1, &temp_file_str);
		
		FU_FILE temp_file = {0};
		fu_create_mem_file(&temp_file);
		fu_write_data(&temp_file, dat->entries[i].data, dat->entries[i].size);
		fu_to_file(temp_file_str.p, &temp_file, 1);
		
		fu_close(&temp_file);
		pu_free_string(&temp_file_str);
		
		//printf("Path: %s\n", temp_file_str.p);
	}
}

/* 
	Packer
*/