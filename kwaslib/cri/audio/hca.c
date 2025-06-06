#include "hca.h"

#include <kwaslib/core/io/type_readers.h>

/* 
    Reads the section name without encryption
*/
#define HCA_READ_SECTION_NAME(data, out)    \
        (out)[0] = (data)[0]&0x7f;          \
        (out)[1] = (data)[1]&0x7f;          \
        (out)[2] = (data)[2]&0x7f;          \
        (out)[3] = (data)[3]&0x7f;

HCA_HEADER hca_read_header_from_data(const uint8_t* data, const uint32_t size)
{
    HCA_HEADER h = {0};
    HCA_READ_SECTION_NAME(data, &h.magic[0])
    
    if(su_cmp_char(HCA_MAGIC, 4, (const char*)h.magic, 4) != SU_STRINGS_MATCH)
    {
        /* Not a HCA */
        return h;
    }
    
    h.version_major = data[4];
    h.version_minor = data[5];
    h.data_offset = tr_read_u16be(&data[6]);
    
    /* Reading sections until it's 0x00000000 or position reached data_offset-2*/
    uint32_t pos = 8;
    uint8_t name[4] = {0};
    
    while(1)
    {
        /* Minus 2 because last two bytes are CRC-16/UMTS checksum */
        if(pos >= (h.data_offset-2)) break;

        /* Reading the section name without encryption */
        HCA_READ_SECTION_NAME(&data[pos], &name[0]);
        pos += 4;
        
        /* Humongous if statement */
        if(su_cmp_char("\0\0\0\0", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            break;
        }
        else if(su_cmp_char("fmt\0", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.fmt = 1;
            uint8_t sample_rate_buf[4] = {0};
            
            h.fmt.channel_count = data[pos];
            tr_read_array(&data[pos+1], 3, &sample_rate_buf[1]);
            h.fmt.sample_rate = tr_read_u32be(sample_rate_buf);
            h.fmt.block_count = tr_read_u32be(&data[pos+4]);
            h.fmt.inserted_samples = tr_read_u16be(&data[pos+8]);
            h.fmt.appended_samples = tr_read_u16be(&data[pos+10]);
            pos += 12;
        }
        else if(su_cmp_char("comp", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.comp = 1;
            h.comp.block_size = tr_read_u16be(&data[pos]);
            h.comp.min_res = data[pos+2];
            h.comp.max_res = data[pos+3];
            h.comp.track_count = data[pos+4];
            h.comp.channel_config = data[pos+5];
            h.comp.total_band_count = data[pos+6];
            h.comp.base_band_count = data[pos+7];
            h.comp.stereo_band_count = data[pos+8];
            h.comp.bands_per_hfr_group = data[pos+9];
            tr_read_array(&data[pos+10], 2, &h.comp.reserved[0]);
            pos += 12;
        }
        else if(su_cmp_char("dec\0", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.dec = 1;
            h.dec.block_size = tr_read_u16be(&data[pos]);
            h.dec.min_res = data[pos+2];
            h.dec.max_res = data[pos+3];
            h.dec.total_band_count = data[pos+4];
            h.dec.base_band_count = data[pos+5];
            h.dec.track_count = (data[pos+6]>>4)&0x0F;
            h.dec.channel_config = (data[pos+6])&0x0F;
            h.dec.stereo_type = data[pos+7];
            pos += 8;
        }
        else if(su_cmp_char("vbr\0", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.vbr = 1;
            h.vbr.max_frame_size = tr_read_u16be(&data[pos]);
            h.vbr.noise_level = tr_read_u16be(&data[pos+2]);
            pos += 4;
        }
        else if(su_cmp_char("ath\0", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.ath = 1;
            h.ath.ath_table_type = tr_read_u16be(&data[pos]);
            pos += 2;
        }
        else if(su_cmp_char("loop", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.loop = 1;
            h.loop.start = tr_read_u32be(&data[pos]);
            h.loop.end = tr_read_u32be(&data[pos+4]);
            h.loop.pre_loop_samples = tr_read_u16be(&data[pos+8]);
            h.loop.post_loop_samples = tr_read_u16be(&data[pos+10]);
            pos += 12;
        }
        else if(su_cmp_char("ciph", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.ciph = 1;
            h.ciph.type = tr_read_u16be(&data[pos]);
            pos += 2;
        }
        else if(su_cmp_char("rva\0", 4, (const char*)name, 4) == SU_STRINGS_MATCH)
        {
            h.sections.rva = 1;
            h.rva.volume = tr_read_f32be(&data[pos]);
            pos += 4;
        }
    }

    return h;
}

const uint32_t hca_get_file_size(const HCA_HEADER hcah)
{
    uint32_t size = 0;
    uint32_t block_size = 0;
    uint32_t block_count = 0;
    
    if(hcah.data_offset) size = hcah.data_offset;
    if(hcah.sections.fmt) block_count = hcah.fmt.block_count;
    
    if(hcah.sections.comp) block_size = hcah.comp.block_size;
    else if(hcah.sections.dec) block_size = hcah.dec.block_size;

    size += block_size*block_count;
    
    return size;
}