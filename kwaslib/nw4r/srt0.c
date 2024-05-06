#include "srt0.h"

#include <stdlib.h>
#include <string.h>

static const char* SRT0_MATRIX_MODES_STR[] = 
{
	"MAYA", "XSI", "3DSMAX"
};

static const char* SRT0_FRAME_DATA_NAME_STR[] = 
{
	"Scale X", "Scale Y", "Rotation", "Trans X", "Trans Y"
};

SRT0_FILE* srt0_read_file(FU_FILE* srt0)
{
	SRT0_FILE* srt0f = srt0_alloc_file();
	
	if(srt0f == NULL) return NULL;
	
	srt0_read_sub_header(srt0, &srt0f->sub_header);
	
	if(strncmp(&srt0f->sub_header.magic[0], SRT0_MAGIC, 4) != 0)
	{
		free(srt0f);
		return NULL;
	}
	
	srt0_read_header(srt0, &srt0f->header);
	srt0_read_groups(srt0, srt0f);
	
	return srt0f;
}

void srt0_read_sub_header(FU_FILE* srt0, BRRES_SUB_HEADER* sub_header)
{
	/* Common in all sub headers */
	fu_read_data(srt0, (uint8_t*)&sub_header->magic[0], 4, NULL);
	sub_header->length = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	sub_header->version = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	sub_header->outer_brres_offset = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	
	/* Allocate section offsets */
	switch(sub_header->version)
	{
		case SRT0_V4: /* 1 section */
			sub_header->section_offsets = (uint32_t*)calloc(1, 4*SRT0_V4_SECTIONS);
			break;
		case SRT0_V5: /* 2 sections */
			sub_header->section_offsets = (uint32_t*)calloc(1, 4*SRT0_V5_SECTIONS);
			break;
	}
	
	/* Read all common offsets */
	sub_header->section_offsets[SRT0_SECTION_ANIM_DATA] = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	
	/* Read an offset only present in version 5 */
	if(sub_header->version == SRT0_V5)
	{
		sub_header->section_offsets[SRT0_SECTION_UNKNOWN] = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	}
	
	/* Read name offset */
	sub_header->name_offset = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	
	/* Read a name into helper fields */
	sub_header->name = brres_read_str_and_back(srt0,
											   sub_header->name_offset,
											   &sub_header->name_size);
}

void srt0_read_header(FU_FILE* srt0, SRT0_HEADER* header)
{
	header->unk_0 = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	header->frame_count = fu_read_u16(srt0, NULL, FU_BIG_ENDIAN);
	header->anim_data_count = fu_read_u16(srt0, NULL, FU_BIG_ENDIAN);
	header->matrix_mode = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	header->looping = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
}

void srt0_read_groups(FU_FILE* srt0, SRT0_FILE* srt0f)
{
	const uint64_t base_offset = fu_tell(srt0);
	
	brres_read_group(srt0, &srt0f->group);
	
	srt0f->anim_data = (SRT0_TEX_ANIM_DATA*)calloc(1, sizeof(SRT0_TEX_ANIM_DATA)*srt0f->group.entries_count);
	
	for(uint32_t i = 0; i != srt0f->group.entries_count; ++i)
	{
		fu_seek(srt0, base_offset+srt0f->group.entries[i+1].data_offset, FU_SEEK_SET);
		srt0_read_tex_anim_data(srt0, &srt0f->anim_data[i]);
	}
}

void srt0_read_tex_anim_data(FU_FILE* srt0, SRT0_TEX_ANIM_DATA* anim_data)
{
	const uint64_t base_offset = fu_tell(srt0);
	
	anim_data->material_name_offset = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	anim_data->texture_flags = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	anim_data->unk_08 = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	
	anim_data->m_bits = srt0_count_bits32(anim_data->texture_flags);
	
	anim_data->entry_offsets = (uint32_t*)calloc(1, 4*anim_data->m_bits);
	anim_data->entries = (SRT0_TEX_ANIM_DESC*)calloc(1, sizeof(SRT0_TEX_ANIM_DESC)*anim_data->m_bits);
	
	for(uint32_t i = 0; i != anim_data->m_bits; ++i)
	{
		anim_data->entry_offsets[i] = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	}
	
	for(uint32_t i = 0; i != anim_data->m_bits; ++i)
	{
		fu_seek(srt0, base_offset + anim_data->entry_offsets[i], FU_SEEK_SET);
		srt0_read_tex_anim_desc(srt0, &anim_data->entries[i]);
	}
	
	/* Name */
	const uint32_t base_and_material_offset = base_offset + anim_data->material_name_offset;
	
	anim_data->material_name = brres_read_str_and_back(srt0,
													   base_and_material_offset,
													   &anim_data->material_name_size);
}

void srt0_read_tex_anim_desc(FU_FILE* srt0, SRT0_TEX_ANIM_DESC* anim_desc)
{
	anim_desc->code.data = fu_read_u32(srt0, NULL, FU_LITTLE_ENDIAN);
	
	if(anim_desc->code.fixed_x_scale)
	{
		anim_desc->frame_data[0].frame_count = 1;
		anim_desc->frame_data[0].unk_02 = 0;
		anim_desc->frame_data[0].frame_scale = 0.f;
		if(anim_desc->code.scale_one)
		{
			anim_desc->frame_data[0].fixed_val = 1.f;
			anim_desc->frame_data[0].fixed = 1;
		}
		anim_desc->frame_data[0].keyframes = NULL;
	}
	else
	{
		srt0_read_tex_frame_data(srt0, &anim_desc->frame_data[0]);
	}
	
	if(anim_desc->code.fixed_y_scale)
	{	
		anim_desc->frame_data[1].frame_count = 1;
		anim_desc->frame_data[1].unk_02 = 0;
		anim_desc->frame_data[1].frame_scale = 0.f;
		if(anim_desc->code.scale_one)
		{
			anim_desc->frame_data[1].fixed_val = 1.f;
			anim_desc->frame_data[1].fixed = 1;
		}
		anim_desc->frame_data[1].keyframes = NULL;
	}
	else
	{
		srt0_read_tex_frame_data(srt0, &anim_desc->frame_data[1]);
	}
	
	if(anim_desc->code.fixed_rotation)
	{
		anim_desc->frame_data[2].frame_count = 1;
		anim_desc->frame_data[2].unk_02 = 0;
		anim_desc->frame_data[2].frame_scale = 0.f;
		if(anim_desc->code.rot_zero)
		{
			anim_desc->frame_data[2].fixed_val = 0.f;
			anim_desc->frame_data[2].fixed = 1;
		}
		anim_desc->frame_data[2].keyframes = NULL;
	}
	else
	{
		srt0_read_tex_frame_data(srt0, &anim_desc->frame_data[2]);
	}
	
	if(anim_desc->code.fixed_x_translation)
	{
		anim_desc->frame_data[3].frame_count = 1;
		anim_desc->frame_data[3].unk_02 = 0;
		anim_desc->frame_data[3].frame_scale = 0.f;
		if(anim_desc->code.translate_zero)
		{
			anim_desc->frame_data[3].fixed_val = 0.f;
			anim_desc->frame_data[3].fixed = 1;
		}
		anim_desc->frame_data[3].keyframes = NULL;
	}
	else
	{
		srt0_read_tex_frame_data(srt0, &anim_desc->frame_data[3]);
	}
	
	if(anim_desc->code.fixed_y_translation)
	{
		anim_desc->frame_data[4].frame_count = 1;
		anim_desc->frame_data[4].unk_02 = 0;
		anim_desc->frame_data[4].frame_scale = 0.f;
		if(anim_desc->code.translate_zero)
		{
			anim_desc->frame_data[4].fixed_val = 0.f;
			anim_desc->frame_data[4].fixed = 1;
		}
		anim_desc->frame_data[4].keyframes = NULL;
	}
	else
	{
		srt0_read_tex_frame_data(srt0, &anim_desc->frame_data[4]);
	}
}

void srt0_read_tex_frame_data(FU_FILE* srt0, SRT0_TEX_ANIM_FRAME_DATA* frame_data)
{
	const uint64_t base_offset = fu_tell(srt0);
	const uint32_t offset = fu_read_u32(srt0, NULL, FU_BIG_ENDIAN);
	
	fu_seek(srt0, base_offset + offset, FU_SEEK_SET);
	
	frame_data->frame_count = fu_read_u16(srt0, NULL, FU_BIG_ENDIAN);
	frame_data->unk_02 = fu_read_u16(srt0, NULL, FU_BIG_ENDIAN);
	frame_data->frame_scale = fu_read_f32(srt0, NULL, FU_BIG_ENDIAN);
	
	frame_data->keyframes = (SRT0_TEX_ANIM_FRAME_INFO*)calloc(1, sizeof(SRT0_TEX_ANIM_FRAME_INFO)*frame_data->frame_count);

	for(uint32_t i = 0; i != frame_data->frame_count; ++i)
	{
		frame_data->keyframes[i].index = fu_read_f32(srt0, NULL, FU_BIG_ENDIAN);
		frame_data->keyframes[i].value = fu_read_f32(srt0, NULL, FU_BIG_ENDIAN);
		frame_data->keyframes[i].tangent = fu_read_f32(srt0, NULL, FU_BIG_ENDIAN);
	}
	
	fu_seek(srt0, base_offset+4, FU_SEEK_SET);
}

uint8_t srt0_count_bits32(uint32_t val)
{
    uint8_t c = 0;
    while (val > 0)
	{
        if((val & 1) == 1) c++;
        val >>= 1;
    }
    return c;
}

void srt0_free(SRT0_FILE* srt0)
{	
	for(uint32_t i = 0; i != srt0->group.entries_count; ++i)
	{
		free(srt0->anim_data[i].entry_offsets);
		free(srt0->anim_data[i].entries);
		free(srt0->anim_data[i].material_name);
	}
	free(srt0->anim_data);
	
	brres_free_sub_header(&srt0->sub_header);
	brres_free_group(&srt0->group);
}

SRT0_FILE* srt0_alloc_file()
{
	return (SRT0_FILE*)calloc(1, sizeof(SRT0_FILE));
}

void srt0_print(SRT0_FILE* srt0)
{
	BRRES_SUB_HEADER* subh = &srt0->sub_header;
	SRT0_HEADER* head = &srt0->header;
	
	printf("====BRRES HEADER\n");
	printf("Magic: %.4s\n", subh->magic);
	printf("Length: %u\n", subh->length);
	printf("Version: %u\n", subh->version);
	printf("Outer_brres_offset: %u\n", subh->outer_brres_offset);
	
	/* Common sections for versions 4 and 5 */
	printf("SRT0_SECTION_ANIM_DATA: %x\n", subh->section_offsets[SRT0_SECTION_ANIM_DATA]);
	
	if(subh->version == SRT0_V5)
	{
		printf("SRT0_SECTION_UNKNOWN: %x\n", subh->section_offsets[SRT0_SECTION_UNKNOWN]);
	}
	
	printf("Name offset: %u\n", subh->name_offset);
	printf("Name: %s\n", subh->name);
	printf("Name size: %u\n", subh->name_size);
	
	printf("====SRT0 HEADER\n");
	printf("unk_0: %u\n", head->unk_0);
	printf("frame_count: %hu\n", head->frame_count);
	printf("anim_data_count: %hu\n", head->anim_data_count);
	printf("matrix_mode: %s\n", SRT0_MATRIX_MODES_STR[head->matrix_mode]);
	printf("looping: %u\n", head->looping);
	
	brres_print_group(&srt0->group);
	
	for(uint32_t i = 0; i != srt0->group.entries_count; ++i)
	{
		SRT0_TEX_ANIM_DATA* tex_anim_data = &srt0->anim_data[i];
		
		printf("====TEX ANIM DATA[%u]\n", i);
		printf("material_name_offset: %x\n", tex_anim_data->material_name_offset);
		printf("texture_flags: %u\n", tex_anim_data->texture_flags);
		printf("m_bits: %u\n", tex_anim_data->m_bits);
		printf("unk_08: %u\n", tex_anim_data->unk_08);
		
		printf("material_name: %s\n", tex_anim_data->material_name);
		printf("material_name_size: %u\n", tex_anim_data->material_name_size);
		
		for(uint32_t j = 0; j != tex_anim_data->m_bits; ++j)
		{
			SRT0_TEX_ANIM_DESC* entry = &tex_anim_data->entries[j];
			
			printf("==Entry[%u]\n", j);
			printf("entry_offsets: %x\n", tex_anim_data->entry_offsets[j]);
			printf("always_set_unk: %x\n", entry->code.always_set_unk);
			printf("scale_one: %x\n", entry->code.scale_one);
			printf("rot_zero: %x\n", entry->code.rot_zero);
			printf("translate_zero: %x\n", entry->code.translate_zero);
			printf("isotropic_scale: %x\n", entry->code.isotropic_scale);
			printf("fixed_x_scale: %x\n", entry->code.fixed_x_scale);
			printf("fixed_y_scale: %x\n", entry->code.fixed_y_scale);
			printf("fixed_rotation: %x\n", entry->code.fixed_rotation);
			printf("fixed_x_translation: %x\n", entry->code.fixed_x_translation);
			printf("fixed_y_translation: %x\n", entry->code.fixed_y_translation);
			
			for(uint32_t k = 0; k != 5; ++k)
			{
				SRT0_TEX_ANIM_FRAME_DATA* frame_data = &entry->frame_data[k];
				
				printf("=Data[%u] - %s\n", k, SRT0_FRAME_DATA_NAME_STR[k]);
				printf("frame_count: %u\n", frame_data->frame_count);
				printf("unk_02: %u\n", frame_data->unk_02);
				printf("frame_scale: %f\n", frame_data->frame_scale);
				printf("fixed: %u\n", frame_data->fixed);
				printf("fixed_val: %f\n", frame_data->fixed_val);
				
				if(frame_data->fixed == 0)
				{
					for(uint32_t l = 0; l != frame_data->frame_count; ++l)
					{
						SRT0_TEX_ANIM_FRAME_INFO* kfs = &frame_data->keyframes[l];
						printf("Keyframe[%f]: %f | %f\n", kfs->index, kfs->value, kfs->tangent);
					}
				}
			}
		}
	}
}