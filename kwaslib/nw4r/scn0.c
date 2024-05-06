#include "scn0.h"

#include <stdlib.h>
#include <string.h>

SCN0_FILE* scn0_read_file(FU_FILE* scn0)
{
	SCN0_FILE* scn0f = scn0_alloc_file();
	
	if(scn0f == NULL) return NULL;
	
	scn0_read_sub_header(scn0, &scn0f->sub_header);
	
	if(strncmp(&scn0f->sub_header.magic[0], SCN0_MAGIC, 4) != 0)
	{
		free(scn0f);
		return NULL;
	}
	
	scn0_read_header(scn0, &scn0f->header);
	
	scn0_read_sections(scn0, scn0f);
	
	return scn0f;
}

void scn0_read_sub_header(FU_FILE* scn0, BRRES_SUB_HEADER* sub_header)
{
	/* Common in all sub headers */
	fu_read_data(scn0, (uint8_t*)&sub_header->magic[0], 4, NULL);
	sub_header->length = fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	sub_header->version = fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	sub_header->outer_brres_offset = fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);

	/* Allocate section offsets */
	switch(sub_header->version)
	{
		case SCN0_V4: /* 6 sections */
			sub_header->section_offsets = (uint32_t*)calloc(1, 4*SCN0_V4_SECTIONS);
			break;
		case SCN0_V5: /* 7 sections */
			sub_header->section_offsets = (uint32_t*)calloc(1, 4*SCN0_V5_SECTIONS);
			break;
	}
	
	/* Read all common offsets */
	for(uint32_t i = 0; i != SCN0_V4_SECTIONS; ++i)
	{
		sub_header->section_offsets[i] = fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	}
	
	/* Read an offset only present in version 5 */
	if(sub_header->version == SCN0_V5)
	{
		sub_header->section_offsets[SCN0_SECTION_7] = fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	}
	
	/* Read name offset */
	sub_header->name_offset = fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	
	/* Read a name into helper fields */
	sub_header->name = brres_read_str_and_back(scn0,
											   sub_header->name_offset,
											   &sub_header->name_size);
}

void scn0_read_header(FU_FILE* scn0, SCN0_HEADER* header)
{
	header->unk_0				= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	header->frame_count			= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->spec_light_count	= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->looping				= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	header->light_sets			= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->ambient_count		= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->light_count			= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->fog_count			= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->cam_count			= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
	header->unk_count			= fu_read_u16(scn0, NULL, FU_BIG_ENDIAN);
}

void scn0_read_sections(FU_FILE* scn0, SCN0_FILE* scn0f)
{
	
	/* TODO: Implement everything else besides camera */
	
	/* Camera */
	if(scn0f->sub_header.section_offsets[SCN0_SECTION_CAMERA])
	{
		fu_seek(scn0, scn0f->sub_header.section_offsets[SCN0_SECTION_CAMERA], SEEK_SET);
		
		const uint16_t cam_count = scn0f->header.cam_count;
		
		scn0f->sections[SCN0_SECTION_CAMERA] = (SCN0_SECTION*)calloc(cam_count, sizeof(SCN0_SECTION));
	
		SCN0_SECTION* section = scn0f->sections[SCN0_SECTION_CAMERA];
		
		section->kf_sets = (BRRES_KEYFRAME_SET*)calloc(15, sizeof(BRRES_KEYFRAME_SET));
		
		for(uint16_t i = 0; i != cam_count; ++i)
		{
			scn0_read_section_header(scn0, &section->header);
			scn0_read_camera(scn0, &section->cam);
			scn0_read_camera_kf_sets(scn0, section);
		}
	}
}

void scn0_read_section_header(FU_FILE* scn0, SCN0_SECTION_HEADER* header)
{
	const uint64_t base_offset = fu_tell(scn0);
	
	header->length				= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	header->scn0_offset			= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	header->name_offset			= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	header->node_index			= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	header->real_index			= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	
	header->name = brres_read_str_and_back(scn0,
										   base_offset + header->name_offset,
										   &header->name_size);
}

void scn0_read_camera(FU_FILE* scn0, SCN0_CAMERA* camera)
{
	camera->projection				= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	camera->flags1.data				= fu_read_u16(scn0, NULL, FU_LITTLE_ENDIAN);
	camera->flags2.data				= fu_read_u16(scn0, NULL, FU_LITTLE_ENDIAN);
	camera->user_data_offset		= fu_read_u32(scn0, NULL, FU_BIG_ENDIAN);
	camera->pos_x					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->pos_y					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->pos_z					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->aspect					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->near_z					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->far_z					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->rot_x					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->rot_y					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->rot_z					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->aim_x					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->aim_y					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->aim_z					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->twist					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->fov_y					= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
	camera->ortho_height			= fu_read_f32(scn0, NULL, FU_BIG_ENDIAN);
}

void scn0_read_camera_kf_sets(FU_FILE* scn0, SCN0_SECTION* camera)
{
	if(camera->cam.flags1.pos_x_const)
	{
		camera->kf_sets[SCN0_CAMERA_POS_X].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_POS_X].fixed_val = camera->cam.pos_x;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_POS_X]);
	
	if(camera->cam.flags1.pos_y_const)
	{
		camera->kf_sets[SCN0_CAMERA_POS_Y].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_POS_Y].fixed_val = camera->cam.pos_y;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_POS_Y]);
	
	if(camera->cam.flags1.pos_z_const)
	{
		camera->kf_sets[SCN0_CAMERA_POS_Z].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_POS_Z].fixed_val = camera->cam.pos_z;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_POS_Z]);
	
	if(camera->cam.flags1.aspect_const)
	{
		camera->kf_sets[SCN0_CAMERA_ASPECT].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_ASPECT].fixed_val = camera->cam.aspect;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_ASPECT]);
	
	if(camera->cam.flags1.near_const)
	{
		camera->kf_sets[SCN0_CAMERA_NEAR_Z].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_NEAR_Z].fixed_val = camera->cam.near_z;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_NEAR_Z]);
	
	if(camera->cam.flags1.far_const)
	{
		camera->kf_sets[SCN0_CAMERA_FAR_Z].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_FAR_Z].fixed_val = camera->cam.far_z;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_FAR_Z]);

	if(camera->cam.flags1.fov_y_const)
	{
		camera->kf_sets[SCN0_CAMERA_FOV_Y].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_FOV_Y].fixed_val = camera->cam.fov_y;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_FOV_Y]);
	
	if(camera->cam.flags1.ortho_height_const)
	{
		camera->kf_sets[SCN0_CAMERA_ORTHO_HEIGHT].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_ORTHO_HEIGHT].fixed_val = camera->cam.ortho_height;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_ORTHO_HEIGHT]);

	if(camera->cam.flags1.rot_x_const)
	{
		camera->kf_sets[SCN0_CAMERA_ROT_X].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_ROT_X].fixed_val = camera->cam.rot_x;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_ROT_X]);
	
	if(camera->cam.flags1.rot_y_const)
	{
		camera->kf_sets[SCN0_CAMERA_ROT_Y].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_ROT_Y].fixed_val = camera->cam.rot_y;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_ROT_Y]);
	
	if(camera->cam.flags1.rot_z_const)
	{
		camera->kf_sets[SCN0_CAMERA_ROT_Z].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_ROT_Z].fixed_val = camera->cam.rot_z;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_ROT_Z]);
	
	if(camera->cam.flags1.aim_x_const)
	{
		camera->kf_sets[SCN0_CAMERA_AIM_X].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_AIM_X].fixed_val = camera->cam.aim_x;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_AIM_X]);
	
	if(camera->cam.flags1.aim_y_const)
	{
		camera->kf_sets[SCN0_CAMERA_AIM_Y].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_AIM_Y].fixed_val = camera->cam.aim_y;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_AIM_Y]);
	
	if(camera->cam.flags1.aim_z_const)
	{
		camera->kf_sets[SCN0_CAMERA_AIM_Z].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_AIM_Z].fixed_val = camera->cam.aim_z;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_AIM_Z]);
	
	if(camera->cam.flags1.twist_const)
	{
		camera->kf_sets[SCN0_CAMERA_TWIST].fixed = 1;
		camera->kf_sets[SCN0_CAMERA_TWIST].fixed_val = camera->cam.twist;
	} else brres_read_keyframe_set(scn0, &camera->kf_sets[SCN0_CAMERA_TWIST]);
}

SCN0_FILE* scn0_alloc_file()
{
	return (SCN0_FILE*)calloc(1, sizeof(SCN0_FILE));
}

void scn0_print(SCN0_FILE* scn0)
{
	BRRES_SUB_HEADER* subh = &scn0->sub_header;
	SCN0_HEADER* head = &scn0->header;
	
	printf("====BRRES HEADER\n");
	printf("Magic: %.4s\n", subh->magic);
	printf("Length: %u\n", subh->length);
	printf("Version: %u\n", subh->version);
	printf("Outer_brres_offset: %u\n", subh->outer_brres_offset);

	/* Common sections for versions 4 and 5 */
	printf("SCN0_SECTION_SPEC_LIGHT: %x\n", subh->section_offsets[SCN0_SECTION_SPEC_LIGHT]);
	printf("SCN0_SECTION_LIGHT_SET: %x\n", subh->section_offsets[SCN0_SECTION_LIGHT_SET]);
	printf("SCN0_SECTION_AMBIENT: %x\n", subh->section_offsets[SCN0_SECTION_AMBIENT]);
	printf("SCN0_SECTION_LIGHT: %x\n", subh->section_offsets[SCN0_SECTION_LIGHT]);
	printf("SCN0_SECTION_FOG: %x\n", subh->section_offsets[SCN0_SECTION_FOG]);
	printf("SCN0_SECTION_CAMERA: %x\n", subh->section_offsets[SCN0_SECTION_CAMERA]);
	
	if(subh->version == SCN0_V5)
	{
		printf("SCN0_SECTION_7: %x\n", subh->section_offsets[SCN0_SECTION_7]);
	}
	
	printf("Name offset: %u\n", subh->name_offset);
	printf("Name: %s\n", subh->name);
	printf("Name size: %u\n", subh->name_size);
	
	printf("====SCN0 HEADER\n");
	printf("unk_0: %u\n", head->unk_0);
	printf("frame_count: %u\n", head->frame_count);
	printf("spec_light_count: %u\n", head->spec_light_count);
	printf("looping: %u\n", head->looping);
	printf("light_sets: %u\n", head->light_sets);
	printf("ambient_count: %u\n", head->ambient_count);
	printf("light_count: %u\n", head->light_count);
	printf("fog_count: %u\n", head->fog_count);
	printf("cam_count: %u\n", head->cam_count);
	printf("unk_count: %u\n", head->unk_count);
	
	/* TODO: Implement everything else besides camera */
	
	/* Camera */
	if(subh->section_offsets[SCN0_SECTION_CAMERA])
	{
		SCN0_SECTION* section = scn0->sections[SCN0_SECTION_CAMERA];
		
		for(uint16_t i = 0; i != head->cam_count; ++i)
		{
			printf("====SCN0 CAMERA %u\n", i);
			printf("length: %u\n", section[i].header.length);
			printf("scn0_offset: %d\n", section[i].header.scn0_offset);
			printf("name_offset: %u\n", section[i].header.name_offset);
			printf("node_index: %u\n", section[i].header.node_index);
			printf("real_index: %u\n", section[i].header.real_index);
			printf("camera name: %s\n", section[i].header.name);
			
			printf("projection: %u\n", section[i].cam.projection);
			
			printf("unk: %u\n", section[i].cam.flags1.unk);
			printf("pos_x_const: %u\n", section[i].cam.flags1.pos_x_const);
			printf("pos_y_const: %u\n", section[i].cam.flags1.pos_y_const);
			printf("pos_z_const: %u\n", section[i].cam.flags1.pos_z_const);
			printf("aspect_const: %u\n", section[i].cam.flags1.aspect_const);
			printf("near_const: %u\n", section[i].cam.flags1.near_const);
			printf("far_const: %u\n", section[i].cam.flags1.far_const);
			printf("fov_y_const: %u\n", section[i].cam.flags1.fov_y_const);
			printf("ortho_height_const: %u\n", section[i].cam.flags1.ortho_height_const);
			printf("aim_x_const: %u\n", section[i].cam.flags1.aim_x_const);
			printf("aim_y_const: %u\n", section[i].cam.flags1.aim_y_const);
			printf("aim_z_const: %u\n", section[i].cam.flags1.aim_z_const);
			printf("twist_const: %u\n", section[i].cam.flags1.twist_const);
			printf("rot_x_const: %u\n", section[i].cam.flags1.rot_x_const);
			printf("rot_y_const: %u\n", section[i].cam.flags1.rot_y_const);
			printf("rot_z_const: %u\n", section[i].cam.flags1.rot_z_const);
			
			printf("user_data_offset: %d\n", section[i].cam.user_data_offset);
			printf("pos_x: %f\n", section[i].cam.pos_x);
			printf("pos_y: %f\n", section[i].cam.pos_y);
			printf("pos_z: %f\n", section[i].cam.pos_z);
			printf("aspect: %f\n", section[i].cam.aspect);
			printf("near_z: %f\n", section[i].cam.near_z);
			printf("far_z: %f\n", section[i].cam.far_z);
			printf("rot_x: %f\n", section[i].cam.rot_x);
			printf("rot_y: %f\n", section[i].cam.rot_y);
			printf("rot_z: %f\n", section[i].cam.rot_z);
			printf("aim_x: %f\n", section[i].cam.aim_x);
			printf("aim_y: %f\n", section[i].cam.aim_y);
			printf("aim_z: %f\n", section[i].cam.aim_z);
			printf("twist: %f\n", section[i].cam.twist);
			printf("fov_y: %f\n", section[i].cam.fov_y);
			printf("ortho_height: %f\n", section[i].cam.ortho_height);
			
			for(uint32_t j = 0; j != SCN0_CAMERA_KFSETS; ++j)
			{
				printf("[%u] fixed: %u\n", j, section[i].kf_sets[j].fixed);
				
				if(section[i].kf_sets[j].fixed)
				{
					printf("\t fixed_val: %f\n", section[i].kf_sets[j].fixed_val);
				}
				else
				{
					printf("\t keyframe[%u]: %f\n", (uint32_t)section[i].kf_sets[j].keyframes[0].index,
													section[i].kf_sets[j].keyframes[0].value);
				}
			}
		}
	}
}

void scn0_free(SCN0_FILE* scn0)
{
	free(scn0->sub_header.section_offsets);
	free(scn0->sub_header.name);
}