#include "wtb.h"

#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>
#include <kwaslib/core/math/boundary.h>
#include <kwaslib/core/data/image/dds.h>

WTB_FILE* wtb_parse_wta_wtp(FU_FILE* fwta, FU_FILE* fwtp)
{
    WTB_FILE* wtb = wtb_alloc_wtb();
    
    if(wtb == NULL)
    {
        printf("Could not allocate WTB structure\n");
        return NULL;
    }
    
    wtb_read_header(fwta, wtb);
    
    if(wtb->platform == WTB_PLATFORM_INVALID)
    {
        printf("File is not a valid WTB.\n");
        wtb = wtb_free(wtb);
        return NULL;
    }
    
    wtb_read_image_data(fwtp, wtb);
    
    return wtb;
}

WTB_FILE* wtb_parse_wtb(FU_FILE* fwtb)
{
    return wtb_parse_wta_wtp(fwtb, fwtb);
}

FU_FILE* wtb_header_to_fu_file(WTB_FILE* wtb)
{
    FU_FILE* fwta = fu_alloc_file();
    fu_create_mem_file(fwta);
    
    if(wtb->platform == WTB_PLATFORM_LE)
    {
        fu_write_data(fwta, (const uint8_t*)WTB_MAGIC_LE, 4);
    }
    if(wtb->platform == WTB_PLATFORM_BE)
    {
        fu_write_data(fwta, (const uint8_t*)WTB_MAGIC_BE, 4);
    }
    
    fu_write_u32(fwta, wtb->header.version, wtb->platform);
    fu_write_u32(fwta, wtb->header.tex_count, wtb->platform);
    fu_write_u32(fwta, wtb->header.positions_offset, wtb->platform);
    fu_write_u32(fwta, wtb->header.sizes_offset, wtb->platform);
    fu_write_u32(fwta, wtb->header.flags_offset, wtb->platform);
    fu_write_u32(fwta, wtb->header.ids_offset, wtb->platform);
    fu_write_u32(fwta, wtb->header.xpr_info_offset, wtb->platform);
    
    fu_check_buf_rem(fwta, wtb->header.positions_offset);
    fu_check_buf_rem(fwta, wtb->header.sizes_offset);
    fu_check_buf_rem(fwta, wtb->header.flags_offset);
    fu_check_buf_rem(fwta, wtb->header.ids_offset);
    
    for(uint32_t i = 0; i != cvec_size(wtb->entries); ++i)
    {
        WTB_ENTRY* entry = wtb_get_entry_by_id(wtb->entries, i);
        
        if(wtb->header.positions_offset)
        {
            const uint64_t cur_pos = wtb->header.positions_offset + i*4;
            fu_seek(fwta, cur_pos, FU_SEEK_SET);
            fu_write_u32(fwta, entry->position, wtb->platform);
        }

        if(wtb->header.sizes_offset)
        {
            const uint64_t cur_pos = wtb->header.sizes_offset + i*4;
            fu_seek(fwta, cur_pos, FU_SEEK_SET);
            fu_write_u32(fwta, entry->size, wtb->platform);
        }

        if(wtb->header.flags_offset)
        {
            const uint64_t cur_pos = wtb->header.flags_offset + i*4;
            fu_seek(fwta, cur_pos, FU_SEEK_SET);
            fu_write_u32(fwta, entry->flags.buf, wtb->platform);
        }
        
        if(wtb->header.ids_offset)
        {
            const uint64_t cur_pos = wtb->header.ids_offset + i*4;
            fu_seek(fwta, cur_pos, FU_SEEK_SET);
            fu_write_u32(fwta, entry->id, wtb->platform);
        }
    }
    
    fu_seek(fwta, 0, FU_SEEK_END);
    const uint64_t bound = bound_calc_leftover(WTB_SECTION_ALIGNMENT, fu_tell(fwta));
    fu_add_to_buf_size(fwta, bound);
    
    return fwta;
}

FU_FILE* wtb_data_to_fu_file(WTB_FILE* wtb)
{
    FU_FILE* fwtp = fu_alloc_file();
    fu_create_mem_file(fwtp);
    
    WTB_ENTRY* last_entry = cvec_back(wtb->entries);
    fu_check_buf_rem(fwtp, last_entry->position);
    
    for(uint32_t i = 0; i != cvec_size(wtb->entries); ++i)
    {
        WTB_ENTRY* entry = wtb_get_entry_by_id(wtb->entries, i);
        fu_seek(fwtp, entry->position, FU_SEEK_SET);
        fu_write_data(fwtp, entry->data, entry->size);
    }
    
    fu_seek(fwtp, 0, FU_SEEK_END);
    const uint64_t bound = bound_calc_leftover(WTB_BLOCK_SIZE, fu_tell(fwtp));
    fu_add_to_buf_size(fwtp, bound);
    
    return fwtp;
}

void wtb_update(WTB_FILE* wtb, const uint8_t wtp)
{
    if(wtb->platform == WTB_PLATFORM_LE)
    {
        memcpy(&wtb->header.magic[0], WTB_MAGIC_LE, 4);
    }
    else if(wtb->platform == WTB_PLATFORM_BE)
    {
        memcpy(&wtb->header.magic[0], WTB_MAGIC_BE, 4);
    }
    else /* What are you doing */
    {
        printf("How is the platform %u???\n", wtb->platform);
        return;
    }
    
    wtb->header.version = 1;
    wtb->header.tex_count = cvec_size(wtb->entries);
    
    uint32_t cur_pos = 0x20;
    wtb->header.positions_offset = cur_pos;
    
    cur_pos += wtb->header.tex_count*4;
    cur_pos += bound_calc_leftover(WTB_SECTION_ALIGNMENT, cur_pos);
    wtb->header.sizes_offset = cur_pos;
    
    cur_pos += wtb->header.tex_count*4;
    cur_pos += bound_calc_leftover(WTB_SECTION_ALIGNMENT, cur_pos);
    wtb->header.flags_offset = cur_pos;
    
    cur_pos += wtb->header.tex_count*4;
    cur_pos += bound_calc_leftover(WTB_SECTION_ALIGNMENT, cur_pos);
    wtb->header.ids_offset = cur_pos;
    
    if(wtb->platform == WTB_PLATFORM_BE)
    {
        cur_pos += wtb->header.tex_count*4;
        cur_pos += bound_calc_leftover(WTB_SECTION_ALIGNMENT, cur_pos);
        wtb->header.xpr_info_offset = cur_pos;
        cur_pos += wtb->header.tex_count*sizeof(D3DBaseTexture);
    }

    if(wtp == 1)
    {
        cur_pos = 0;
    }
    else
    {
        cur_pos += bound_calc_leftover(WTB_BLOCK_SIZE, cur_pos); 
    }
    
    for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
    {
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        entry->position = cur_pos;
        cur_pos += entry->size;
        cur_pos += bound_calc_leftover(WTB_BLOCK_SIZE, cur_pos); 
    }
}

WTB_FILE* wtb_alloc_wtb()
{
    WTB_FILE* wtb = (WTB_FILE*)calloc(1, sizeof(WTB_FILE));
    wtb->entries = cvec_create(sizeof(WTB_ENTRY));
    wtb->platform = WTB_PLATFORM_LE;
    return wtb;
}

WTB_FILE* wtb_free(WTB_FILE* wtb)
{
    for(uint32_t i = 0; i != cvec_size(wtb->entries); ++i)
    {
        WTB_ENTRY* entry = (WTB_ENTRY*)cvec_at(wtb->entries, i);
        if(entry->data)
        {
            free(entry->data);
        }
    }
    
    wtb->entries = cvec_destroy(wtb->entries);
    free(wtb);
    
    return NULL;
}

void wtb_read_header(FU_FILE* fwtb, WTB_FILE* wtb)
{
    fu_read_data(fwtb, &wtb->header.magic[0], 4, NULL);
    
    if(strncmp((const char*)&wtb->header.magic[0], WTB_MAGIC_LE, 4) == 0)
    {
        wtb->platform = WTB_PLATFORM_LE;
    }
    else if(strncmp((const char*)&wtb->header.magic[0], WTB_MAGIC_BE, 4) == 0)
    {
        wtb->platform = WTB_PLATFORM_BE;
    }
    else /* Invalid file */
    {
        wtb->platform = WTB_PLATFORM_INVALID;
        return;
    }
  
    /* Header */
    wtb->header.version = fu_read_u32(fwtb, NULL, wtb->platform);
    wtb->header.tex_count = fu_read_u32(fwtb, NULL, wtb->platform);
    wtb->header.positions_offset = fu_read_u32(fwtb, NULL, wtb->platform);
    wtb->header.sizes_offset = fu_read_u32(fwtb, NULL, wtb->platform);
    wtb->header.flags_offset = fu_read_u32(fwtb, NULL, wtb->platform);
    wtb->header.ids_offset = fu_read_u32(fwtb, NULL, wtb->platform);
    wtb->header.xpr_info_offset = fu_read_u32(fwtb, NULL, wtb->platform);
    
    cvec_resize(wtb->entries, wtb->header.tex_count);
    
    for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
    {
        WTB_ENTRY* entry = wtb_get_entry_by_id(wtb->entries, i);
        
        if(wtb->header.positions_offset)
        {
            const uint64_t cur_pos = wtb->header.positions_offset + i*4;
            fu_seek(fwtb, cur_pos, FU_SEEK_SET);
            entry->position = fu_read_u32(fwtb, NULL, wtb->platform);
        }
      
        if(wtb->header.sizes_offset)
        {
            const uint64_t cur_pos = wtb->header.sizes_offset + i*4;
            fu_seek(fwtb, cur_pos, FU_SEEK_SET);
            entry->size = fu_read_u32(fwtb, NULL, wtb->platform);
        }
        
        if(wtb->header.flags_offset)
        {
            const uint64_t cur_pos = wtb->header.flags_offset + i*4;
            fu_seek(fwtb, cur_pos, FU_SEEK_SET);
            entry->flags.buf = fu_read_u32(fwtb, NULL, wtb->platform);
        }
        
        if(wtb->header.ids_offset)
        {
            const uint64_t cur_pos = wtb->header.ids_offset + i*4;
            fu_seek(fwtb, cur_pos, FU_SEEK_SET);
            entry->id = fu_read_u32(fwtb, NULL, wtb->platform);
        }
        
        if(wtb->header.xpr_info_offset && (wtb->platform == WTB_PLATFORM_BE))
        {
            const uint64_t cur_pos = wtb->header.xpr_info_offset + (i*sizeof(D3DBaseTexture));
            fu_seek(fwtb, cur_pos, FU_SEEK_SET);
            
            D3DBaseTexture* xb = &entry->x360;
            uint32_t* xb_dword = (uint32_t*)xb;
            
            for(uint32_t i = 0; i != (sizeof(D3DBaseTexture)/sizeof(uint32_t)); ++i)
            {
                xb_dword[i] = fu_read_u32(fwtb, NULL, FU_BIG_ENDIAN);
            }
        }
    }
}

void wtb_read_image_data(FU_FILE* fwtb, WTB_FILE* wtb)
{
    for(uint32_t i = 0; i != cvec_size(wtb->entries); ++i)
    {
        WTB_ENTRY* entry = wtb_get_entry_by_id(wtb->entries, i);
        
        if(entry)
        {
            fu_seek(fwtb, entry->position, SEEK_SET);
            entry->data = (uint8_t*)calloc(1, entry->size);
            fu_read_data(fwtb, entry->data, entry->size, NULL);
        }
    }
}

void wtb_append_entry(CVEC entries,
                      const uint32_t size,
                      const uint32_t id,
                      const uint8_t* data)
{
    WTB_ENTRY entry = {0};
    entry.size = size;
    entry.id = id;
    entry.data = (uint8_t*)calloc(1, size);
    memcpy(entry.data, data, size);
    cvec_push_back(entries, &entry);
}

WTB_ENTRY* wtb_get_entry_by_id(CVEC entries, const uint64_t id)
{
    return (WTB_ENTRY*)cvec_at(entries, id);
}

void wtb_set_entry_flags(WTB_ENTRY* entry, const uint8_t atlas)
{
    /* Checks for non-DDS */
    if((strncmp((const char*)&entry->data[0], "II*\0", 4) == 0)      /* TIFF */
        || (strncmp((const char*)&entry->data[0], "MM\0*", 4) == 0)  /* Also TIFF */
        || (strncmp((const char*)&entry->data[0], "‰PNG", 4) == 0)   /* PNG */
        || (strncmp((const char*)&entry->data[6], "JFIF", 4) == 0))  /* JPEG */
    {
        entry->flags.data.noncomplex = 1;
        entry->flags.data.always_set = 1;
        entry->flags.data.atlas = atlas;
        return;
    }
    
    DDS_FILE dds = {0};
    dds_read_header(&dds, (const char*)&entry->data[0]);
    
    entry->flags.data.noncomplex = dds.header.caps.data.complex ? 0 : 1;
    entry->flags.data.always_set = 1;
    entry->flags.data.alphaonly = dds.header.pf.flags.data.alpha;
    entry->flags.data.atlas = atlas;
    entry->flags.data.cubemap = dds.header.caps2.data.cubemap;
    
    /*
        Checking for DXT1A.
    */
    if(strncmp((const char*)&dds.header.pf.fourcc[0], "DXT1", 4) == 0)
    {
        for(uint32_t i = 0x80; i < entry->size; i+=8)
        {
            const uint16_t c1 = tr_read_u16le((uint8_t*)&entry->data[i]);
            const uint16_t c2 = tr_read_u16le((uint8_t*)&entry->data[i+2]);
            const uint32_t px = tr_read_u32le((uint8_t*)&entry->data[i+4]);
            
            /*
                color1 <= color2
                It's how we know it's a DXT1A texture
            */
            if(c1 <= c2)
            {
                /*
                    Here we check if the colors in the
                    4x4 block are even using the transparency,
                    which is the value 3
                */
                for(uint8_t j = 0; j != 32; j+=2)
                {
                    const uint8_t code = (px>>j)&0x3;
                    
                    /* Pixel is black (transparent) */
                    if(code == 3)
                    {
                        entry->flags.data.dxt1a = 1;
                        return;
                    }
                }
            }
        }
    }
}
