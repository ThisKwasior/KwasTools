#include "mirage.h"

#include <stdlib.h>

void mirage_read_header(FU_FILE* mirage, MIRAGE_HEADER* header)
{
	fu_seek(mirage, 0, FU_SEEK_SET);
	header->file_size = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	header->root_node_type = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	header->root_node_size = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	header->root_node_offset = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	header->footer_offset = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	header->file_end_offset = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
}

void mirage_read_info(FU_FILE* mirage, MIRAGE_INFO* info)
{
	fu_seek(mirage, MIRAGE_HEADER_SIZE, FU_SEEK_SET);
	info->metadata_offset = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	info->metadata_size = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	info->keyframes_offset = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	info->keyframes_size = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	info->string_table_offset = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	info->string_table_size = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
}

MIRAGE_KEYFRAME* mirage_read_keyframes(FU_FILE* mirage, MIRAGE_INFO* info)
{
	const uint32_t kfs_count = info->keyframes_size/MIRAGE_KEYFRAME_SIZE;
	MIRAGE_KEYFRAME* kfs = (MIRAGE_KEYFRAME*)calloc(kfs_count, MIRAGE_KEYFRAME_SIZE);
	
	if(kfs == NULL) return NULL;
	
	fu_seek(mirage, MIRAGE_HEADER_SIZE + info->keyframes_offset, FU_SEEK_SET);
	
	for(uint32_t i = 0; i != kfs_count; ++i)
	{
		kfs[i].index = fu_read_f32(mirage, NULL, FU_BIG_ENDIAN);
		kfs[i].value = fu_read_f32(mirage, NULL, FU_BIG_ENDIAN);
	}
	
	return kfs;
}

char* mirage_read_string_table(FU_FILE* mirage, MIRAGE_INFO* info)
{
	char* string_table = (char*)calloc(1, info->string_table_size);
	
	if(string_table == NULL) return NULL;
	
	fu_seek(mirage, MIRAGE_HEADER_SIZE + info->string_table_offset, FU_SEEK_SET);
	fu_read_data(mirage, (uint8_t*)string_table, info->string_table_size, NULL);
	
	return string_table;
}

void mirage_read_footer(FU_FILE* mirage, MIRAGE_HEADER* header, MIRAGE_FOOTER* footer)
{
	fu_seek(mirage, header->footer_offset, FU_SEEK_SET);
	footer->offset_count = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	footer->offsets = (uint32_t*)calloc(sizeof(uint32_t), footer->offset_count);
	
	if(footer->offsets == NULL) return;
	
	for(uint32_t i = 0; i != footer->offset_count; ++i)
	{
		footer->offsets[i] = fu_read_u32(mirage, NULL, FU_BIG_ENDIAN);
	}
}

void mirage_write_header(FU_FILE* mirage, MIRAGE_HEADER* header)
{
	fu_seek(mirage, 0, FU_SEEK_SET);
	
	fu_write_u32(mirage, header->file_size, FU_BIG_ENDIAN);
	fu_write_u32(mirage, header->root_node_type, FU_BIG_ENDIAN);
	fu_write_u32(mirage, header->root_node_size, FU_BIG_ENDIAN);
	fu_write_u32(mirage, header->root_node_offset, FU_BIG_ENDIAN);
	fu_write_u32(mirage, header->footer_offset, FU_BIG_ENDIAN);
	fu_write_u32(mirage, header->file_end_offset, FU_BIG_ENDIAN);
}

void mirage_write_info(FU_FILE* mirage, MIRAGE_INFO* info)
{
	fu_seek(mirage, MIRAGE_HEADER_SIZE, FU_SEEK_SET);
	
	fu_write_u32(mirage, info->metadata_offset, FU_BIG_ENDIAN);
	fu_write_u32(mirage, info->metadata_size, FU_BIG_ENDIAN);
	fu_write_u32(mirage, info->keyframes_offset, FU_BIG_ENDIAN);
	fu_write_u32(mirage, info->keyframes_size, FU_BIG_ENDIAN);
	fu_write_u32(mirage, info->string_table_offset, FU_BIG_ENDIAN);
	fu_write_u32(mirage, info->string_table_size, FU_BIG_ENDIAN);
}

void mirage_write_keyframes(FU_FILE* mirage, MIRAGE_INFO* info, MIRAGE_KEYFRAME* keyframes)
{
	fu_seek(mirage, MIRAGE_HEADER_SIZE + info->keyframes_offset, FU_SEEK_SET);
	
	const uint32_t kfs_count = info->keyframes_size/MIRAGE_KEYFRAME_SIZE;
	
	for(uint32_t i = 0; i != kfs_count; ++i)
	{
		fu_write_f32(mirage, keyframes[i].index, FU_BIG_ENDIAN);
		fu_write_f32(mirage, keyframes[i].value, FU_BIG_ENDIAN);
	}
}

void mirage_write_string_table(FU_FILE* mirage, MIRAGE_INFO* info, char* string_table)
{
	fu_seek(mirage, MIRAGE_HEADER_SIZE + info->string_table_offset, FU_SEEK_SET);
	fu_write_data(mirage, (uint8_t*)string_table, info->string_table_size);
}

void mirage_write_footer(FU_FILE* mirage, MIRAGE_HEADER* header, MIRAGE_FOOTER* footer)
{
	fu_seek(mirage, header->footer_offset, FU_SEEK_SET);
	
	fu_write_u32(mirage, footer->offset_count, FU_BIG_ENDIAN);
	
	for(uint32_t i = 0; i != footer->offset_count; ++i)
	{
		fu_write_u32(mirage, footer->offsets[i], FU_BIG_ENDIAN);
	}
}

/*
	Print functions
*/
void mirage_print_header(MIRAGE_HEADER* header)
{
	printf("====Header\n");
	printf("\tFile size: %u\n", header->file_size);
	printf("\tRoot node type: %u\n", header->root_node_type);
	printf("\tRoot node size: %u\n", header->root_node_size);
	printf("\tRoot node offset: 0x%x\n", header->root_node_offset);
	printf("\tFooter offset: 0x%x\n", header->footer_offset);
	printf("\tFile end offset: 0x%x\n", header->file_end_offset);									 
}

void mirage_print_info(MIRAGE_INFO* info)
{
	printf("====Info\n");
	printf("\tMetadata offset: 0x%x\n", info->metadata_offset);
	printf("\tMetadata size: %u\n", info->metadata_size);
	printf("\tKeyframes offset: 0x%x\n", info->keyframes_offset);
	printf("\tKeyframes size: %u\n", info->keyframes_size);
	printf("\tString table offset: 0x%x\n", info->string_table_offset);
	printf("\tString table size: %u\n", info->string_table_size);
}

void mirage_print_keyframes(MIRAGE_INFO* info, MIRAGE_KEYFRAME* keyframes)
{
	const uint32_t kf_count = info->keyframes_size/8;
	
	printf("====Keyframes\n");
	for(uint32_t i = 0; i != kf_count; ++i)
	{
		MIRAGE_KEYFRAME* ckf = &keyframes[i];
		printf("Keyframe[%u] [%u]: %f\n", i, (uint32_t)ckf->index, ckf->value);
	}
}

void mirage_print_string_table(MIRAGE_INFO* info, char* string_table)
{
	printf("====String table[%u]\n\t", info->string_table_size);
	for(uint32_t i = 0; i != info->string_table_size; ++i)
	{
		char c = string_table[i];
		if(c == 0) c = ' ';
		printf("%c", c);
	}
	printf("\n");
}


void mirage_print_offsets(MIRAGE_FOOTER* footer)
{
	printf("====Footer offsets\n");
	for(uint32_t i = 0; i != footer->offset_count; ++i)
	{
		printf("\tOffset[%u/%u]: %08x\n", i+1, footer->offset_count, footer->offsets[i]);
	}
}