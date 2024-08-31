#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/cpu/endianness.h>
#include <kwaslib/core/data/image/image.h>
#include <kwaslib/core/data/image/gtf.h>
#include <kwaslib/platinum/wtb.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <kwaslib/ext/stb_image_write.h>

/*
	Arguments
*/
const AP_ARG_DESC arg_list[] =
{
	{"--skip_ext_check", AP_TYPE_NOV}
};
const uint32_t arg_list_size = 1;

AP_VALUE_NODE* arg_node = NULL;

/* Flags */
uint8_t flag_skip_ext_check = 0;

void wtb_tool_parse_arguments(int argc, char** argv);

/*
	Common
*/
void wtb_tool_print_usage(char* program_name);
void wtb_tool_print_wtb(WTB_FILE* wtb);

/* 
	Unpacker
*/
void wtb_tool_extract_to_folder(WTB_FILE* wtb, PU_PATH* folder);

/* 
	Packer
*/

/*
	Entry point
*/
int main(int argc, char** argv)
{
	if(argc == 1)
	{
		wtb_tool_print_usage(argv[0]);
		return 0;
	}
	
	/* It's a file so let's process WTB */
	if(pu_is_file(argv[1]))
	{
		FU_FILE file_wtb = {0};
		fu_open_file(argv[1], 1, &file_wtb);
		
		WTB_FILE* wtb = wtb_parse_wtb(&file_wtb);
		
		fu_close(&file_wtb);
		
		if(wtb == NULL)
		{
			printf("Couldn't process the WTB file.\n");
		}
		else
		{
			/*printf("Amount of DDS files: %u\n", wtb->header.tex_count);*/
			
			wtb_tool_print_wtb(wtb);
			
			/* Directory name */
			PU_PATH wtb_path = {0};
			pu_split_path(argv[1], strlen(argv[1]), &wtb_path);
			pu_free_string(&wtb_path.ext);
			wtb_path.type = PU_PATH_TYPE_DIR;
			pu_insert_char("_wtb", 4, -1, &wtb_path.name);
			
			PU_STRING str = {0};
			pu_path_to_string(&wtb_path, &str);
			printf("Dir path: %s\n", str.p);
			
			/* Save to a directory */
			wtb_tool_extract_to_folder(wtb, &wtb_path);
			
			/* Free all of it */
			wtb_free(wtb);
			
			printf("\nUnpacking done without issues (I hope)\n");
		}
	}
	else if(pu_is_dir(argv[1])) /* It's a directory so let's create WTB files */
	{
		/* Create WTB from a directory of files */
		WTB_FILE* wtb = wtb_parse_directory(argv[1]);
		
		if(wtb == NULL)
		{
			printf("Couldn't process the directory.\n");
		}
		else
		{
			FU_FILE fwtb = {0};
			wtb_save_wtb_to_fu_file(wtb, &fwtb);
			
			uint32_t arg1_len = strlen(argv[1]);
			
			/* Change the directory path length to skip the suffix */
			if(flag_skip_ext_check == 0 && argv[1][arg1_len-4] == '_')
			{
				arg1_len -= 4;
			}
			
			/* Save generated WTB to file on disk */
			PU_STRING output_str_wtb = {0};
			pu_create_string(argv[1], arg1_len, &output_str_wtb);
			pu_insert_char(".wtb", 5, -1, &output_str_wtb);
			
			fu_to_file(output_str_wtb.p, &fwtb, 1);
			
			pu_free_string(&output_str_wtb);
			
			wtb_free(wtb);
			
			fu_close(&fwtb);
			
			printf("\nPacking done without issues (I hope)\n");
		}
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
void wtb_tool_parse_arguments(int argc, char** argv)
{
	arg_node = ap_parse_argv(argv, argc, arg_list, arg_list_size);

	AP_VALUE_NODE* arg_skip_ext_check = ap_get_node_by_arg(arg_node, "--skip_ext_check");

	if(arg_skip_ext_check != NULL)
	{
		flag_skip_ext_check = 1;
	}
}

void wtb_tool_print_usage(char* program_name)
{
	printf("Usage:\n");
	printf("\tTo unpack: %s file.wtb\n", program_name);
	printf("\tTo pack: %s <directory with DDS files>\n", program_name);
	printf("\n");
	printf("Options:\n");
	printf("\tPacking:\n");
	printf("\t\t%24s\t%s\n", "--skip_ext_check", "Do not remove the extension from the folder suffix");
	printf("\n");
	printf("DDS filenames in the unpacked directory are the texture IDs in decimal format.\n");
	printf("Only change them if you know what you are doing.\n");
}

void wtb_tool_print_wtb(WTB_FILE* wtb)
{
	printf("Platform: %s\n", wtb->platform == 1 ? "PC" : "X360/PS3");
	printf("Texture count: %u\n", wtb->header.tex_count);
	printf("Texture offset array: %x\n", wtb->header.tex_offset_array_offset);
	printf("Size offset array: %x\n", wtb->header.tex_size_offset);
	printf("Unknown offset array: %x\n", wtb->header.unk_array_offset);
	printf("Texture ID offset array: %x\n", wtb->header.tex_id_array_offset);
	printf("Texture info offset array: %x\n\n", wtb->header.xpr_info_offset);
	
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		printf("Texture [%u/%u]\n", i+1, wtb->header.tex_count);
		printf("\tOffset %u\n", wtb->entries[i].offset);
		printf("\tSize %u\n", wtb->entries[i].size);
		printf("\tID %u/%x\n", wtb->entries[i].id, wtb->entries[i].id);
	}
}

/* 
	Unpacker
*/
void wtb_tool_extract_to_folder(WTB_FILE* wtb, PU_PATH* folder)
{
	pu_create_dir(folder);
	
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		/* Get the id as a string */
		char id_str[11] = {0};
		sprintf(&id_str[0], "%u", wtb->entries[i].id);
		printf("ID: %u\n", wtb->entries[i].id);
		const uint32_t len = strlen(&id_str[0]);
		
		/*  Construct the path */
		PU_STRING temp_file_str = {0};
		pu_path_to_string(folder, &temp_file_str);
		pu_insert_char("/", 1, -1, &temp_file_str);
		pu_insert_char(id_str, len, -1, &temp_file_str);

		FU_FILE temp_file = {0};
		fu_create_mem_file(&temp_file);
		fu_write_data(&temp_file, wtb->entries[i].data, wtb->entries[i].size);
		fu_seek(&temp_file, 0, FU_SEEK_SET);
		
		if(wtb->platform == FU_BIG_ENDIAN)
		{
			pu_insert_char(".png", 4, -1, &temp_file_str);
			
			IMAGE* img = NULL;
			uint64_t img_size = 0;
			uint8_t* img_data = NULL;
			
			if(wtb->header.xpr_info_offset) /* X360 */
			{
				img = x360_texture_to_image(wtb->entries[i].x360, (uint8_t*)temp_file.buf, temp_file.size);
			}
			else /* PS3 */
			{
				GTF_FILE gtf = gtf_read_file(&temp_file);
				img = gtf_to_image(&gtf);
			}
			
			img_data = img_to_raw_data(img, &img_size);
			
			stbi_write_png(temp_file_str.p, img->width, img->height, img->bpp, img_data, img->width*img->bpp);
			
			free(img_data);
			img_free_image(img);
		}
		else
		{
			pu_insert_char(".dds", 4, -1, &temp_file_str);
			fu_to_file(temp_file_str.p, &temp_file, 1);
		}
		
		//printf("Path: %s\n", temp_file_str.p);
		
		fu_close(&temp_file);
		pu_free_string(&temp_file_str);
	}
}

/* 
	Packer
*/