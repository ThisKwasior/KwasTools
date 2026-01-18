#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/cpu/endianness.h>
#include <kwaslib/core/math/boundary.h>
#include <kwaslib/platinum/dat.h>
#include <kwaslib/platinum/wtb.h>

/*
	Common
*/
void dat2dds2dat_tool_print_usage(char* program_name);
void dat2dds2dat_tool_fix_positions(DAT_FILE* dat);

/* 
	Unpacker
*/
DAT_FILE* dat2dds2dat_tool_parse_file(const char* filepath);
void dat2dds2dat_tool_extract_to_folder(WTB_FILE* wtb, PU_PATH* folder);

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
		dat2dds2dat_tool_print_usage(argv[0]);
		return 0;
	}

	uint32_t file_wta = -1;
	uint32_t file_wtp = -1;
	uint32_t file_wtb = -1;
	DAT_FILE_ENTRY* entry_wta = NULL;
	DAT_FILE_ENTRY* entry_wtp = NULL;
	DAT_FILE_ENTRY* entry_wtb = NULL;

	/* It's a directory so let's replace the wta/wtp or wtb file */
	if(pu_is_dir(argv[1]))
	{
		/* Create WTB from a directory of files */
		WTB_FILE* wtb = wtb_parse_directory(argv[1]);
		
		/* Get the folder name without suffix */
		uint32_t wtb_path_len = strlen(argv[1]);
		if(argv[1][wtb_path_len-4] == '_') wtb_path_len -= 4;
		PU_PATH wtb_pu_path = {0};
		pu_split_path(argv[1], wtb_path_len, &wtb_pu_path);
		printf("Name: %s\n", wtb_pu_path.name.p);
		
		if(wtb == NULL)
		{
			printf("Couldn't process the directory.\n");
		}
		else
		{
			/* Load DAT files */
			const uint32_t dat_count = argc - 2;
			DAT_FILE** dats = (DAT_FILE**)calloc(dat_count, sizeof(DAT_FILE));
			
			for(uint32_t i = 0; i != dat_count; ++i)
			{
				dats[i] = dat2dds2dat_tool_parse_file(argv[2+i]);
			}
			
			/* Search for texture files */
			for(uint32_t i = 0; i != dat_count; ++i)
			{
				if(dats[i] != NULL)
				{
					DAT_FILE* dat = dats[i];
					for(uint32_t j = 0; j != dat->header.files_amount; ++j)
					{
						printf("[%u/%u] %s %s\n", j+1, dat->header.files_amount,
												  dat->entries[j].name, dat->entries[j].extension);
						printf("SIZES %x\n", dat->header.sizes_offset);
						printf("HASHES %x\n", dat->header.hashes_offset);
						printf("PREHASH %u\n", dat->prehash_shift);
												  
						if(strncmp((const char*)dat->entries[j].name, wtb_pu_path.name.p, wtb_pu_path.name.s) == 0)
						{
							if(strncmp((const char*)dat->entries[j].extension, "wta", 3) == 0)
							{
								printf("Found WTA %s %s\n", dat->entries[j].name, dat->entries[j].extension);
								file_wta = i;
								entry_wta = &dat->entries[j];
							}
							else if(strncmp((const char*)&dat->entries[j].extension[0], "wtp", 3) == 0)
							{
								printf("Found WTP %s %s\n", dat->entries[j].name, dat->entries[j].extension);
								file_wtp = i;
								entry_wtp = &dat->entries[j];
							}
							else if(strncmp((const char*)&dat->entries[j].extension[0], "wtb", 3) == 0)
							{
								printf("Found WTB %s %s\n", dat->entries[j].name, dat->entries[j].extension);
								file_wtb = i;
								entry_wtb = &dat->entries[j];
							}
						}
					}
				}
			}
			
			/* Replace files */
			if(entry_wta && entry_wtp)
			{
				FU_FILE fwta = {0};
				FU_FILE fwtp = {0};
				wtb_save_wta_wtp_to_fu_files(wtb, &fwta, &fwtp);
				
				entry_wta->size = fwta.size;
				entry_wtp->size = fwtp.size;
				
				free(entry_wta->data);
				free(entry_wtp->data);
				
				entry_wta->data = (uint8_t*)calloc(1, fwta.size);
				entry_wtp->data = (uint8_t*)calloc(1, fwtp.size);
				
				memcpy(entry_wta->data, fwta.buf, fwta.size);
				memcpy(entry_wtp->data, fwtp.buf, fwtp.size);
				
				dat2dds2dat_tool_fix_positions(dats[file_wta]);
				dat2dds2dat_tool_fix_positions(dats[file_wtp]);
				
				FU_FILE* fdatwta = dat_save_to_fu_file(dats[file_wta], FU_HOST_ENDIAN);
				FU_FILE* fdatwtp = dat_save_to_fu_file(dats[file_wtp], FU_HOST_ENDIAN);
				
				fu_to_file(argv[file_wta+2], fdatwta, 1);
				fu_to_file(argv[file_wtp+2], fdatwtp, 1);
				
				fu_close(&fwta);
				fu_close(&fwtp);
				fu_close(fdatwta);
				fu_close(fdatwtp);
				free(fdatwta);
				free(fdatwtp);
			}
			else if(entry_wtb)
			{
				FU_FILE fwtb = {0};
				wtb_save_wtb_to_fu_file(wtb, &fwtb);
				
				entry_wtb->size = fwtb.size;
				
				free(entry_wtb->data);
				
				entry_wtb->data = (uint8_t*)calloc(1, fwtb.size);
				
				memcpy(entry_wtb->data, fwtb.buf, fwtb.size);
				
				dat2dds2dat_tool_fix_positions(dats[file_wtb]);
				
				FU_FILE* fdatwtb = dat_save_to_fu_file(dats[file_wtb], FU_HOST_ENDIAN);
				
				fu_to_file(argv[file_wtb+2], fdatwtb, 1);
				
				fu_close(&fwtb);
				fu_close(fdatwtb);
				free(fdatwtb);
			}
			else
			{
				printf("Missing files.\n"); 
			}		
			
			/* Cleanup */
			for(uint32_t i = 0; i != dat_count; ++i) 
				if(dats[i] != NULL)
					dat_free_dat(dats[i]);
			free(dats);
			wtb_free(wtb);
		}
	}
	else if(pu_is_file(argv[1]))
	{
		/* Get the dat name without suffix */
		uint32_t dat_path_len = strlen(argv[1]);
		if(argv[1][dat_path_len-4] == '_') dat_path_len -= 4;
		PU_PATH dat_pu_path = {0};
		pu_split_path(argv[1], dat_path_len, &dat_pu_path);
		pu_free_string(&dat_pu_path.ext);
		printf("Name: %s\n", dat_pu_path.name.p);
		
		/* Load DAT files */
		const uint32_t dat_count = argc - 1;
		DAT_FILE** dats = (DAT_FILE**)calloc(dat_count, sizeof(DAT_FILE));
		
		for(uint32_t i = 0; i != dat_count; ++i)
		{
			dats[i] = dat2dds2dat_tool_parse_file(argv[1+i]);
		}
		
		/* Search for texture files */
		for(uint32_t i = 0; i != dat_count; ++i)
		{
			if(dats[i] != NULL)
			{
				DAT_FILE* dat = dats[i];
				for(uint32_t j = 0; j != dat->header.files_amount; ++j)
				{
					printf("[%u/%u] %s %s\n", j+1, dat->header.files_amount,
											  dat->entries[j].name, dat->entries[j].extension);
											  
					if(strncmp((const char*)dat->entries[j].name, dat_pu_path.name.p, dat_pu_path.name.s) == 0)
					{
						if(strncmp((const char*)dat->entries[j].extension, "wta", 3) == 0)
						{
							printf("Found WTA %s %s\n", dat->entries[j].name, dat->entries[j].extension);
							file_wta = i;
							entry_wta = &dat->entries[j];
						}
						else if(strncmp((const char*)&dat->entries[j].extension[0], "wtp", 3) == 0)
						{
							printf("Found WTP %s %s\n", dat->entries[j].name, dat->entries[j].extension);
							file_wtp = i;
							entry_wtp = &dat->entries[j];
						}
						else if(strncmp((const char*)&dat->entries[j].extension[0], "wtb", 3) == 0)
						{
							printf("Found WTB %s %s\n", dat->entries[j].name, dat->entries[j].extension);
							file_wtb = i;
							entry_wtb = &dat->entries[j];
						}
					}
				}
			}
		}
		
		WTB_FILE* wtb = NULL;
		
		if(entry_wta && entry_wtp)
		{
			FU_FILE fwta = {0};
			FU_FILE fwtp = {0};
			
			fwta.is_buf = 1;
			fwta.buf = (char*)entry_wta->data;
			fwta.size = entry_wta->size;
			fwta.rem = entry_wta->size;
			
			fwtp.is_buf = 1;
			fwtp.buf = (char*)entry_wtp->data;
			fwtp.size = entry_wtp->size;
			fwtp.rem = entry_wtp->size;
			
			wtb = wtb_parse_wta_wtp(&fwta, &fwtp);
			
			pu_insert_char("_wtp", 4, -1, &dat_pu_path.name);
		}
		else if(entry_wtb)
		{
			FU_FILE fwtb = {0};
			
			fwtb.is_buf = 1;
			fwtb.buf = (char*)entry_wtb->data;
			fwtb.size = entry_wtb->size;
			fwtb.rem = entry_wtb->size;

			wtb = wtb_parse_wtb(&fwtb);
			
			pu_insert_char("_wtb", 4, -1, &dat_pu_path.name);
		}
		
		if(wtb)
		{
			dat2dds2dat_tool_extract_to_folder(wtb, &dat_pu_path);
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

void dat2dds2dat_tool_print_usage(char* program_name)
{
	printf("Usage:\n");
	printf("\tTo unpack:\t\t%s <dat files>\n", program_name);
	printf("\tTo pack:\t\t%s <directory with DDS files> <dat files>\n", program_name);
	printf("\n");
}

void dat2dds2dat_tool_fix_positions(DAT_FILE* dat)
{
	const uint32_t temp_data_pos = dat->header.hashes_offset
								   + dat->indices_offset
								   + 2*dat->header.files_amount;
	
	uint64_t bytes_to_pad = bound_calc_leftover(DAT_BLOCK_SIZE, temp_data_pos);
	
	const uint32_t first_data_pos = temp_data_pos + bytes_to_pad;
	uint32_t cur_data_pos = first_data_pos;
	
	for(uint32_t i = 0; i != dat->header.files_amount; ++i)
	{
		dat->entries[i].position = cur_data_pos;
		
		cur_data_pos += dat->entries[i].size;
		bytes_to_pad = bound_calc_leftover(DAT_BLOCK_SIZE, cur_data_pos);
		cur_data_pos += bytes_to_pad;
	}
}

/* 
	Unpacker
*/
DAT_FILE* dat2dds2dat_tool_parse_file(const char* filepath)
{
	FU_FILE dat_file = {0};
	fu_open_file(filepath, 1, &dat_file);
	
	DAT_FILE* dat = dat_parse_dat(&dat_file, FU_HOST_ENDIAN);
	
	fu_close(&dat_file);
	
	return dat;
}

void dat2dds2dat_tool_extract_to_folder(WTB_FILE* wtb, PU_PATH* folder)
{
	PU_STRING str = {0};
	pu_path_to_string(folder, &str);
	pu_create_dir_char(str.p);
	
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		/* Get the id as a string */
		char id_str[11] = {0};
		sprintf(&id_str[0], "%u", wtb->entries[i].id);
		printf("%u\n", wtb->entries[i].id);
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