#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/cpu/endianness.h>
#include <kwaslib/platinum/wtb.h>

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
		PU_PATH arg1_path = {0};
		PU_PATH arg2_path = {0};
		pu_split_path(argv[1], strlen(argv[1]), &arg1_path);
		pu_split_path(argv[2], strlen(argv[2]), &arg2_path);

		if(strncmp(arg1_path.ext.p, "wta", 3) == 0)
		{
			str_wta = &argv[1][0];
			str_wtp = &argv[2][0];
		}
		else if(strncmp(arg2_path.ext.p, "wta", 3) == 0)
		{
			str_wta = &argv[2][0];
			str_wtp = &argv[1][0];
		}

		pu_free_path(&arg1_path);
		pu_free_path(&arg2_path);
		
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
			PU_PATH wtp_path = {0};
			pu_split_path(argv[1], strlen(argv[1]), &wtp_path);
			pu_free_string(&wtp_path.ext);
			wtp_path.type = PU_PATH_TYPE_DIR;
			pu_insert_char("_wtp", 4, -1, &wtp_path.name);
			
			PU_STRING str = {0};
			pu_path_to_string(&wtp_path, &str);
			printf("Dir path: %s\n", str.p);
			
			/* Save to a directory */
			wta_tool_extract_to_folder(wtb, &wtp_path);
			
			/* Free all of it */
			wtb_free(wtb);
			
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
			PU_STRING output_str_wta = {0};
			pu_create_string(argv[1], arg1_len, &output_str_wta);
			pu_insert_char(".wta", 5, -1, &output_str_wta);
			
			PU_STRING output_str_wtp = {0};
			pu_create_string(argv[1], arg1_len, &output_str_wtp);
			pu_insert_char(".wtp", 5, -1, &output_str_wtp);
			
			/* Save generated WTA/WTP to file on disk */
			fu_to_file(output_str_wta.p, &fwta, 1);
			fu_to_file(output_str_wtp.p, &fwtp, 1);
			
			pu_free_string(&output_str_wta);
			pu_free_string(&output_str_wtp);
			
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
	printf("Texture info offset array: %x\n\n", wtb->header.tex_info_offset);
	
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		printf("Texture [%u/%u]\n", i+1, wtb->header.tex_count);
		printf("\tOffset %u\n", wtb->entries[i].offset);
		printf("\tSize %u\n", wtb->entries[i].size);
		printf("\tID %u/%x\n", wtb->entries[i].id, wtb->entries[i].id);
		
		if(wtb->header.tex_info_offset)
		{
			WTB_X360_INFO* xo = &wtb->entries[i].x360;
			printf("    XBOX 360 INFO:\n");
			printf("\tUnk set: %08x %08x %08x %08x %08x %08x %08x\n",
			       xo->unk_0, xo->unk_4, xo->unk_8, xo->unk_C,
				   xo->unk_10, xo->unk_14, xo->unk_18);
			printf("\tUnk_19: %x\n", xo->unk_19);
			printf("\tStride: %x / %u\n", xo->stride, xo->stride*128);
			printf("\tFlags: %02x%02x%02x\n", xo->flags[0], xo->flags[1], xo->flags[2]);

			printf("\tunk_23: %x\n", xo->unk_23);
			printf("\tSurface fmt: %x / %s\n", xo->surface_fmt, x360_fmt_str[xo->surface_fmt]);
			printf("\tResolution: %x / %ux%u\n", xo->packed_res,
												 xo->unpacked_width,
												 xo->unpacked_height);
			
			printf("\tSome fmt 1: %x / %s\n", xo->some_fmt_again_1, x360_fmt_str[xo->some_fmt_again_1]);
			printf("\tSome fmt 2: %x / %s\n", xo->some_fmt_again_2, x360_fmt_str[xo->some_fmt_again_2]);
			
			printf("\tMipmap stuff 1: %x / %u\n", xo->mipmap_stuff_1, xo->mipmap_stuff_1);
			printf("\tMipmap stuff 2: %x / %u\n", xo->mipmap_stuff_2, xo->mipmap_stuff_2);
			
			printf("\tunk_31: %x / %u\n", xo->unk_31, xo->unk_31);
			printf("\tunk_32: %x / %u\n", xo->unk_32, xo->unk_32);
		}
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
		/*printf("%u\n", wtb->entries[i].id);*/
		const uint32_t len = strlen(&id_str[0]);
		
		/*  Construct the path */
		PU_STRING temp_file_str = {0};
		pu_path_to_string(folder, &temp_file_str);
		pu_insert_char("/", 1, -1, &temp_file_str);
		pu_insert_char(id_str, len, -1, &temp_file_str);
		pu_insert_char(".dds", 4, -1, &temp_file_str);
		
		FU_FILE temp_file = {0};
		fu_create_mem_file(&temp_file);
		fu_write_data(&temp_file, wtb->entries[i].data, wtb->entries[i].size);
		fu_to_file(temp_file_str.p, &temp_file, 1);
		
		//printf("Path: %s\n", temp_file_str.p);
		
		fu_close(&temp_file);
		pu_free_string(&temp_file_str);
	}
}

/* 
	Packer
*/