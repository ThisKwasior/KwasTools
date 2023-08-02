#include "uv_anim.h"

#include <stdlib.h>

UV_ANIM_FILE* uv_anim_load_file(FU_FILE* file)
{
	UV_ANIM_FILE* uv = (UV_ANIM_FILE*)calloc(1, sizeof(UV_ANIM_FILE));
	
	mirage_read_header(file, &uv->header);
	mirage_read_info(file, &uv->info);
	uv->keyframes = mirage_read_keyframes(file, &uv->info); 
	uv->string_table = mirage_read_string_table(file, &uv->info); 
	mirage_read_footer(file, &uv->header, &uv->footer);

	uv_anim_load_metadata(file, uv);
	uv_anim_load_entries(file, uv);
	
	return uv;
}

FU_FILE* uv_anim_save_to_fu_file(UV_ANIM_FILE* uv)
{
	FU_FILE* uv_file = (FU_FILE*)calloc(1, sizeof(FU_FILE));
	fu_create_mem_file(uv_file);
	
	fu_change_buf_size(uv_file, uv->header.file_size);
	
	/* Mirage header */
	mirage_write_header(uv_file, &uv->header);
	
	/* Mirage info */
	mirage_write_info(uv_file, &uv->info);
	
	/* UV metadata */
	uv_anim_write_metadata(uv_file, uv);
	
	/* UV entries */
	uv_anim_write_entries(uv_file, uv);
	
	/* Keyframes */
	mirage_write_keyframes(uv_file, &uv->info, uv->keyframes);
	
	/* String table */
	mirage_write_string_table(uv_file, &uv->info, uv->string_table);
	
	/* Mirage footer */
	mirage_write_footer(uv_file, &uv->header, &uv->footer);
	
	return uv_file;
}

void uv_anim_free_uv_file(UV_ANIM_FILE* uv)
{
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		free(uv->entries[i].keyframe_sets);
	}
	
	if(uv->entries) free(uv->entries);
	if(uv->keyframes) free(uv->keyframes);
	if(uv->string_table) free(uv->string_table);
}

void uv_anim_load_metadata(FU_FILE* file, UV_ANIM_FILE* uv)
{
	uint8_t status = 0;
	
	fu_seek(file, MIRAGE_HEADER_SIZE + uv->info.metadata_offset, FU_SEEK_SET);
	
	uv->metadata.material_name_offset = fu_read_u32(file, &status, FU_BIG_ENDIAN);
	uv->metadata.texture_name_offset = fu_read_u32(file, &status, FU_BIG_ENDIAN);
	uv->metadata.anim_count = fu_read_u32(file, &status, FU_BIG_ENDIAN);
	
	uv->metadata.anim_offsets = (uint32_t*)calloc(sizeof(uint32_t), uv->metadata.anim_count);
	
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		uv->metadata.anim_offsets[i] = fu_read_u32(file, &status, FU_BIG_ENDIAN);
	}
	
	uv->metadata.material_name = (char*)&uv->string_table[uv->metadata.material_name_offset];
	uv->metadata.texture_name = (char*)&uv->string_table[uv->metadata.texture_name_offset];
}

void uv_anim_write_metadata(FU_FILE* file, UV_ANIM_FILE* uv)
{
	fu_seek(file, MIRAGE_HEADER_SIZE + uv->info.metadata_offset, FU_SEEK_SET);
	
	fu_write_u32(file, uv->metadata.material_name_offset, FU_BIG_ENDIAN);
	fu_write_u32(file, uv->metadata.texture_name_offset, FU_BIG_ENDIAN);
	fu_write_u32(file, uv->metadata.anim_count, FU_BIG_ENDIAN);
	
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		fu_write_u32(file, uv->metadata.anim_offsets[i], FU_BIG_ENDIAN);
	}
}

void uv_anim_write_entries(FU_FILE* file, UV_ANIM_FILE* uv)
{
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		fu_seek(file, MIRAGE_HEADER_SIZE + uv->metadata.anim_offsets[i], FU_SEEK_SET);
		
		UV_ANIM_ENTRY* ce = &uv->entries[i];
		
		fu_write_u32(file, ce->name_offset, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->frame_rate, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->start_frame, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->end_frame, FU_BIG_ENDIAN);
		fu_write_u32(file, ce->keyframe_set_count, FU_BIG_ENDIAN);
		
		for(uint32_t j = 0; j != ce->keyframe_set_count; ++j)
		{
			MIRAGE_KEYFRAME_SET* kfs = &ce->keyframe_sets[j];
			
			fu_write_u8(file, kfs->type);
			fu_write_u8(file, kfs->flag2);
			fu_write_u8(file, kfs->interpolation);
			fu_write_u8(file, kfs->flag4);
			fu_write_u32(file, kfs->length, FU_BIG_ENDIAN);
			fu_write_u32(file, kfs->start, FU_BIG_ENDIAN);
		}
	}
}

void uv_anim_load_entries(FU_FILE* file, UV_ANIM_FILE* uv)
{
	uint8_t status = 0;
	
	uv->entries = (UV_ANIM_ENTRY*)calloc(uv->metadata.anim_count, sizeof(UV_ANIM_ENTRY));
	
	/* Entries */
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		UV_ANIM_ENTRY* entry = &uv->entries[i];
		
		entry->name_offset = fu_read_u32(file, &status, FU_BIG_ENDIAN);
		entry->frame_rate = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->start_frame = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->end_frame = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->keyframe_set_count = fu_read_u32(file, &status, FU_BIG_ENDIAN);
		
		entry->keyframe_sets = (MIRAGE_KEYFRAME_SET*)calloc(entry->keyframe_set_count,
															 sizeof(MIRAGE_KEYFRAME_SET));
												
		/* Keyframe sets */
		for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
		{
			MIRAGE_KEYFRAME_SET* set = &entry->keyframe_sets[j];
			
			set->type = fu_read_u8(file, &status);
			set->flag2 = fu_read_u8(file, &status);
			set->interpolation = fu_read_u8(file, &status);
			set->flag4 = fu_read_u8(file, &status);
			set->length = fu_read_u32(file, &status, FU_BIG_ENDIAN);
			set->start = fu_read_u32(file, &status, FU_BIG_ENDIAN);
		}
		
		entry->name = &uv->string_table[entry->name_offset];
	}
}

/*
	Print functions
*/
void uv_anim_print_metadata(UV_ANIM_FILE* uv)
{
	printf("====Metadata\n");

	printf("\tMaterial name offset: 0x%x | %s\n", uv->metadata.material_name_offset,
													uv->metadata.material_name);
	printf("\tTexture name offset: 0x%x | %s\n", uv->metadata.texture_name_offset,
														uv->metadata.texture_name);
	printf("\tAnim count: %u\n", uv->metadata.anim_count);
	
	for(uint32_t j = 0; j != uv->metadata.anim_count; ++j)
	{
		printf("\t\tAnim offset[%u]: %08x\n", j, uv->metadata.anim_offsets[j]);
	}
}

void uv_anim_print_entries(UV_ANIM_FILE* uv)
{
	printf("====Entries\n");
	
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		printf("\tEntry[%u/%u]\n", i+1, uv->metadata.anim_count);
		printf("\t\tName offset: 0x%x | %s\n", uv->entries[i].name_offset,
											   uv->entries[i].name);
		printf("\t\tFrame rate: %f\n", uv->entries[i].frame_rate);
		printf("\t\tStart frame: %f\n", uv->entries[i].start_frame);
		printf("\t\tEnd frame: %f\n", uv->entries[i].end_frame);
		printf("\t\tKeyframe set count: %u\n", uv->entries[i].keyframe_set_count);
		
		for(uint32_t j = 0; j != uv->entries[i].keyframe_set_count; ++j)
		{
			MIRAGE_KEYFRAME_SET* kfs = &uv->entries[i].keyframe_sets[j];
			printf("\t\t\tKeyframe set[%u]\n", j);
			printf("\t\t\t\tType: %u\n", kfs->type);
			printf("\t\t\t\tFlag2: %u\n", kfs->flag2);
			printf("\t\t\t\tInterpolation: %u\n", kfs->interpolation);
			printf("\t\t\t\tFlag4: %u\n", kfs->flag4);
			printf("\t\t\t\tLength: %u\n", kfs->length);
			printf("\t\t\t\tStart: %u\n", kfs->start);
		}
	}
}

void uv_anim_print_uv(UV_ANIM_FILE* uv)
{
	/* Header */
	mirage_print_header(&uv->header);

	/* Info */
	mirage_print_info(&uv->info);
	
	/* String table */	
	mirage_print_string_table(&uv->info, uv->string_table);
	
	/* Mirage offsets */
	mirage_print_offsets(&uv->footer);
	
	/* UV Metadata */
	uv_anim_print_metadata(uv);
	
	/* UV Entries */
	uv_anim_print_entries(uv);
	
	/* Keyframes */
	/* mirage_print_keyframes(&uv->info, uv->keyframes); */
	
}