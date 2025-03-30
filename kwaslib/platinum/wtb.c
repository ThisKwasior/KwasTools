#include "wtb.h"

#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/io/dir_list.h>
#include <kwaslib/core/math/boundary.h>

WTB_FILE* wtb_alloc_empty_wtb()
{
    WTB_FILE* wtb = (WTB_FILE*)calloc(1, sizeof(WTB_FILE));
    wtb->entries = cvec_create(sizeof(WTB_ENTRY));
    wtb->platform = WTB_PLATFORM_LE;
    return wtb;
}

WTB_FILE* wtb_free(WTB_FILE* wtb)
{
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
		if(entry->data) free(entry->data);
	}
	
	wtb->entries = cvec_destroy(wtb->entries);
	free(wtb);
    
    return NULL;
}

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
	if(valid == 0) 
    {
        free(wtb);
        return NULL;
    }
	
    wtb->entries = cvec_create(sizeof(WTB_ENTRY));
    cvec_resize(wtb->entries, wtb->header.tex_count);
    
	/* Load entries */
	wtb_populate_entries(fwta, wtb);
	wtb_load_texture_data(fwtp, wtb);
	
	return wtb;
}

WTB_FILE* wtb_parse_wtb(FU_FILE* fwtb)
{
	return wtb_parse_wta_wtp(fwtb, fwtb);
}

uint8_t wtb_read_header(FU_FILE* file, WTB_FILE* wtb)
{
	fu_read_data(file, (uint8_t*)&wtb->header.magic[0], 4, NULL);
	
	/* 0 means it matches */
	const uint8_t is_little = strncmp(WTB_MAGIC_LE, (const char*)&wtb->header.magic[0], 4);
	const uint8_t is_big = strncmp(WTB_MAGIC_BE, (const char*)&wtb->header.magic[0], 4);

	if(is_little && is_big == 0) wtb->platform = WTB_PLATFORM_BE;
	else if(is_little == 0 && is_big) wtb->platform = WTB_PLATFORM_LE;

	if(wtb->platform == 0)
	{
		printf("File is not a valid WTB file\n");
		return 0;
	}
	
	wtb->header.unknown04 = fu_read_u32(file, NULL, wtb->platform);
	wtb->header.tex_count = fu_read_u32(file, NULL, wtb->platform);
	wtb->header.tex_offset_array_offset = fu_read_u32(file, NULL, wtb->platform);
	wtb->header.tex_size_offset = fu_read_u32(file, NULL, wtb->platform);
	wtb->header.flag_array_offset = fu_read_u32(file, NULL, wtb->platform);
	wtb->header.tex_id_array_offset = fu_read_u32(file, NULL, wtb->platform);
	wtb->header.xpr_info_offset = fu_read_u32(file, NULL, wtb->platform);
	
	return 1;
}

void wtb_populate_entries(FU_FILE* file, WTB_FILE* wtb)
{
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        
		/* Offsets */
		fu_seek(file, wtb->header.tex_offset_array_offset+(i*4), FU_SEEK_SET);
		entry->offset = fu_read_u32(file, NULL, wtb->platform);
		
		/* Sizes */
		fu_seek(file, wtb->header.tex_size_offset+(i*4), FU_SEEK_SET);
		entry->size = fu_read_u32(file, NULL, wtb->platform);
		
		/* Flags */
		fu_seek(file, wtb->header.flag_array_offset+(i*4), FU_SEEK_SET);
        const uint8_t b0 = fu_read_u8(file, NULL);
        entry->flags.b12 = fu_read_u16(file, NULL, wtb->platform);
        const uint8_t b3 = fu_read_u8(file, NULL);
        memcpy(&entry->flags.b0, &b0, 1);
        memcpy(&entry->flags.b3, &b3, 1);
        
		/* Ids */
		fu_seek(file, wtb->header.tex_id_array_offset+(i*4), FU_SEEK_SET);
		entry->id = fu_read_u32(file, NULL, wtb->platform);
		
		/* XBOX 360 info */
		if(wtb->platform == WTB_PLATFORM_BE && wtb->header.xpr_info_offset)
		{
			D3DBaseTexture* xb = &entry->x360;
			uint32_t* xb_dword = (uint32_t*)xb;
			fu_seek(file, wtb->header.xpr_info_offset+(i*sizeof(D3DBaseTexture)), FU_SEEK_SET);
			
			for(uint32_t i = 0; i != (sizeof(D3DBaseTexture)/sizeof(uint32_t)); ++i)
			{
				xb_dword[i] = fu_read_u32(file, NULL, FU_BIG_ENDIAN);
			}
		}
	}
}

void wtb_load_texture_data(FU_FILE* file, WTB_FILE* wtb)
{
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
		entry->data = (uint8_t*)calloc(1, entry->size);
        fu_seek(file, entry->offset, FU_SEEK_SET);
        fu_read_data(file, entry->data, entry->size, NULL);
	}
}

void wtb_update_on_export(WTB_FILE* wtb, const uint8_t external_data)
{
    WTB_HEADER* h = &wtb->header;
    
    /* Header */
    
    if(wtb->platform == WTB_PLATFORM_LE)
        memcpy(&h->magic[0], WTB_MAGIC_LE, 4);
    else if(wtb->platform == WTB_PLATFORM_BE)
        memcpy(&h->magic[0], WTB_MAGIC_BE, 4);
    
    h->unknown04 = 1;
    h->tex_count = cvec_size(wtb->entries);
    h->tex_offset_array_offset = 0x20;
    
    uint32_t section_size = h->tex_count * 4;
    section_size += bound_calc_leftover(WTB_SECTION_ALIGNMENT, section_size);

    h->tex_size_offset = h->tex_offset_array_offset + section_size;
    h->flag_array_offset = h->tex_size_offset + section_size;
    h->tex_id_array_offset = h->flag_array_offset + section_size;
  
    /* Entries */
    
    uint32_t data_offset = 0;
    
    if(external_data)
    {
         data_offset = h->tex_id_array_offset + section_size;
         data_offset += bound_calc_leftover(WTB_BLOCK_SIZE, data_offset);
    }
    
    for(uint32_t i = 0; i != h->tex_count; ++i)
    {
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        entry->offset = data_offset;
        
        data_offset += entry->size;
        data_offset += bound_calc_leftover(WTB_BLOCK_SIZE, data_offset);;
    }
    
    wtb_set_flags_from_dds(wtb);
}

void wtb_write_wta_fu(WTB_FILE* wtb, FU_FILE* fwta)
{
    /* Header and offsets to data */
    WTB_HEADER* h = &wtb->header;
    
    fu_write_data(fwta, &wtb->header.magic[0], 4);
    fu_write_u32(fwta, h->unknown04, wtb->platform);
    fu_write_u32(fwta, h->tex_count, wtb->platform);
    fu_write_u32(fwta, h->tex_offset_array_offset, wtb->platform);
    fu_write_u32(fwta, h->tex_size_offset, wtb->platform);
    fu_write_u32(fwta, h->flag_array_offset, wtb->platform);
    fu_write_u32(fwta, h->tex_id_array_offset, wtb->platform);
    fu_write_u32(fwta, h->xpr_info_offset, wtb->platform);
    
    /* Offsets */
    if(h->tex_offset_array_offset)
    {
        fu_change_buf_size(fwta, h->tex_offset_array_offset);
        fu_seek(fwta, h->tex_offset_array_offset, SEEK_SET);
        for(uint32_t i = 0; i != h->tex_count; ++i)
        {
            WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
            fu_write_u32(fwta, entry->offset, wtb->platform);
        }
    }
    
    if(h->tex_size_offset)
    {
        fu_change_buf_size(fwta, h->tex_size_offset);
        fu_seek(fwta, h->tex_size_offset, SEEK_SET);
        for(uint32_t i = 0; i != h->tex_count; ++i)
        {
            WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
            fu_write_u32(fwta, entry->size, wtb->platform);
        }
    }
    
    if(h->flag_array_offset)
    {
        fu_change_buf_size(fwta, h->flag_array_offset);
        fu_seek(fwta, h->flag_array_offset, SEEK_SET);
        for(uint32_t i = 0; i != h->tex_count; ++i)
        {
            WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
            const uint8_t flags_b0 = *(const uint8_t*)&entry->flags.b0;
            const uint8_t flags_b3 = *(const uint8_t*)&entry->flags.b3;

            fu_write_u8(fwta, flags_b0);
            fu_write_u8(fwta, 0);
            fu_write_u8(fwta, 0);
            fu_write_u8(fwta, flags_b3);
        }
    }
    
    if(h->tex_id_array_offset)
    {
        fu_change_buf_size(fwta, h->tex_id_array_offset);
        fu_seek(fwta, h->tex_id_array_offset, SEEK_SET);
        for(uint32_t i = 0; i != h->tex_count; ++i)
        {
            WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
            fu_write_u32(fwta, entry->id, wtb->platform);
        }
    }
    
    const uint32_t leftover = bound_calc_leftover(WTB_SECTION_ALIGNMENT, h->tex_count*4);
    fu_add_to_buf_size(fwta, leftover);
    
    fu_seek(fwta, 0, FU_SEEK_END);
}

void wtb_write_wtp_fu(WTB_FILE* wtb, FU_FILE* fwtp)
{
	for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
	{
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        fu_change_buf_size(fwtp, entry->offset);
        fu_seek(fwtp, 0, FU_SEEK_END);
		fu_write_data(fwtp, entry->data, entry->size);
		fu_add_to_buf_size(fwtp, bound_calc_leftover(WTB_BLOCK_SIZE, fu_tell(fwtp)));
	}
    
    fu_seek(fwtp, 0, FU_SEEK_END);
}

void wtb_save_wtb_to_fu_file(WTB_FILE* wtb, FU_FILE* fwtb)
{
	fu_create_mem_file(fwtb);
    
    wtb_update_on_export(wtb, 1);
	
    wtb_write_wta_fu(wtb, fwtb);
    wtb_write_wtp_fu(wtb, fwtb);
}

void wtb_save_wta_wtp_to_fu_files(WTB_FILE* wtb, FU_FILE* fwta, FU_FILE* fwtp)
{
	fu_create_mem_file(fwta);
	fu_create_mem_file(fwtp);
	
    wtb_update_on_export(wtb, 0);

    wtb_write_wta_fu(wtb, fwta);
    wtb_write_wtp_fu(wtb, fwtp);
}

void wtb_set_flags_from_dds(WTB_FILE* wtb)
{
    for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
    {
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        
        const uint8_t complex_byte = entry->data[0x6C];
        const uint8_t alpha_byte = entry->data[0x50];
        const uint8_t cubemap_byte = entry->data[0x71];
        
        entry->flags.b0.complex = ((complex_byte&0x8)>>3) ? 0 : 1;
        entry->flags.b0.always2_0 = 1;
        entry->flags.b3.alpha = ((alpha_byte&0x2)>>1);
        entry->flags.b3.always2_1 = 1;
        entry->flags.b3.cubemap = ((cubemap_byte&0x2)>>1);
    }
}