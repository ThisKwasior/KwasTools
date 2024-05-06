#include "cam_anim.h"

#include <stdlib.h>

static const char* CAM_ANIM_KF_SET_TYPES[] =
{
	"CAM_POS_X",
	"CAM_POS_Z",
	"CAM_POS_Y",
	"CAM_ROT_X",
	"CAM_ROT_Z",
	"CAM_ROT_Y",
	"AIM_POS_X",
	"AIM_POS_Z",
	"AIM_POS_Y",
	"TWIST",
	"Z_NEAR",
	"Z_FAR",
	"Z_FOV",
	"ASPECT_RATIO",
};

CAM_ANIM_FILE* cam_anim_load_file(FU_FILE* file)
{
	CAM_ANIM_FILE* cam = (CAM_ANIM_FILE*)calloc(1, sizeof(CAM_ANIM_FILE));
	
	mirage_read_header(file, &cam->header);
	mirage_read_info(file, &cam->info);
	cam->keyframes = mirage_read_keyframes(file, &cam->info); 
	cam->string_table = mirage_read_string_table(file, &cam->info); 
	mirage_read_footer(file, &cam->header, &cam->footer);

	cam_anim_load_metadata(file, cam);
	cam_anim_load_entries(file, cam);
	
	return cam;
}

FU_FILE* cam_anim_save_to_fu_file(CAM_ANIM_FILE* cam)
{
	FU_FILE* cam_file = (FU_FILE*)calloc(1, sizeof(FU_FILE));
	printf("create mem file: %u\n", fu_create_mem_file(cam_file));
	
	fu_change_buf_size(cam_file, cam->header.file_size);
	
	/* Mirage header */
	mirage_write_header(cam_file, &cam->header);
	
	/* Mirage info */
	mirage_write_info(cam_file, &cam->info);
	
	/* UV metadata */
	cam_anim_write_metadata(cam_file, cam);
	
	/* UV entries */
	cam_anim_write_entries(cam_file, cam);
	
	/* Keyframes */
	mirage_write_keyframes(cam_file, &cam->info, cam->keyframes);
	
	/* String table */
	mirage_write_string_table(cam_file, &cam->info, cam->string_table);
	
	/* Mirage footer */
	mirage_write_footer(cam_file, &cam->header, &cam->footer);
	
	return cam_file;
}

void cam_anim_free_uv_file(CAM_ANIM_FILE* cam)
{
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		free(cam->entries[i].keyframe_sets);
	}
	
	if(cam->entries) free(cam->entries);
	if(cam->keyframes) free(cam->keyframes);
	if(cam->string_table) free(cam->string_table);
}

void cam_anim_load_metadata(FU_FILE* file, CAM_ANIM_FILE* cam)
{
	uint8_t status = 0;
	
	fu_seek(file, MIRAGE_HEADER_SIZE + cam->info.metadata_offset, FU_SEEK_SET);

	cam->metadata.anim_count = fu_read_u32(file, &status, FU_BIG_ENDIAN);
	cam->metadata.anim_offsets = (uint32_t*)calloc(sizeof(uint32_t), cam->metadata.anim_count);
	
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		cam->metadata.anim_offsets[i] = fu_read_u32(file, &status, FU_BIG_ENDIAN);
	}
}

void cam_anim_write_metadata(FU_FILE* file, CAM_ANIM_FILE* cam)
{
	fu_seek(file, MIRAGE_HEADER_SIZE + cam->info.metadata_offset, FU_SEEK_SET);
	
	fu_write_u32(file, cam->metadata.anim_count, FU_BIG_ENDIAN);
	
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		fu_write_u32(file, cam->metadata.anim_offsets[i], FU_BIG_ENDIAN);
	}
}

void cam_anim_write_entries(FU_FILE* file, CAM_ANIM_FILE* cam)
{
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		fu_seek(file, MIRAGE_HEADER_SIZE + cam->metadata.anim_offsets[i], FU_SEEK_SET);
		
		CAM_ANIM_ENTRY* ce = &cam->entries[i];
		
		fu_write_u32(file, ce->name_offset, FU_BIG_ENDIAN);
		fu_write_u8(file, ce->rot_or_aim);
		fu_write_u8(file, ce->flag2);
		fu_write_u8(file, ce->flag3);
		fu_write_u8(file, ce->flag4);
		fu_write_f32(file, ce->frame_rate, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->start_frame, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->end_frame, FU_BIG_ENDIAN);
		fu_write_u32(file, ce->keyframe_set_count, FU_BIG_ENDIAN);

		fu_write_f32(file, ce->cam_position.x, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->cam_position.z, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->cam_position.y, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->cam_rotation.x, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->cam_rotation.z, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->cam_rotation.y, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->aim_position.x, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->aim_position.z, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->aim_position.y, FU_BIG_ENDIAN);

		fu_write_f32(file, ce->twist, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->z_near, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->z_far, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->fov, FU_BIG_ENDIAN);
		fu_write_f32(file, ce->aspect_ratio, FU_BIG_ENDIAN);
		
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

void cam_anim_load_entries(FU_FILE* file, CAM_ANIM_FILE* cam)
{
	uint8_t status = 0;
	
	cam->entries = (CAM_ANIM_ENTRY*)calloc(cam->metadata.anim_count, sizeof(CAM_ANIM_ENTRY));
	
	/* Entries */
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		CAM_ANIM_ENTRY* entry = &cam->entries[i];
		
		entry->name_offset = fu_read_u32(file, &status, FU_BIG_ENDIAN);
		entry->rot_or_aim = fu_read_u8(file, &status);
		entry->flag2 = fu_read_u8(file, &status);
		entry->flag3 = fu_read_u8(file, &status);
		entry->flag4 = fu_read_u8(file, &status);
		entry->frame_rate = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->start_frame = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->end_frame = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->keyframe_set_count = fu_read_u32(file, &status, FU_BIG_ENDIAN);

		entry->cam_position.x = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->cam_position.z = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->cam_position.y = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->cam_rotation.x = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->cam_rotation.z = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->cam_rotation.y = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->aim_position.x = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->aim_position.z = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->aim_position.y = fu_read_f32(file, &status, FU_BIG_ENDIAN);

		entry->twist = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->z_near = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->z_far = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->fov = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		entry->aspect_ratio = fu_read_f32(file, &status, FU_BIG_ENDIAN);
		
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
		
		entry->name = &cam->string_table[entry->name_offset];
	}
}

/*
	Print functions
*/
void cam_anim_print_metadata(CAM_ANIM_FILE* cam)
{
	printf("====Metadata\n");

	printf("\tAnim count: %u\n", cam->metadata.anim_count);
	
	for(uint32_t j = 0; j != cam->metadata.anim_count; ++j)
	{
		printf("\t\tAnim offset[%u]: %08x\n", j, cam->metadata.anim_offsets[j]);
	}
}

void cam_anim_print_entries(CAM_ANIM_FILE* cam)
{
	printf("====Entries\n");
	
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		printf("\tEntry[%u/%u]\n", i+1, cam->metadata.anim_count);
		printf("\t\tName offset: 0x%x | %s\n", cam->entries[i].name_offset,
											   cam->entries[i].name);
		printf("\t\tRotation or Aim: %u\n", cam->entries[i].rot_or_aim);
		printf("\t\tFlag2: %u\n", cam->entries[i].flag2);
		printf("\t\tFlag3: %u\n", cam->entries[i].flag3);
		printf("\t\tFlag4: %u\n", cam->entries[i].flag4);
		printf("\t\tFrame rate: %f\n", cam->entries[i].frame_rate);
		printf("\t\tStart frame: %f\n", cam->entries[i].start_frame);
		printf("\t\tEnd frame: %f\n", cam->entries[i].end_frame);
		printf("\t\tKeyframe set count: %u\n", cam->entries[i].keyframe_set_count);

		printf("\t\tCam pos: (%f,%f,%f)\n", cam->entries[i].cam_position.x,
											cam->entries[i].cam_position.z,
											cam->entries[i].cam_position.y);
		printf("\t\tCam rot: (%f,%f,%f)\n", cam->entries[i].cam_rotation.x,
											cam->entries[i].cam_rotation.z,
											cam->entries[i].cam_rotation.y);
		printf("\t\tAim pos: (%f,%f,%f)\n", cam->entries[i].aim_position.x,
											cam->entries[i].aim_position.z,
											cam->entries[i].aim_position.y);

		printf("\t\tAim z rot: %f\n", cam->entries[i].twist);
		printf("\t\tZ near: %f\n", cam->entries[i].z_near);
		printf("\t\tZ far: %f\n", cam->entries[i].z_far);
		printf("\t\tFOV: %f\n", cam->entries[i].fov);
		printf("\t\tAspect ratio: %f\n", cam->entries[i].aspect_ratio);
		
		for(uint32_t j = 0; j != cam->entries[i].keyframe_set_count; ++j)
		{
			MIRAGE_KEYFRAME_SET* kfs = &cam->entries[i].keyframe_sets[j];
			printf("\t\t\tKeyframe set[%u]\n", j);
			printf("\t\t\t\tType: %u (%s)\n", kfs->type, CAM_ANIM_KF_SET_TYPES[kfs->type]);
			printf("\t\t\t\tFlag2: %u\n", kfs->flag2);
			printf("\t\t\t\tInterpolation: %u\n", kfs->interpolation);
			printf("\t\t\t\tFlag4: %u\n", kfs->flag4);
			printf("\t\t\t\tLength: %u\n", kfs->length);
			printf("\t\t\t\tStart: %u\n", kfs->start);
		}
	}
}

void cam_anim_print_cam(CAM_ANIM_FILE* cam)
{
	/* Header */
	mirage_print_header(&cam->header);

	/* Info */
	mirage_print_info(&cam->info);
	
	/* String table */	
	mirage_print_string_table(&cam->info, cam->string_table);
	
	/* Mirage offsets */
	mirage_print_offsets(&cam->footer);
	
	/* UV Metadata */
	cam_anim_print_metadata(cam);
	
	/* UV Entries */
	cam_anim_print_entries(cam);
	
	/* Keyframes */
	/* mirage_print_keyframes(&uv->info, uv->keyframes); */
	
}