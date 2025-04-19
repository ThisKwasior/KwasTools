#include "dds.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kwaslib/core/io/type_readers.h>

DDS_FILE* dds_header_from_data(const char* data)
{
    DDS_FILE* dds = NULL;
    
    if(strncmp("DDS ", &data[0], 4) == 0)
    {
        dds = (DDS_FILE*)calloc(1, sizeof(DDS_FILE));
        dds_read_header(dds, data);
    }
    
    return dds;
}

void dds_read_header(DDS_FILE* dds, const char* data)
{
    uint32_t* dds_int = (uint32_t*)dds;
    
    for(uint32_t i = 0; i != 0x80; i+=4)
    {
        *dds_int = tr_read_u32le((uint8_t*)&data[i]);
        dds_int += 1;
    }
}

void dds_print_info(DDS_FILE* dds)
{
    printf("flags.caps:              %u\n", dds->header.flags.data.caps);
    printf("flags.height:            %u\n", dds->header.flags.data.height);
    printf("flags.width:             %u\n", dds->header.flags.data.width);
    printf("flags.pitch:             %u\n", dds->header.flags.data.pitch);
    printf("flags.pixelformat:       %u\n", dds->header.flags.data.pixelformat);
    printf("flags.mipmapcount:       %u\n", dds->header.flags.data.mipmapcount);
    printf("flags.linearsize         %u\n", dds->header.flags.data.linearsize);
    printf("flags.depth:             %u\n", dds->header.flags.data.depth);
    printf("Height:                  %u\n", dds->header.height);
    printf("Width:                   %u\n", dds->header.width);
    printf("pitch_or_linear_size:    %u\n", dds->header.pitch_or_linear_size);
    printf("Depth:                   %u\n", dds->header.depth);
    printf("Mipmap count:            %u\n", dds->header.mipmap_count);
    
    const DDS_PIXELFORMAT_FLAGS* plf = &dds->header.pf.flags;
    printf("pf.flags.alphapixels:    %u\n", plf->data.alphapixels);
    printf("pf.flags.alpha:          %u\n", plf->data.alpha);
    printf("pf.flags.fourcc:         %u\n", plf->data.fourcc);
    printf("pf.flags.rgb:            %u\n", plf->data.rgb);
    printf("pf.flags.yuv:            %u\n", plf->data.yuv);
    printf("pf.flags.luminance:      %u\n", plf->data.luminance);
    printf("pf.fourcc:               %*s\n", 4, dds->header.pf.fourcc);
    printf("pf.rgb_bit_count:        %u\n", dds->header.pf.rgb_bit_count);
    printf("pf.r_bitmask:            %08x\n", dds->header.pf.r_bitmask);
    printf("pf.r_bitmask:            %08x\n", dds->header.pf.g_bitmask);
    printf("pf.r_bitmask:            %08x\n", dds->header.pf.b_bitmask);
    printf("pf.r_bitmask:            %08x\n", dds->header.pf.a_bitmask);
    printf("caps.complex:            %u\n", dds->header.caps.data.complex);
    printf("caps.texture:            %u\n", dds->header.caps.data.texture);
    printf("caps.mipmap:             %u\n", dds->header.caps.data.mipmap);
    printf("caps2.cubemap:           %u\n", dds->header.caps2.data.cubemap);
    printf("cubemap surfaces:        {%u,%u,%u,%u,%u,%u}\n",
                                     dds->header.caps2.data.cubemap_positivex,
                                     dds->header.caps2.data.cubemap_negativex,
                                     dds->header.caps2.data.cubemap_positivey,
                                     dds->header.caps2.data.cubemap_negativey,
                                     dds->header.caps2.data.cubemap_positivez,
                                     dds->header.caps2.data.cubemap_negativez);
}