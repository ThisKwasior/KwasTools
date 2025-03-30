#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/string_utils.h>
#include <kwaslib/core/io/dir_list.h>
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
WTB_FILE* wtb_tool_parse_directory(const char* dir);

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
			PU_PATH* wtb_path = pu_split_path(argv[1], strlen(argv[1]));
			su_remove(wtb_path->ext, 0, -1);
			wtb_path->type = PU_PATH_TYPE_DIR;
			su_insert_char(wtb_path->name, -1, "_wtb", 4);
			
			SU_STRING* str = pu_path_to_string(wtb_path);
			printf("Dir path: %s\n", str->ptr);
			
			/* Save to a directory */
			wtb_tool_extract_to_folder(wtb, wtb_path);
			
			/* Free all of it */
			wtb_free(wtb);
            pu_free_path(wtb_path);
            su_free(str);
			
			printf("\nUnpacking done without issues (I hope)\n");
		}
	}
	else if(pu_is_dir(argv[1])) /* It's a directory so let's create WTB file */
	{
		/* Create WTB from a directory of files */
		WTB_FILE* wtb = wtb_tool_parse_directory(argv[1]);
		
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
			SU_STRING* output_str_wtb = su_create_string(argv[1], arg1_len);
			su_insert_char(output_str_wtb, -1, ".wtb", 5);
			
			fu_to_file(output_str_wtb->ptr, &fwtb, 1);
			
			su_free(output_str_wtb);
			
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
	printf("Flags offset array: %x\n", wtb->header.flag_array_offset);
	printf("Texture ID offset array: %x\n", wtb->header.tex_id_array_offset);
	printf("Texture info offset array: %x\n\n", wtb->header.xpr_info_offset);
	
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        
		printf("Texture [%u/%u]\n", i+1, wtb->header.tex_count);
		printf("\tOffset %u\n", entry->offset);
		printf("\tSize %u\n", entry->size);
		printf("\tID %u/%x\n", entry->id, entry->id);
		printf("\tFlags: %02x0000%02x\n", *(const uint8_t*)&entry->flags.b0,
                                          *(const uint8_t*)&entry->flags.b3);
		printf("\t\talways2_0 - %u\n", entry->flags.b0.always2_0);
		printf("\t\tcomplex   - %u\n", entry->flags.b0.complex);
		printf("\t\tcubemap   - %u\n", entry->flags.b3.cubemap);
		printf("\t\talways2_1 - %u\n", entry->flags.b3.always2_1);
		printf("\t\talpha     - %u\n", entry->flags.b3.alpha);
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
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        
		/* Get the id as a string */
		char id_str[11] = {0};
		sprintf(&id_str[0], "%u", entry->id);
		const uint32_t len = strlen(&id_str[0]);
		
		/*  Construct the path */
		SU_STRING* temp_file_str = pu_path_to_string(folder);
		su_insert_char(temp_file_str, -1, "/", 1);
		su_insert_char(temp_file_str, -1, id_str, len);

		FU_FILE temp_file = {0};
		fu_create_mem_file(&temp_file);
		fu_write_data(&temp_file, entry->data, entry->size);
		fu_seek(&temp_file, 0, FU_SEEK_SET);
		
		if(wtb->platform == WTB_PLATFORM_BE)
		{
			su_insert_char(temp_file_str, -1, ".png", 4);
			
			IMAGE* img = NULL;
			uint64_t img_size = 0;
			uint8_t* img_data = NULL;
			
			if(wtb->header.xpr_info_offset) /* X360 */
			{
				img = x360_texture_to_image(entry->x360,
                                            (uint8_t*)temp_file.buf,
                                            temp_file.size);
			}
			else /* PS3 */
			{
				GTF_FILE gtf = gtf_read_file(&temp_file);
				img = gtf_to_image(&gtf);
			}
			
			img_data = img_to_raw_data(img, &img_size);
			
			stbi_write_png(temp_file_str->ptr,
                           img->width, img->height,
                           img->bpp, img_data,
                           img->width*img->bpp);
			
			free(img_data);
			img_free_image(img);
		}
		else
		{
            if(strncmp((const char*)&entry->data[6], "JFIF", 4) == 0)
            {
                su_insert_char(temp_file_str, -1, ".jpg", 4);
            }
            else if(strncmp((const char*)&entry->data[6], "Exif", 4) == 0)
            {
                su_insert_char(temp_file_str, -1, ".jpg", 4);
            }
            else if(strncmp((const char*)&entry->data[1], "PNG", 3) == 0)
            {
                su_insert_char(temp_file_str, -1, ".png", 4);
            }
            else 
            {
                su_insert_char(temp_file_str, -1, ".dds", 4);
            }
            
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
WTB_FILE* wtb_tool_parse_directory(const char* dir)
{
    DL_DIR_LIST dirlist = {0};
    dl_parse_directory(dir, &dirlist);
    
    WTB_FILE* wtb = wtb_alloc_empty_wtb();
    cvec_resize(wtb->entries, dirlist.file_count);
    
    uint32_t entry_it = 0;
    for(uint32_t i = 0; i != dirlist.size; ++i)
    {
        if(dirlist.entries[i].type == DL_TYPE_FILE)
        {
            WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, entry_it);
            SU_STRING* path = dl_get_full_entry_path(&dirlist, i);
            
            FU_FILE temp_file = {0};
            fu_open_file(path->ptr, 1, &temp_file);
        
            entry->size = temp_file.size;
            sscanf(dirlist.entries[i].path->name->ptr, "%u", &entry->id);

            entry->data = (uint8_t*)calloc(1, entry->size);
            memcpy(entry->data, temp_file.buf, entry->size);
        
            su_free(path);
            fu_close(&temp_file);
            
            entry_it += 1;
        }
    }
    
    return wtb;
}