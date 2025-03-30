#include "awb.h"

#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/math/boundary.h>
#include <kwaslib/core/io/dir_list.h>

/*
	Unpack
*/

AWB_FILE* awb_read_file(FU_FILE* awb, const uint32_t offset)
{	
	AWB_FILE* afs2 = awb_alloc_file();

	if(afs2 == NULL)
	{
		printf("Could not allocate awb file structure\n");
		return NULL;
	}
	
	afs2->file_offset = offset;

	awb_read_header(awb, afs2);
	
	if(strncmp(afs2->header.magic, AWB_MAGIC, 4) != 0)
	{
		printf("Something went horribly wrong with reading AFS2 file. Oops\n");
		free(afs2);
		return NULL;
	}
	
	awb_read_entries(awb, afs2);
	
	return afs2;
}

AWB_FILE* awb_parse_directory(const char* dir)
{
	DL_DIR_LIST dirlist = {0};
	dl_parse_directory(dir, &dirlist);
	
	AWB_FILE* afs2 = awb_alloc_file();
	if(afs2 == NULL)
	{
		printf("Could not allocate AWB file structure\n");
		return NULL;
	}
	
	AWB_HEADER* h = &afs2->header;

	/* Default header values */
	awb_default_header(&afs2->header, dirlist.file_count);
	
	/* Fill out entries */
	afs2->entries = awb_alloc_entries(dirlist.file_count);
	uint32_t entries_it = 0;
	
	for(uint32_t i = 0; i != dirlist.size; ++i)
	{
		if(dirlist.entries[i].type == DL_TYPE_FILE)
		{
			SU_STRING* full_path = dl_get_full_entry_path(&dirlist, i);
			FU_FILE f = {0};
			fu_open_file(full_path->ptr, 1, &f);
			
			afs2->entries[entries_it].id = entries_it;
			afs2->entries[entries_it].size = f.size;
			afs2->entries[entries_it].data = (uint8_t*)calloc(1, f.size);
			memcpy(afs2->entries[entries_it].data, f.buf, f.size);
			
			entries_it += 1;
			
			fu_close(&f);
			full_path = su_free(full_path);
		}
	}
	
	/* Calculate offsets */
	uint32_t first_offset = AWB_ID_OFFSET + h->file_count * (h->offset_size + h->id_align);
	first_offset += bound_calc_leftover(h->alignment, first_offset);
	
	uint32_t cur_offset = first_offset;
	
	for(uint32_t i = 0; i != h->file_count; ++i)
	{	
		afs2->entries[i].offset = cur_offset;
		cur_offset += afs2->entries[i].size;
		cur_offset += bound_calc_leftover(h->alignment, cur_offset);
	}
	
	dl_free_list(&dirlist);
	
	return afs2;
}

void awb_extract_to_folder(AWB_FILE* afs2, SU_STRING* dir)
{
	if(afs2->no_data == 1) return;
	
	SU_STRING* file_str = su_create_string(dir->ptr, dir->size);
	
	pu_create_dir_char(file_str->ptr);
	
	su_insert_char(file_str, -1, "/00000.hca", 10);
	
	for(uint32_t i = 0; i != afs2->header.file_count; ++i)
	{
		sprintf(&file_str->ptr[file_str->size-9], "%05u.hca", i);
		fu_buffer_to_file(file_str->ptr, (char*)afs2->entries[i].data, afs2->entries[i].size, 1);
	}
    
    su_free(file_str);
}


void awb_read_header(FU_FILE* awb, AWB_FILE* afs2)
{
	AWB_HEADER* header = &afs2->header;

	fu_read_data(awb, (uint8_t*)&header->magic[0], 4, NULL);
	header->version = fu_read_u8(awb, NULL);
	header->offset_size = fu_read_u8(awb, NULL);
	header->id_align = fu_read_u16(awb, NULL, FU_LITTLE_ENDIAN);
	header->file_count = fu_read_u32(awb, NULL, FU_LITTLE_ENDIAN);
	header->alignment = fu_read_u16(awb, NULL, FU_LITTLE_ENDIAN);
	header->subkey = fu_read_u16(awb, NULL, FU_LITTLE_ENDIAN);
}

void awb_read_entries(FU_FILE* awb, AWB_FILE* afs2)
{
	AWB_HEADER* h = &afs2->header;

	/*	
		Plus one because last offset is the file size.
		It's easier to calculate file sizes for individual files
	*/
	afs2->entries = awb_alloc_entries(h->file_count+1);
	
	/* file IDs */
	for(uint32_t i = 0; i != h->file_count; ++i)
	{
		if(h->id_align == 2) afs2->entries[i].id = fu_read_u16(awb, NULL, FU_LITTLE_ENDIAN);
		if(h->id_align == 4) afs2->entries[i].id = fu_read_u32(awb, NULL, FU_LITTLE_ENDIAN);
	}
	
	/* file offsets */
	for(uint32_t i = 0; i != h->file_count+1; ++i)
	{
		if(h->offset_size == 2) afs2->entries[i].offset = fu_read_u16(awb, NULL, FU_LITTLE_ENDIAN);
		if(h->offset_size == 4) afs2->entries[i].offset = fu_read_u32(awb, NULL, FU_LITTLE_ENDIAN);
	}

	/* Check if data exists */
	awb_check_for_data(awb, afs2);

	if(afs2->no_data == 0)
	{
		/* fix file offsets */ 
		for(uint32_t i = 0; i != h->file_count; ++i)
		{
			if(afs2->entries[i].offset % h->alignment)
			{
				afs2->entries[i].offset += bound_calc_leftover(h->alignment, afs2->entries[i].offset);
			}
		}
	
		/* file sizes */
		for(uint32_t i = 0; i != h->file_count; ++i)
		{
			afs2->entries[i].size = afs2->entries[i+1].offset - afs2->entries[i].offset;
		}
		
		/* file data */
		for(uint32_t i = 0; i != h->file_count; ++i)
		{
			afs2->entries[i].data = (uint8_t*)calloc(1, afs2->entries[i].size);
			fu_seek(awb, afs2->entries[i].offset, FU_SEEK_SET);
			fu_read_data(awb, afs2->entries[i].data, afs2->entries[i].size, NULL);
		}
	}
}

FU_FILE* awb_write_file(AWB_FILE* afs2)
{
	FU_FILE* awb = fu_alloc_file();
	fu_create_mem_file(awb);

	awb_write_header(awb, afs2);
	awb_write_entries(awb, afs2);

	return awb;
}

void awb_write_header(FU_FILE* awb, AWB_FILE* afs2)
{
	AWB_HEADER* header = &afs2->header;

	fu_write_data(awb, (uint8_t*)header->magic, 4);
	fu_write_u8(awb, header->version);
	fu_write_u8(awb, header->offset_size);
	fu_write_u16(awb, header->id_align, FU_LITTLE_ENDIAN);
	fu_write_u32(awb, header->file_count, FU_LITTLE_ENDIAN);
	fu_write_u16(awb, header->alignment, FU_LITTLE_ENDIAN);
	fu_write_u16(awb, header->subkey, FU_LITTLE_ENDIAN);
}

void awb_write_entries(FU_FILE* awb, AWB_FILE* afs2)
{
	AWB_HEADER* header = &afs2->header;

	for(uint32_t i = 0; i != header->file_count; ++i)
	{
		if(header->id_align == 2) fu_write_u16(awb, afs2->entries[i].id, FU_LITTLE_ENDIAN);
		if(header->id_align == 4) fu_write_u32(awb, afs2->entries[i].id, FU_LITTLE_ENDIAN);
	}

	for(uint32_t i = 0; i != header->file_count; ++i)
	{
		if(header->offset_size == 2) fu_write_u16(awb, afs2->entries[i].offset, FU_LITTLE_ENDIAN);
		if(header->offset_size == 4) fu_write_u32(awb, afs2->entries[i].offset, FU_LITTLE_ENDIAN);
	}

	if(afs2->no_data == 0)
	{
		/* End of the file */
		const uint32_t end_off = afs2->entries[header->file_count-1].offset + afs2->entries[header->file_count-1].size;
		fu_write_u32(awb, end_off, FU_LITTLE_ENDIAN);

		/* Don't do data stuff when no data is present */
		if(afs2->no_data == 1) return;

		/* Reserve entire file size */
		fu_change_buf_size(awb, end_off);

		/* Write the file data */
		for(uint32_t i = 0; i != header->file_count; ++i)
		{
			fu_seek(awb, afs2->entries[i].offset, FU_SEEK_SET);
			fu_write_data(awb, afs2->entries[i].data, afs2->entries[i].size);
		}
	}
	else /* No data */
	{
		/* End of the file */
		const uint32_t end_off = afs2->entries[header->file_count].offset;

		if(header->offset_size == 2) fu_write_u16(awb, end_off, FU_LITTLE_ENDIAN);
		if(header->offset_size == 4) fu_write_u32(awb, end_off, FU_LITTLE_ENDIAN);
	}
}

void awb_default_header(AWB_HEADER* header, const uint32_t file_count)
{
	/* Default header values */
	memcpy(&header->magic[0], AWB_MAGIC, 4);
	header->version = 2;
	header->offset_size = 4;
	header->id_align = 2;
	header->file_count = file_count;
	header->alignment = 32;
	header->subkey = 0;
}

AWB_FILE* awb_alloc_file()
{
	return (AWB_FILE*)calloc(1, sizeof(AWB_FILE));
}

AWB_ENTRY* awb_alloc_entries(const uint32_t entry_count)
{
	return (AWB_ENTRY*)calloc(entry_count, sizeof(AWB_ENTRY));
}

uint8_t awb_check_for_data(FU_FILE* awb, AWB_FILE* afs2)
{
	AWB_HEADER* h = &afs2->header;

	const uint32_t afs2_size = afs2->entries[h->file_count].offset;	   
	
	/* Only the header */
	if(afs2_size > awb->size)
	{
		printf("No data\n");
		afs2->no_data = 1;
	}
	else
	{
		printf("There's data\n");
		afs2->no_data = 0;
	}

	return afs2->no_data;
}

void awb_print(AWB_FILE* afs2)
{
	printf("version: %u\n", afs2->header.version);
	printf("offset_size: %u\n", afs2->header.offset_size);
	printf("id_align: %u\n", afs2->header.id_align);
	printf("file_count: %u\n", afs2->header.file_count);
	printf("alignment: %u\n", afs2->header.alignment);
	printf("subkey: %u\n", afs2->header.subkey);
	
	if(afs2->entries)
	{
		for(uint32_t i = 0; i != afs2->header.file_count; ++i)
		{
			printf("\tID: %u\n", afs2->entries[i].id);
			printf("\tOffset: 0x%x\n", afs2->entries[i].offset);
			printf("\tData size: %x\n\n", afs2->entries[i].size);
		}
	}
}

void awb_free(AWB_FILE* afs2)
{
	for(uint32_t i = 0; i != afs2->header.file_count; ++i)
	{
		if(afs2->no_data == 0) free(afs2->entries[i].data);
		memset(&afs2->entries[i], 0, sizeof(AWB_ENTRY));
	}
		
	free(afs2->entries);
	
	afs2->entries = NULL;
	memset(&afs2->header, 0, sizeof(AWB_HEADER));
}