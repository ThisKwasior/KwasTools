#include "adx.h"

#include <stdlib.h>

#include <kwaslib/core/io/type_readers.h>

ADX_FILE* adx_alloc()
{
    ADX_FILE* adx = (ADX_FILE*)calloc(1, sizeof(ADX_FILE)); 
    adx->header.loop = cvec_create(sizeof(ADX_LOOP));
    adx->header.hist = cvec_create(sizeof(ADX_HISTORY));
    adx->frames = cvec_create(sizeof(ADX_FRAME));
    return adx;
}

ADX_FILE* adx_free(ADX_FILE* adx)
{
    adx->header.loop = cvec_destroy(adx->header.loop);
    adx->header.hist = cvec_destroy(adx->header.hist);
    
    for(uint32_t i = 0; i != cvec_size(adx->frames); ++i)
    {
        ADX_FRAME* frame = (ADX_FRAME*)cvec_at(adx->frames, i);
        frame->samples = cvec_destroy(frame->samples);
    }
    
    adx->frames = cvec_destroy(adx->frames);
    free(adx);
    return NULL;
}

ADX_FILE* adx_load_from_data(const uint8_t* data, const uint32_t size)
{
    if(size < 48)
    {
        return NULL;
    }
    
    /* Check if it's a valid ADX before doing ANYTHING (learned hard way) */
    if(su_cmp_char(ADX_MAGIC, 2, (const char*)&data[0], 2) != SU_STRINGS_MATCH)
    {
        return NULL;
    }
    
    /*
        Read the header size and check
        for cri copyright before allocating anything
    */
    const uint16_t header_size = tr_read_u16be(&data[2]);
    char cric[6] = {0};
    tr_read_array(&data[header_size-2], 6, (uint8_t*)&cric[0]);
    
    if(su_cmp_char(ADX_CRI_COPYRIGHT_STR, 6, cric, 6) != SU_STRINGS_MATCH)
    {
        return NULL;
    }
    
    /* Allocate the structure */
    ADX_FILE* adx = adx_alloc();
    ADX_HEADER* h = &adx->header;
    ADX_FOOTER* f = &adx->footer;
    uint32_t pos = 0;
    
    /* Reading the header */
    tr_read_array(&data[pos], 2, &h->magic[0]);
    h->header_size = header_size;
    h->encoding_type = data[pos+4];
    h->frame_size = data[pos+5];
    h->bit_depth = data[pos+6];
    h->channel_count = data[pos+7];
    h->sample_rate = tr_read_u32be(&data[pos+8]);
    h->sample_count = tr_read_u32be(&data[pos+12]);
    h->highpass_freq = tr_read_u16be(&data[pos+16]);
    h->version = data[pos+18];
    h->revision = data[pos+19];
    pos = 20;
    
    /* History */
    if(h->version == 4)
    {
        /*h->pad = tr_read_u32be(&data[pos]);*/
        pos += 4;
        uint32_t hist_count = h->channel_count;
        if(h->channel_count < 2) hist_count = 2;
        cvec_resize(h->hist, hist_count);
        
        for(uint32_t i = 0; i != hist_count; ++i)
        {
            ADX_HISTORY* hist = (ADX_HISTORY*)cvec_at(h->hist, i);
            hist->hist1 = tr_read_u16be(&data[pos]);
            hist->hist2 = tr_read_u16be(&data[pos+2]);
            pos += 4;
        }
    }

    /*
        Loop data
        Disabled for now. Sometimes (c)CRI is in place of h->loop_count
        TODO: Fix ADX loop
    */
    /*
    h->alignment_samples = tr_read_u16be(&data[pos]);
    h->loop_count = tr_read_u16be(&data[pos+2]);
    cvec_resize(h->loop, h->loop_count);
    pos += 4;
    
    for(uint32_t i = 0; i != h->loop_count; ++i)
    {
        ADX_LOOP* loop = (ADX_LOOP*)cvec_at(h->loop, i);
        loop->loop_num = tr_read_u16be(&data[pos]);
        loop->loop_type = tr_read_u16be(&data[pos+2]);
        loop->loop_start_sample = tr_read_u32be(&data[pos+4]);
        loop->loop_start_byte = tr_read_u32be(&data[pos+8]);
        loop->loop_end_sample = tr_read_u32be(&data[pos+12]);
        loop->loop_end_byte = tr_read_u32be(&data[pos+16]);
        pos += 20;
    }
    */
    
    /* Reading audio frames */
    pos = 4 + h->header_size;
    
    const uint8_t sample_bytes_frame = (h->frame_size-2);
    const uint16_t samples_frame = sample_bytes_frame*2;
    uint32_t frame_count = h->sample_count/samples_frame;
    if(h->sample_count%samples_frame)
        frame_count += 1;
    
    frame_count *= h->channel_count;
    cvec_resize(adx->frames, frame_count);
    
    for(uint32_t i = 0; i != frame_count; ++i)
    {
        ADX_FRAME* frame = (ADX_FRAME*)cvec_at(adx->frames, i);
        frame->samples = cvec_create(sample_bytes_frame);
        cvec_resize(frame->samples, 1);
        uint8_t* samples = (uint8_t*)cvec_at(frame->samples, 0);
        uint16_t* frame_u16 = (uint16_t*)frame;
        
        *frame_u16 = tr_read_u16be(&data[pos]);
        tr_read_array(&data[pos+2], sample_bytes_frame, samples);
        pos += h->frame_size;
    }
    
    /* At last, the footer */
    tr_read_array(&data[pos], 2, &f->magic[0]);
    f->pad_len = tr_read_u16be(&data[pos+2]);
    
    return adx;
}

const uint32_t adx_get_file_size(ADX_FILE* adx)
{
    return 4 + adx->header.header_size
           + cvec_size(adx->frames)*adx->header.frame_size
           + 4 + adx->footer.pad_len;
}