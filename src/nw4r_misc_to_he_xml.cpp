#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>
#include <vector>

#include <pugixml/pugixml.hpp>

#include <kwaslib/kwas_all.h>

void nw4r_srt0_to_uvanim_xml(SRT0_HEADER* header, SRT0_TEX_ANIM_DATA* anim_data, SU_STRING* out);
void nw4r_scn0_to_camanim_xml(SCN0_HEADER* header, SCN0_SECTION* camera_section, SU_STRING* out);
void nw4r_append_keyframe_set(pugi::xml_node* anim, const uint8_t type, BRRES_KEYFRAME_SET* kfs);

/*
	Entry point
*/
int main(int argc, char** argv)
{
	if(argc < 2)
	{
		printf("No arguments\n");
		return 0;
	}
	
	if(pu_is_file(argv[1]))
	{
		PU_PATH* input_file_path = pu_split_path(argv[1], strlen(argv[1]));
		
		/* UV Anims */
		if(strncmp("srt0", input_file_path->ext->ptr, 4) == 0)
		{
			FU_FILE srt0 = {0};
			fu_open_file(argv[1], 1, &srt0);
			
			SRT0_FILE* srt0f = srt0_read_file(&srt0);
			fu_close(&srt0);
			
			srt0_print(srt0f);
			
			/* Fill out UV Anim entries */
			
			for(uint32_t i = 0; i != srt0f->header.anim_data_count; ++i)
			{
				su_remove(input_file_path->ext, 0, -1);
				input_file_path->ext = su_create_string("xml", 3);
				
				/* Append material name to the file name */
				SRT0_HEADER* header = &srt0f->header;
				SRT0_TEX_ANIM_DATA* anim_data = &srt0f->anim_data[i];
				
				su_insert_char(input_file_path->name, -1, "_", 1);
				su_insert_char(input_file_path->name, -1,
                               anim_data->material_name,
							   anim_data->material_name_size);
				
				SU_STRING* out_str = pu_path_to_string(input_file_path);
				
				printf("Save path: %s\n", out_str->ptr);
				
				nw4r_srt0_to_uvanim_xml(header, anim_data, out_str);
				
				pu_free_path(input_file_path);
				su_free(out_str);
			}
			
			srt0_free(srt0f);
			free(srt0f);
		}
		
		/* Cam Anims */
		if(strncmp("scn0", input_file_path->ext->ptr, 4) == 0)
		{
			FU_FILE scn0 = {0};
			fu_open_file(argv[1], 1, &scn0);
			
			SCN0_FILE* scn0f = scn0_read_file(&scn0);
			fu_close(&scn0);
			
			scn0_print(scn0f);

			su_free(input_file_path->ext);
			input_file_path->ext = su_create_string("xml", 3);
			
			/* Append camera name to the file name */
			SCN0_HEADER* header = &scn0f->header;
			SCN0_SECTION* camera_section = scn0f->sections[SCN0_SECTION_CAMERA];
			
			su_insert_char(input_file_path->name, -1, "_", 1);
			su_insert_char(input_file_path->name, -1,
                           camera_section->header.name,
						   camera_section->header.name_size);
			
			SU_STRING* out_str = pu_path_to_string(input_file_path);
			
			printf("Save path: %s\n", out_str->ptr);

			nw4r_scn0_to_camanim_xml(header, camera_section, out_str);
			
			pu_free_path(input_file_path);
			su_free(out_str);

			scn0_free(scn0f);
			free(scn0f);
		}
        
        pu_free_path(input_file_path);
	}
}

void nw4r_srt0_to_uvanim_xml(SRT0_HEADER* header, SRT0_TEX_ANIM_DATA* anim_data, SU_STRING* out)
{
	pugi::xml_document doc;

	pugi::xml_node uv_anim = doc.append_child("UVAnimation");
	uv_anim.append_attribute("root_node_type").set_value(3);
	uv_anim.append_attribute("material_name").set_value((const pugi::char_t*)anim_data->material_name);
	uv_anim.append_attribute("texture_name").set_value((const pugi::char_t*)"none");

	pugi::xml_node anim = uv_anim.append_child("Animation");

	anim.append_attribute("name").set_value((const pugi::char_t*)anim_data->material_name);
	anim.append_attribute("frame_rate").set_value(60); /* Actually not in the SRT0 */
	anim.append_attribute("start_frame").set_value(0);
	anim.append_attribute("end_frame").set_value((const uint16_t)header->frame_count-1);

	for(uint32_t i = 0; i != anim_data->m_bits; ++i)
	{		
		SRT0_TEX_ANIM_DESC* entry = &anim_data->entries[i];
		
		for(uint32_t k = 0; k != 2; ++k)
		{
			SRT0_TEX_ANIM_FRAME_DATA* frame_data = &entry->frame_data[3+k];
			
			pugi::xml_node kfs = anim.append_child("KeyframeSet");
			kfs.append_attribute("type").set_value(k);
			kfs.append_attribute("flag2").set_value(1);
			kfs.append_attribute("interpolation").set_value(0);
			kfs.append_attribute("flag4").set_value(0);
			
			if(frame_data->frame_count == 0)
			{
				pugi::xml_node kf = kfs.append_child("Keyframe");
				kf.append_attribute("index").set_value(0);
				kf.append_attribute("value").set_value(0);
			}
			else
			{
				for(uint32_t j = 0; j != frame_data->frame_count; ++j)
				{
					pugi::xml_node kf = kfs.append_child("Keyframe");
					kf.append_attribute("index").set_value(frame_data->keyframes[j].index);
					if(k == 0) kf.append_attribute("value").set_value(-frame_data->keyframes[j].value);
					else kf.append_attribute("value").set_value(frame_data->keyframes[j].value);
				}
			}
		}

	}

	doc.save_file(out->ptr);
}

void nw4r_scn0_to_camanim_xml(SCN0_HEADER* header, SCN0_SECTION* camera_section, SU_STRING* out)
{
	pugi::xml_document doc;

	pugi::xml_node cam_anim = doc.append_child("CAMAnimation");
	cam_anim.append_attribute("root_node_type").set_value(2);
	
	pugi::xml_node anim = cam_anim.append_child("Animation");
	anim.append_attribute("name").set_value((const pugi::char_t*)camera_section->header.name);
	anim.append_attribute("rot_or_aim").set_value(1);
	anim.append_attribute("flag2").set_value(0);
	anim.append_attribute("flag3").set_value(0);
	anim.append_attribute("flag4").set_value(0);
	anim.append_attribute("frame_rate").set_value(60); /* Not in SCN0 */
	anim.append_attribute("start_frame").set_value(0);
	anim.append_attribute("end_frame").set_value(header->frame_count);
	
	/* Prepare keyframe values */
	
	/*
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_POS_X].frame_count; ++i)
		camera_section->kf_sets[SCN0_CAMERA_POS_X].keyframes[i].value /= 10.f;
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_POS_Y].frame_count; ++i)
		camera_section->kf_sets[SCN0_CAMERA_POS_Y].keyframes[i].value /= 10.f;
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_POS_Z].frame_count; ++i)
		camera_section->kf_sets[SCN0_CAMERA_POS_Z].keyframes[i].value /= 10.f;
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_AIM_X].frame_count; ++i)
		camera_section->kf_sets[SCN0_CAMERA_AIM_X].keyframes[i].value /= 10.f;
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_AIM_Y].frame_count; ++i)
		camera_section->kf_sets[SCN0_CAMERA_AIM_Y].keyframes[i].value /= 10.f;
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_AIM_Z].frame_count; ++i)
		camera_section->kf_sets[SCN0_CAMERA_AIM_Z].keyframes[i].value /= 10.f;
	*/
	
	/* FOV is a special case */

	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_FOV_Y].frame_count; ++i)
	{
		BRRES_KEYFRAME* cur_kf = &camera_section->kf_sets[SCN0_CAMERA_FOV_Y].keyframes[i];
		const double mypi = 3.141592653589;
		//const double aspect = camera_section->cam.aspect;
		const double angle_x_rad = (cur_kf->value*mypi)/180.f;
		//const double angle_y_rad = 2*atan(tan(angle_x_rad/2.f)*aspect);
		cur_kf->value = angle_x_rad;
	}
	
	/* Twist too I guess */
	for(uint32_t i = 0; i != camera_section->kf_sets[SCN0_CAMERA_TWIST].frame_count; ++i)
	{
		BRRES_KEYFRAME* cur_kf = &camera_section->kf_sets[SCN0_CAMERA_TWIST].keyframes[i];
		const double mypi = 3.141592653589;
		//const double twist = camera_section->cam.twist;
		const double twist_rad = (cur_kf->value*mypi)/180.f;
		cur_kf->value = twist_rad;
	}
	
	if(camera_section->kf_sets[SCN0_CAMERA_POS_X].fixed == 0)
		camera_section->cam.pos_x = camera_section->kf_sets[SCN0_CAMERA_POS_X].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_POS_Y].fixed == 0)
		camera_section->cam.pos_z = camera_section->kf_sets[SCN0_CAMERA_POS_Y].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_POS_Z].fixed == 0)
		camera_section->cam.pos_y = camera_section->kf_sets[SCN0_CAMERA_POS_Z].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_AIM_X].fixed == 0)
		camera_section->cam.aim_x = camera_section->kf_sets[SCN0_CAMERA_AIM_X].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_AIM_Y].fixed == 0)
		camera_section->cam.aim_z = camera_section->kf_sets[SCN0_CAMERA_AIM_Y].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_AIM_Z].fixed == 0)
		camera_section->cam.aim_y = camera_section->kf_sets[SCN0_CAMERA_AIM_Z].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_TWIST].fixed == 0)
		camera_section->cam.twist = camera_section->kf_sets[SCN0_CAMERA_TWIST].keyframes[0].value;
	if(camera_section->kf_sets[SCN0_CAMERA_FOV_Y].fixed == 0)
		camera_section->cam.fov_y = camera_section->kf_sets[SCN0_CAMERA_FOV_Y].keyframes[0].value;
	
	anim.append_attribute("cam_pos_x").set_value(camera_section->cam.pos_x);
	anim.append_attribute("cam_pos_z").set_value(camera_section->cam.pos_z);
	anim.append_attribute("cam_pos_y").set_value(camera_section->cam.pos_y);
	anim.append_attribute("cam_rot_x").set_value(camera_section->cam.rot_x);
	anim.append_attribute("cam_rot_z").set_value(camera_section->cam.rot_z);
	anim.append_attribute("cam_rot_y").set_value(camera_section->cam.rot_y);
	anim.append_attribute("aim_pos_x").set_value(camera_section->cam.aim_x);
	anim.append_attribute("aim_pos_z").set_value(camera_section->cam.aim_z);
	anim.append_attribute("aim_pos_y").set_value(camera_section->cam.aim_y);
	anim.append_attribute("twist").set_value(camera_section->cam.twist);
	anim.append_attribute("z_near").set_value(camera_section->cam.near_z);
	anim.append_attribute("z_far").set_value(camera_section->cam.far_z);
	anim.append_attribute("fov").set_value(camera_section->cam.fov_y);
	anim.append_attribute("aspect_ratio").set_value(camera_section->cam.aspect);
	
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_CAM_X, &camera_section->kf_sets[SCN0_CAMERA_POS_X]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_CAM_Z, &camera_section->kf_sets[SCN0_CAMERA_POS_Y]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_CAM_Y, &camera_section->kf_sets[SCN0_CAMERA_POS_Z]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_Z_NEAR, &camera_section->kf_sets[SCN0_CAMERA_NEAR_Z]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_Z_FAR, &camera_section->kf_sets[SCN0_CAMERA_FAR_Z]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_CAM_ROT_X, &camera_section->kf_sets[SCN0_CAMERA_ROT_X]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_CAM_ROT_Z, &camera_section->kf_sets[SCN0_CAMERA_ROT_Y]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_CAM_ROT_Y, &camera_section->kf_sets[SCN0_CAMERA_ROT_Z]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_AIM_X, &camera_section->kf_sets[SCN0_CAMERA_AIM_X]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_AIM_Z, &camera_section->kf_sets[SCN0_CAMERA_AIM_Y]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_AIM_Y, &camera_section->kf_sets[SCN0_CAMERA_AIM_Z]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_TWIST, &camera_section->kf_sets[SCN0_CAMERA_TWIST]);
	nw4r_append_keyframe_set(&anim, CAM_ANIM_TYPE_FOV, &camera_section->kf_sets[SCN0_CAMERA_FOV_Y]);
	
	doc.save_file(out->ptr);
}

void nw4r_append_keyframe_set(pugi::xml_node* anim, const uint8_t type, BRRES_KEYFRAME_SET* kfs)
{
	if(kfs->fixed) return;
	
	pugi::xml_node kfs_xml = anim->append_child("KeyframeSet");
	kfs_xml.append_attribute("type").set_value(type);
	kfs_xml.append_attribute("flag2").set_value(0);
	kfs_xml.append_attribute("interpolation").set_value(0);
	kfs_xml.append_attribute("flag4").set_value(0);
	
	for(uint32_t i = 0; i != kfs->frame_count; ++i)
	{
		pugi::xml_node kf_xml = kfs_xml.append_child("Keyframe");
		kf_xml.append_attribute("index").set_value((uint32_t)kfs->keyframes[i].index);
		kf_xml.append_attribute("value").set_value(kfs->keyframes[i].value);
	}
}