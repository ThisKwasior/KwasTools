#include "brres.h"

#include <stdlib.h>

void brres_read_group(FU_FILE* brres, BRRES_GROUP* group)
{
	const uint64_t base_offset = fu_tell(brres);
	
	group->length = fu_read_u32(brres, NULL, FU_BIG_ENDIAN);
	group->entries_count = fu_read_u32(brres, NULL, FU_BIG_ENDIAN);
	group->entries = (BRRES_ENTRY*)calloc(sizeof(BRRES_ENTRY), group->entries_count + 1);
	
	for(uint32_t i = 0; i != group->entries_count+1; ++i)
	{
		BRRES_ENTRY* entry = &group->entries[i];
		
		brres_read_entry(brres, entry);
		
		if(group->entries[i].name_offset != 0)
		{
			const uint64_t cur_offset = fu_tell(brres);
			
			fu_seek(brres, base_offset + entry->name_offset - 4, FU_SEEK_SET);
			entry->name_size = fu_read_u32(brres, NULL, FU_BIG_ENDIAN);
			
			entry->name = (char*)calloc(1, entry->name_size+1);
			fu_read_data(brres, (uint8_t*)&entry->name[0], entry->name_size, NULL);
			
			fu_seek(brres, cur_offset, FU_SEEK_SET);
		}
	}
}

void brres_read_entry(FU_FILE* brres, BRRES_ENTRY* entry)
{
	entry->entry_id = fu_read_u16(brres, NULL, FU_BIG_ENDIAN);
	entry->flag = fu_read_u16(brres, NULL, FU_BIG_ENDIAN);
	entry->left_idx = fu_read_u16(brres, NULL, FU_BIG_ENDIAN);
	entry->right_idx = fu_read_u16(brres, NULL, FU_BIG_ENDIAN);
	entry->name_offset = fu_read_u32(brres, NULL, FU_BIG_ENDIAN);
	entry->data_offset = fu_read_u32(brres, NULL, FU_BIG_ENDIAN);
}

void brres_read_keyframe_set(FU_FILE* brres, BRRES_KEYFRAME_SET* kfs)
{
	kfs->frame_count = fu_read_u16(brres, NULL, FU_BIG_ENDIAN);
	kfs->unk_02 = fu_read_u16(brres, NULL, FU_BIG_ENDIAN);
	kfs->frame_scale = fu_read_f32(brres, NULL, FU_BIG_ENDIAN);
	
	/* Read keyframes */
	kfs->keyframes = (BRRES_KEYFRAME*)calloc(kfs->frame_count, sizeof(BRRES_KEYFRAME));
	
	for(uint16_t i = 0; i != kfs->frame_count; ++i)
	{
		brres_read_keyframe(brres, &kfs->keyframes[i]);
	}
}

void brres_read_keyframe(FU_FILE* brres, BRRES_KEYFRAME* keyframe)
{
	keyframe->index = fu_read_f32(brres, NULL, FU_BIG_ENDIAN);
	keyframe->value = fu_read_f32(brres, NULL, FU_BIG_ENDIAN);
	keyframe->tangent = fu_read_f32(brres, NULL, FU_BIG_ENDIAN);
}

void brres_free_sub_header(BRRES_SUB_HEADER* sub_header)
{
	free(sub_header->section_offsets);
	free(sub_header->name);
}

void brres_free_group(BRRES_GROUP* group)
{
	for(uint32_t i = 0; i != group->entries_count+1; ++i)
	{
		if(group->entries[i].name_offset != 0)
		{
			free(group->entries[i].name);
		}
	}
	
	free(group->entries);
}

void brres_print_group(BRRES_GROUP* group)
{
	printf("====BRRES GROUP\n");
	printf("Length: %u\n", group->length);
	printf("Entries: %u\n", group->entries_count);
	
	for(uint32_t i = 0; i != group->entries_count+1; ++i)
	{
		BRRES_ENTRY* entry = &group->entries[i];
		printf("==Entry[%u]\n", i);
		printf("Entry ID: %u\n", entry->entry_id);
		printf("Flag: %u\n", entry->flag);
		printf("left_idx: %u\n", entry->left_idx);
		printf("right_idx: %u\n", entry->right_idx);
		printf("Name offset: %x\n", entry->right_idx);
		printf("Data offset: %x\n", entry->right_idx);
		
		printf("Name: %s\n", entry->name);
		printf("Name size: %x\n", entry->name_size);
	}
}

char* brres_read_str_and_back(FU_FILE* file, const uint32_t offset, uint32_t* name_size)
{
	const uint64_t cur_offset = fu_tell(file);
	char* str = NULL;
	
	fu_seek(file, offset - 4, FU_SEEK_SET);
	*name_size = fu_read_u32(file, NULL, FU_BIG_ENDIAN);
	
	str = (char*)calloc(1, (*name_size)+1);
	fu_read_data(file, (uint8_t*)&str[0], (*name_size), NULL);
	
	fu_seek(file, cur_offset, FU_SEEK_SET);
	
	return str;
}