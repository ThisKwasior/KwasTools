#include "wtb.h"

#include <string.h>
#include <stdlib.h>

#include <kwaslib/utils/io/dir_list.h>
#include <kwaslib/utils/math/boundary.h>

#define WTB_BLOCK_SIZE 4096

WTB_FILE* wtb_parse_wta_wtp(FU_FILE* fwta, FU_FILE* fwtp)
{
	WTB_FILE* wtb = (WTB_FILE*)calloc(1, sizeof(WTB_FILE));
	if(wtb == NULL)
	{
		printf("Could not allocate wtb file structure\n");
		return NULL;
	}
	
	/* Reading the header */
	const uint8_t valid = wtb_read_header(fwta, wtb);
	if(valid == 0) return NULL;
	
	/* Load entries */
	wtb_populate_entries(fwta, wtb);
	wtb_load_entries_dds(fwtp, wtb);
	
	return wtb;
}

WTB_FILE* wtb_parse_wtb(FU_FILE* fwtb)
{
	return wtb_parse_wta_wtp(fwtb, fwtb);
}

WTB_FILE* wtb_parse_directory(const char* dir)
{
	DL_DIR_LIST dirlist = {0};
	dl_parse_directory(dir, &dirlist);
	
	WTB_FILE* wtb = (WTB_FILE*)calloc(1, sizeof(WTB_FILE));
	if(wtb == NULL)
	{
		printf("Could not allocate wtb file structure\n");
		return NULL;
	}
	
	strncpy((char*)wtb->header.magic, "WTB\0", 4);
	wtb->header.unknown04 = 1;
	wtb->header.tex_count = dirlist.file_count;
	
	/* 
		Allocate and populate WTB entries
		Copy-pasted from dat.c
	*/
	wtb->entries = (WTB_ENTRY*)calloc(wtb->header.tex_count, sizeof(WTB_ENTRY));
	
	uint32_t dir_it = 0;
	for(uint32_t i = 0; i != dirlist.size; ++i)
	{
		if(dirlist.entries[i].type == DL_TYPE_FILE)
		{			
			PU_STRING* file_dir_path = dl_get_full_entry_path(&dirlist, i);
			printf("file path: %s\n", file_dir_path->p);
			
			/* Open the file */
			FU_FILE temp_file = {0};
			fu_open_file(file_dir_path->p, 1, &temp_file);
			
			/* load the data to WTB entry */
			wtb->entries[dir_it].size = temp_file.size;
			
			uint64_t bytes_read = 0;
			wtb->entries[dir_it].data = (uint8_t*)calloc(1, wtb->entries[dir_it].size);
			fu_read_data(&temp_file, wtb->entries[dir_it].data,
						 wtb->entries[dir_it].size, &bytes_read);
						 
			wtb->entries[dir_it].unk = 0x20000020;
			
			//wtb->entries[dir_it].id = crc32_encode(wtb->entries[dir_it].data, wtb->entries[dir_it].size);
			sscanf(dirlist.entries[i].path.name.p, "%u", &wtb->entries[dir_it].id);
			printf("%08x\n", wtb->entries[dir_it].id);
			
			/* Freeing strings and closing the file */
			pu_free_string(file_dir_path);
			free(file_dir_path);
			fu_close(&temp_file);
			
			/* Next WTB entry */
			dir_it += 1;
		}
	}
	
	/* Figure out the entries' offsets */
	/*uint64_t cur_pos = wtb->entries[0].size;
	printf("Off: %u %u %u\n", cur_pos, wtb->entries[0].offset, wtb->entries[0].size);
	for(uint32_t i = 1; i != wtb->header.tex_count; ++i)
	{
		cur_pos += bound_calc_leftover(WTB_BLOCK_SIZE, cur_pos);
		wtb->entries[i].offset = cur_pos;
		cur_pos += wtb->entries[i].size;
		printf("Off: %u %u %u\n", cur_pos, wtb->entries[i].offset, wtb->entries[i].size);
	}*/
	
	/* Sections are padded to 16 bytes I think */
	const uint32_t section_raw_size = 4*wtb->header.tex_count;
	const uint32_t section_pad = bound_calc_leftover(16, section_raw_size);
	const uint32_t section_size = section_raw_size + section_pad;
	printf("Pad: %u %u %u\n", section_raw_size, section_pad, section_size);
	
	/* Header offsets */
	wtb->header.tex_offset_array_offset = 0x20;
	wtb->header.tex_size_offset = wtb->header.tex_offset_array_offset + section_size;
	wtb->header.unk_array_offset = wtb->header.tex_size_offset + section_size;
	wtb->header.tex_id_array_offset = wtb->header.unk_array_offset + section_size;
	
	return wtb;
}

void wtb_save_wtb_to_fu_file(WTB_FILE* wtb, FU_FILE* fwtb)
{
	fu_create_mem_file(fwtb);
	
	fu_write_data(fwtb, (uint8_t*)&wtb->header, sizeof(WTB_HEADER));
	
	/* Writing tex offsets */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwtb, wtb->entries[i].offset, wtb->platform);
	
	fu_add_to_buf_size(fwtb, wtb->header.tex_size_offset - fu_tell(fwtb));
	fu_seek(fwtb, 0, FU_SEEK_END);
	
	/* Writing sizes */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwtb, wtb->entries[i].size, wtb->platform);
	
	fu_add_to_buf_size(fwtb, wtb->header.unk_array_offset - fu_tell(fwtb));
	fu_seek(fwtb, 0, FU_SEEK_END);

	/* Writing unk array */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwtb, wtb->entries[i].unk, wtb->platform);
	
	fu_add_to_buf_size(fwtb, wtb->header.tex_id_array_offset - fu_tell(fwtb));
	fu_seek(fwtb, 0, FU_SEEK_END);
	
	/* Writing ids */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwtb, wtb->entries[i].id, wtb->platform);
	
	fu_add_to_buf_size(fwtb, bound_calc_leftover(WTB_BLOCK_SIZE, fu_tell(fwtb)));
	fu_seek(fwtb, 0, FU_SEEK_END);

	/* Write dds data and assign offsets */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		wtb->entries[i].offset = fu_tell(fwtb);
		fu_write_data(fwtb, wtb->entries[i].data, wtb->entries[i].size);
		fu_add_to_buf_size(fwtb, bound_calc_leftover(WTB_BLOCK_SIZE, fu_tell(fwtb)));
		fu_seek(fwtb, 0, FU_SEEK_END);
	}
	
	/* Writing tex offsets again */
	fu_seek(fwtb, wtb->header.tex_offset_array_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwtb, wtb->entries[i].offset, wtb->platform);
	
	fu_seek(fwtb, 0, FU_SEEK_END);
}

void wtb_save_wta_wtp_to_fu_files(WTB_FILE* wtb, FU_FILE* fwta, FU_FILE* fwtp)
{
	fu_create_mem_file(fwta);
	fu_create_mem_file(fwtp);
	
	fu_write_data(fwta, (uint8_t*)&wtb->header, sizeof(WTB_HEADER));
	
	/* Writing tex offsets */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwta, wtb->entries[i].offset, wtb->platform);
	
	fu_add_to_buf_size(fwta, wtb->header.tex_size_offset - fu_tell(fwta));
	fu_seek(fwta, 0, FU_SEEK_END);
	
	/* Writing sizes */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwta, wtb->entries[i].size, wtb->platform);
	
	fu_add_to_buf_size(fwta, wtb->header.unk_array_offset - fu_tell(fwta));
	fu_seek(fwta, 0, FU_SEEK_END);

	/* Writing unk array */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwta, wtb->entries[i].unk, wtb->platform);
	
	fu_add_to_buf_size(fwta, wtb->header.tex_id_array_offset - fu_tell(fwta));
	fu_seek(fwta, 0, FU_SEEK_END);
	
	/* Writing ids */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwta, wtb->entries[i].id, wtb->platform);
	
	fu_seek(fwta, 0, FU_SEEK_END);

	/* Write dds data and assign offsets */
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		wtb->entries[i].offset = fu_tell(fwtp);
		fu_write_data(fwtp, wtb->entries[i].data, wtb->entries[i].size);
		fu_add_to_buf_size(fwtp, bound_calc_leftover(WTB_BLOCK_SIZE, fu_tell(fwtp)));
		fu_seek(fwtp, 0, FU_SEEK_END);
	}
	
	/* Writing tex offsets again */
	fu_seek(fwta, wtb->header.tex_offset_array_offset, FU_SEEK_SET);
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
		fu_write_u32(fwta, wtb->entries[i].offset, wtb->platform);
	
	fu_seek(fwta, 0, FU_SEEK_END);
	fu_seek(fwtp, 0, FU_SEEK_END);
}

uint8_t wtb_read_header(FU_FILE* file, WTB_FILE* wtb)
{
	uint8_t status = 0;
	uint64_t bytes_read = 0;
	fu_read_data(file, (uint8_t*)&wtb->header.magic[0], 4, &bytes_read);
	
	/* 0 means it matches */
	const uint8_t is_little = strncmp("WTB\0", (const char*)&wtb->header.magic[0], 4);
	const uint8_t is_big = strncmp("\0BTW", (const char*)&wtb->header.magic[0], 4);

	if(is_little != 0 && is_big == 0) wtb->platform = FU_BIG_ENDIAN;
	else if(is_little == 0 && is_big != 0) wtb->platform = FU_LITTLE_ENDIAN;

	if(wtb->platform == 0)
	{
		printf("File is not a valid WTB file\n");
		free(wtb);
		return 0;
	}
	
	wtb->header.unknown04 = fu_read_u32(file, &status, wtb->platform);
	wtb->header.tex_count = fu_read_u32(file, &status, wtb->platform);
	wtb->header.tex_offset_array_offset = fu_read_u32(file, &status, wtb->platform);
	wtb->header.tex_size_offset = fu_read_u32(file, &status, wtb->platform);
	wtb->header.unk_array_offset = fu_read_u32(file, &status, wtb->platform);
	wtb->header.tex_id_array_offset = fu_read_u32(file, &status, wtb->platform);
	wtb->header.tex_info_offset = fu_read_u32(file, &status, wtb->platform);
	
	return 1;
}

void wtb_populate_entries(FU_FILE* file, WTB_FILE* wtb)
{
	wtb->entries = (WTB_ENTRY*)calloc(wtb->header.tex_count, sizeof(WTB_ENTRY));
	
	uint8_t status = 0;
	
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		/* Offsets */
		fu_seek(file, wtb->header.tex_offset_array_offset+(i*4), FU_SEEK_SET);
		wtb->entries[i].offset = fu_read_u32(file, &status, wtb->platform);
		
		/* Sizes */
		fu_seek(file, wtb->header.tex_size_offset+(i*4), FU_SEEK_SET);
		wtb->entries[i].size = fu_read_u32(file, &status, wtb->platform);
		
		/* Unknown */
		fu_seek(file, wtb->header.unk_array_offset+(i*4), FU_SEEK_SET);
		wtb->entries[i].unk = fu_read_u32(file, &status, wtb->platform);

		/* Ids */
		fu_seek(file, wtb->header.tex_id_array_offset+(i*4), FU_SEEK_SET);
		wtb->entries[i].id = fu_read_u32(file, &status, wtb->platform);
		
		/* XBOX 360 info */
		if(wtb->platform == FU_BIG_ENDIAN)
		{
			WTB_X360_INFO* xo = &wtb->entries[i].x360;
			
			fu_seek(file, wtb->header.tex_info_offset+(i*52), FU_SEEK_SET);
			xo->unk_0 = fu_read_u32(file, &status, wtb->platform);
			xo->unk_4 = fu_read_u32(file, &status, wtb->platform);
			xo->unk_8 = fu_read_u32(file, &status, wtb->platform);
			xo->unk_C = fu_read_u32(file, &status, wtb->platform);
			xo->unk_10 = fu_read_u32(file, &status, wtb->platform);
			xo->unk_14 = fu_read_u32(file, &status, wtb->platform);
			xo->unk_18 = fu_read_u32(file, &status, wtb->platform);
			
			const uint8_t unk_19_stride = fu_read_u8(file, &status);
			xo->unk_19 = unk_19_stride>>6;
			xo->stride = unk_19_stride;
			
			xo->flags[0] = fu_read_u8(file, &status);
			xo->flags[1] = fu_read_u8(file, &status);
			xo->flags[2] = fu_read_u8(file, &status);
			
			xo->padding_A0[0] = fu_read_u8(file, &status);
			xo->padding_A0[1] = fu_read_u8(file, &status);
			xo->padding_A0[2] = fu_read_u8(file, &status);
			
			const uint8_t unk_23_surface_fmt = fu_read_u8(file, &status);
			xo->unk_23 = unk_23_surface_fmt>>6;
			xo->surface_fmt = unk_23_surface_fmt;
			
			xo->packed_res = fu_read_u32(file, &status, wtb->platform);
			xo->unk_28 = fu_read_u16(file, &status, wtb->platform);
			xo->some_fmt_again_1 = fu_read_u8(file, &status);
			xo->some_fmt_again_2 = fu_read_u8(file, &status);
			xo->unk_2C = fu_read_u16(file, &status, wtb->platform);
			xo->mipmap_stuff_1 = fu_read_u8(file, &status);
			xo->mipmap_stuff_2 = fu_read_u8(file, &status);
			
			xo->unk_30 = fu_read_u8(file, &status);
			xo->unk_31 = fu_read_u8(file, &status);
			xo->unk_32 = fu_read_u8(file, &status);
			xo->unk_33 = fu_read_u8(file, &status);
			
			/* Unpacking the resolution */
			xo->unpacked_width = (uint32_t)xo->packed_res&0xFFF;
			xo->unpacked_height = (uint32_t)(xo->packed_res>>12)&0xFFF;
		}
	}
}

void wtb_load_entries_dds(FU_FILE* file, WTB_FILE* wtb)
{
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		wtb->entries[i].data = (uint8_t*)calloc(1, wtb->entries[i].size);
		
		fu_seek(file, wtb->entries[i].offset, FU_SEEK_SET);
		
		uint64_t bytes_read = 0;
		fu_read_data(file, wtb->entries[i].data, wtb->entries[i].size, &bytes_read);
	}
}

void wtb_free(WTB_FILE* wtb)
{
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
		if(wtb->entries[i].data)
		{
			free(wtb->entries[i].data);
		}
	}
	
	free(wtb->entries);
	free(wtb);
}