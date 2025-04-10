#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>
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

void wta_tool_parse_arguments(int argc, char** argv);

/*
	Common
*/
void wta_tool_print_usage(char* program_name);
void wta_tool_print_wtb(WTB_FILE* wtb);

/* 
	Unpacker
*/
void wta_tool_extract_to_folder(WTB_FILE* wtp, PU_PATH* folder);

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
		wta_tool_print_usage(argv[0]);
		return 0;
	}
	
	wta_tool_parse_arguments(argc, argv);
	
	/* It's a file so let's process WTA/WTP files */
	if(pu_is_file(argv[1]) && pu_is_file(argv[2]))
	{		
		char* str_wta = NULL;
		char* str_wtp = NULL;
		
		/* Let's figure out which file is which */
		PU_PATH* arg1_path = pu_split_path(argv[1], strlen(argv[1]));
		PU_PATH* arg2_path = pu_split_path(argv[2], strlen(argv[2]));

		if(strncmp(arg1_path->ext->ptr, "wta", 3) == 0)
		{
			str_wta = &argv[1][0];
			str_wtp = &argv[2][0];
		}
		else if(strncmp(arg2_path->ext->ptr, "wta", 3) == 0)
		{
			str_wta = &argv[2][0];
			str_wtp = &argv[1][0];
		}

		pu_free_path(arg1_path);
		pu_free_path(arg2_path);
		
		/* Open the files */
		FU_FILE file_wta = {0};
		FU_FILE file_wtp = {0};
		fu_open_file(str_wta, 1, &file_wta);
		fu_open_file(str_wtp, 1, &file_wtp);
		
		/* Process the files to a WTB_FILE structure */
		WTB_FILE* wtb = wtb_parse_wta_wtp(&file_wta, &file_wtp);
		
		/* Free memory */
		fu_close(&file_wta);
		fu_close(&file_wtp);
		
		if(wtb == NULL)
		{
			printf("Couldn't process WTA/WTP files.\n");
		}
		else
		{
			/*printf("Amount of DDS files: %u\n", wtb->header.tex_count);*/
			wta_tool_print_wtb(wtb);
			
			/* Directory name */
			PU_PATH* wtp_path = pu_split_path(argv[1], strlen(argv[1]));
			su_remove(wtp_path->ext, 0, -1);
			wtp_path->type = PU_PATH_TYPE_DIR;
			su_insert_char(wtp_path->name, -1, "_wtp", 4);
			
			SU_STRING* str = pu_path_to_string(wtp_path);
			printf("Dir path: %s\n", str->ptr);
			
			/* Save to a directory */
			wta_tool_extract_to_folder(wtb, wtp_path);
			
			/* Free all of it */
			wtb_free(wtb);
            pu_free_path(wtp_path);
			
			printf("\nUnpacking done without issues (I hope)\n");
		}
	}
	else if(pu_is_dir(argv[1])) /* It's a directory so let's create WTA/WTP files */
	{
		/* Create WTB from a directory of files */
		WTB_FILE* wtb = wtb_parse_directory(argv[1]);
		
		if(wtb == NULL)
		{
			printf("Couldn't process the directory.\n");
		}
		else
		{
			FU_FILE fwta = {0};
			FU_FILE fwtp = {0};
			wtb_save_wta_wtp_to_fu_files(wtb, &fwta, &fwtp);
			
			uint32_t arg1_len = strlen(argv[1]);
			
			/* Change the directory path length to skip the suffix */
			if(flag_skip_ext_check == 0 && argv[1][arg1_len-4] == '_')
			{
				arg1_len -= 4;
			}
			
			/* Create path strings for the files */
			SU_STRING* output_str_wta = su_create_string(argv[1], arg1_len);
			su_insert_char(output_str_wta, -1, ".wta", 5);
			
			SU_STRING* output_str_wtp = su_create_string(argv[1], arg1_len);
			su_insert_char(output_str_wtp, -1, ".wtp", 5);
			
			/* Save generated WTA/WTP to file on disk */
			fu_to_file(output_str_wta->ptr, &fwta, 1);
			fu_to_file(output_str_wtp->ptr, &fwtp, 1);
			
			su_free(output_str_wta);
			su_free(output_str_wtp);
			
			wtb_free(wtb);
			
			fu_close(&fwta);
			fu_close(&fwtp);
			
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
void wta_tool_parse_arguments(int argc, char** argv)
{
	arg_node = ap_parse_argv(argv, argc, arg_list, arg_list_size);

	AP_VALUE_NODE* arg_skip_ext_check = ap_get_node_by_arg(arg_node, "--skip_ext_check");

	if(arg_skip_ext_check != NULL)
	{
		flag_skip_ext_check = 1;
	}
}

void wta_tool_print_usage(char* program_name)
{
	printf("Usage:\n");
	printf("\tTo unpack: %s file.wta file.wtp\n", program_name);
	printf("\tTo pack: %s <directory with DDS files>\n", program_name);
	printf("\n");
	printf("Options:\n");
	printf("\tPacking:\n");
	printf("\t\t%24s\t%s\n", "--skip_ext_check", "Do not remove the extension from the folder suffix");
	printf("\n");
	printf("DDS filenames in the unpacked directory are the texture IDs in decimal format.\n");
	printf("Only change them if you know what you are doing.\n");
}

void wta_tool_print_wtb(WTB_FILE* wtb)
{
	printf("Platform: %s\n", wtb->platform == 1 ? "PC" : "X360");
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
void wta_tool_extract_to_folder(WTB_FILE* wtb, PU_PATH* folder)
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
		SU_STRING* temp_file_str = pu_path_to_string(folder);
		su_insert_char(temp_file_str, -1, "/", 1);
		su_insert_char(temp_file_str, -1, id_str, len);

		FU_FILE temp_file = {0};
		fu_create_mem_file(&temp_file);
		fu_write_data(&temp_file, wtb->entries[i].data, wtb->entries[i].size);
		fu_seek(&temp_file, 0, FU_SEEK_SET);
		
		if(wtb->platform == FU_BIG_ENDIAN)
		{
			su_insert_char(temp_file_str, -1, ".png", 4);
			
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
			
			stbi_write_png(temp_file_str->ptr, img->width, img->height, img->bpp, img_data, img->width*img->bpp);
			
			free(img_data);
			img_free_image(img);
		}
		else
		{
			su_insert_char(temp_file_str, -1, ".dds", 4);
			fu_to_file(temp_file_str->ptr, &temp_file, 1);
		}
		
		//printf("Path: %s\n", temp_file_str.p);
		
		fu_close(&temp_file);
		su_free(temp_file_str);
	}
}

/* 
	Packer
*/