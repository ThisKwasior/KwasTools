#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include <pugixml/pugixml.hpp>

#include <kwaslib/kwas_all.h>
#include <kwasutils_pugi_helper.hpp>

/*
	Common
*/
#define ANIM_TOOL_TYPE_XML	1
#define ANIM_TOOL_TYPE_UV	2
#define ANIM_TOOL_TYPE_CAM	3

/* 
	struct containing an union with pointers to all
	anim formats and a type of stored pointer
*/
typedef struct
{
	union
	{
		UV_ANIM_FILE* uv;
		CAM_ANIM_FILE* cam;
	} ptrs;

	uint8_t type;
} ANIM_TOOL_HE_TYPES;

void anim_tool_print_usage(char* exe_path);
void anim_tool_anim_to_file(ANIM_TOOL_HE_TYPES* anim_type, PU_PATH* xml_path);

/* 
	Unpacker
*/
void anim_tool_anim_to_xml(ANIM_TOOL_HE_TYPES* anim_type, SU_STRING* out);
void anim_tool_uv_to_xml(UV_ANIM_FILE* uv, SU_STRING* out);
void anim_tool_cam_to_xml(CAM_ANIM_FILE* cam, SU_STRING* out);

void anim_tool_append_kfset_xml(pugi::xml_node& anim, MIRAGE_KEYFRAME_SET* kfs, MIRAGE_KEYFRAME* keyframes);

/* 
	Packer
*/
uint8_t anim_tool_get_xml_type(pugi::xml_document& doc);
void anim_tool_xml_to_anim(ANIM_TOOL_HE_TYPES* anim_type, PU_PATH* xml_path);
UV_ANIM_FILE* anim_tool_xml_to_uv(pugi::xml_document& xml, std::vector<uint32_t>& entry_offsets_vec,
								  std::vector<std::string>& string_table_vec, std::vector<uint32_t>& string_table_offsets_vec,
								  std::vector<uint32_t>& keyframe_sets_count_vec, std::vector<MIRAGE_KEYFRAME>& keyframes_vec);

CAM_ANIM_FILE* anim_tool_xml_to_cam(pugi::xml_document& xml, std::vector<uint32_t>& entry_offsets_vec,
								   std::vector<std::string>& string_table_vec, std::vector<uint32_t>& string_table_offsets_vec,
								   std::vector<uint32_t>& keyframe_sets_count_vec, std::vector<MIRAGE_KEYFRAME>& keyframes_vec);


/* Separated because all anim types share some structures */
void anim_tool_xml_load_keyframe_set(pugi::xml_node& kfs_node, MIRAGE_KEYFRAME_SET* set, pugi::xml_node& anim_node, std::vector<MIRAGE_KEYFRAME>& keyframes_vec);
void anim_tool_xml_calc_sizes(MIRAGE_HEADER* header, MIRAGE_INFO* info, std::vector<MIRAGE_KEYFRAME>& keyframes_vec);
MIRAGE_KEYFRAME* anim_tool_xml_load_keyframes(MIRAGE_INFO* info, std::vector<MIRAGE_KEYFRAME>& keyframes_vec);
char* anim_tool_xml_load_string_table(MIRAGE_INFO* info, std::vector<std::string>& string_table_vec, std::vector<uint32_t>& string_table_offsets_vec);
void anim_tool_xml_calc_node_offsets(MIRAGE_INFO* info);
void anim_tool_xml_calc_footer(MIRAGE_FOOTER* footer, const uint32_t anim_count, const uint32_t metadata_offsets_offset);
void anim_tool_xml_calc_header(MIRAGE_HEADER* header, MIRAGE_INFO* info, MIRAGE_FOOTER* footer);

/* 
	Entry
*/
int main(int argc, char** argv)
{
	if(argc == 1)
	{
		anim_tool_print_usage(&argv[0][0]);
		return 0;
	}

	/* It's a file so let's process it */
	if(pu_is_file(argv[1]))
	{
		PU_PATH* input_file_path = pu_split_path(argv[1], strlen(argv[1]));

		ANIM_TOOL_HE_TYPES anim_type = {0};

		/* It's an XML */
		if(strncmp(input_file_path->ext->ptr, "xml", 3) == 0)
		{
			anim_tool_xml_to_anim(&anim_type, input_file_path);
			anim_tool_anim_to_file(&anim_type, input_file_path);
		}
		else /* Check if the file is a HE anim format */
		{
			FU_FILE anim = {0};
			fu_open_file(argv[1], 1, &anim);

			if(strncmp(input_file_path->ext->ptr, "uv-anim", 7) == 0)
			{
				anim_type.ptrs.uv = uv_anim_load_file(&anim);
				anim_type.type = ANIM_TOOL_TYPE_UV;
				uv_anim_print_uv(anim_type.ptrs.uv);
			}
			else if(strncmp(input_file_path->ext->ptr, "cam-anim", 8) == 0)
			{
				anim_type.ptrs.cam = cam_anim_load_file(&anim);
				anim_type.type = ANIM_TOOL_TYPE_CAM;
				cam_anim_print_cam(anim_type.ptrs.cam);
			}

			/* Close the file */
			fu_close(&anim);

			/* Change the extension to xml */
            su_remove(input_file_path->ext, 0, -1);
            SU_STRING* xml_output = pu_path_to_string(input_file_path);
            su_insert_char(xml_output, -1, ".xml", 4);

			/* Save the anim to xml by type */
			anim_tool_anim_to_xml(&anim_type, xml_output);
            
            su_free(xml_output);
		}
        
        pu_free_path(input_file_path);
	}

	return 0;
}

/*
	Common
*/
void anim_tool_print_usage(char* exe_path)
{
	printf("Converts Hedgehog Engine *-anim to XML and vice versa.\n");
	printf("Currently supports:\n");
	printf("\t- uv-anim\n");
	printf("\t- cam-anim\n");
	printf("Usage:\n");
	printf("\tTo unpack: %s <file.*-anim>\n", exe_path);
	printf("\tTo pack: %s <file.xml>\n", exe_path);
}

void anim_tool_anim_to_file(ANIM_TOOL_HE_TYPES* anim_type, PU_PATH* xml_path)
{
	/* Change the extension to anim */
	xml_path->ext = su_free(xml_path->ext);

	FU_FILE* prepared = NULL;
			
	switch(anim_type->type)
	{
		case ANIM_TOOL_TYPE_UV:
			xml_path->ext = su_create_string("uv-anim", 7);
			prepared = uv_anim_save_to_fu_file(anim_type->ptrs.uv);
			break;
		case ANIM_TOOL_TYPE_CAM:
			xml_path->ext = su_create_string("cam-anim", 8);
			prepared = cam_anim_save_to_fu_file(anim_type->ptrs.cam);
			break;
	}

	fu_to_file_pu(xml_path, prepared, 1);
	fu_close(prepared);
    free(prepared);
}

/* 
	Unpacker
*/
void anim_tool_anim_to_xml(ANIM_TOOL_HE_TYPES* anim_types, SU_STRING* out)
{
	switch(anim_types->type)
	{
		case ANIM_TOOL_TYPE_UV:
			anim_tool_uv_to_xml(anim_types->ptrs.uv, out);
			break;
		case ANIM_TOOL_TYPE_CAM:
			anim_tool_cam_to_xml(anim_types->ptrs.cam, out);
			break;
	}
}

void anim_tool_uv_to_xml(UV_ANIM_FILE* uv, SU_STRING* out)
{
	pugi::xml_document doc;

	pugi::xml_node uv_anim = doc.append_child("UVAnimation");
	uv_anim.append_attribute("root_node_type").set_value(uv->header.root_node_type);
	uv_anim.append_attribute("material_name").set_value((const pugi::char_t*)uv->metadata.material_name);
	uv_anim.append_attribute("texture_name").set_value((const pugi::char_t*)uv->metadata.texture_name);

	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		pugi::xml_node anim = uv_anim.append_child("Animation");

		UV_ANIM_ENTRY* entry = &uv->entries[i];
		
		anim.append_attribute("name").set_value((const pugi::char_t*)entry->name);
		anim.append_attribute("frame_rate").set_value((const uint32_t)entry->frame_rate);
		anim.append_attribute("start_frame").set_value((const uint32_t)entry->start_frame);
		anim.append_attribute("end_frame").set_value((const uint32_t)entry->end_frame);

		/* Keyframe set append */
		for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
		{
			MIRAGE_KEYFRAME_SET* kfs = &entry->keyframe_sets[j];
			anim_tool_append_kfset_xml(anim, kfs, uv->keyframes);
		}
	}

	doc.save_file(out->ptr);
}

void anim_tool_cam_to_xml(CAM_ANIM_FILE* cam, SU_STRING* out)
{
	pugi::xml_document doc;

	pugi::xml_node cam_anim = doc.append_child("CAMAnimation");
	cam_anim.append_attribute("root_node_type").set_value(cam->header.root_node_type);

	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		pugi::xml_node anim = cam_anim.append_child("Animation");

		CAM_ANIM_ENTRY* entry = &cam->entries[i];
		
		anim.append_attribute("name").set_value((const pugi::char_t*)entry->name);
		anim.append_attribute("rot_or_aim").set_value(entry->rot_or_aim);
		anim.append_attribute("flag2").set_value(entry->flag2);
		anim.append_attribute("flag3").set_value(entry->flag3);
		anim.append_attribute("flag4").set_value(entry->flag4);
		anim.append_attribute("frame_rate").set_value((const uint32_t)entry->frame_rate);
		anim.append_attribute("start_frame").set_value((const uint32_t)entry->start_frame);
		anim.append_attribute("end_frame").set_value((const uint32_t)entry->end_frame);

		anim.append_attribute("cam_pos_x").set_value(entry->cam_position.x);
		anim.append_attribute("cam_pos_z").set_value(entry->cam_position.z);
		anim.append_attribute("cam_pos_y").set_value(entry->cam_position.y);
		anim.append_attribute("cam_rot_x").set_value(entry->cam_rotation.x);
		anim.append_attribute("cam_rot_z").set_value(entry->cam_rotation.z);
		anim.append_attribute("cam_rot_y").set_value(entry->cam_rotation.y);
		anim.append_attribute("aim_pos_x").set_value(entry->aim_position.x);
		anim.append_attribute("aim_pos_z").set_value(entry->aim_position.z);
		anim.append_attribute("aim_pos_y").set_value(entry->aim_position.y);

		anim.append_attribute("twist").set_value(entry->twist);
		anim.append_attribute("z_near").set_value(entry->z_near);
		anim.append_attribute("z_far").set_value(entry->z_far);
		anim.append_attribute("fov").set_value(entry->fov);
		anim.append_attribute("aspect_ratio").set_value(entry->aspect_ratio);

		/* Keyframe set append */
		for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
		{
			MIRAGE_KEYFRAME_SET* kfs = &entry->keyframe_sets[j];
			anim_tool_append_kfset_xml(anim, kfs, cam->keyframes);
		}
	}

	doc.save_file(out->ptr);
}

void anim_tool_append_kfset_xml(pugi::xml_node& anim, MIRAGE_KEYFRAME_SET* kfs, MIRAGE_KEYFRAME* keyframes)
{
	pugi::xml_node set = anim.append_child("KeyframeSet");

	set.append_attribute("type").set_value(kfs->type);
	set.append_attribute("flag2").set_value(kfs->flag2);
	set.append_attribute("interpolation").set_value(kfs->interpolation);
	set.append_attribute("flag4").set_value(kfs->flag4);

	for(uint32_t k = 0; k != kfs->length; ++k)
	{
		pugi::xml_node keyframe = set.append_child("Keyframe");
		MIRAGE_KEYFRAME* kf = &keyframes[kfs->start + k];
		keyframe.append_attribute("index").set_value(kf->index);
		keyframe.append_attribute("value").set_value(kf->value);
	}
}

/* 
	Packer
*/
uint8_t anim_tool_get_xml_type(pugi::xml_document& doc)
{
	if(kwasuitls_does_node_exists(doc, "UVAnimation")) return ANIM_TOOL_TYPE_UV;
	if(kwasuitls_does_node_exists(doc, "CAMAnimation")) return ANIM_TOOL_TYPE_CAM;
	return 0;
}

void anim_tool_xml_to_anim(ANIM_TOOL_HE_TYPES* anim_type, PU_PATH* xml_path)
{
	SU_STRING* xml_path_str = pu_path_to_string(xml_path);

	pugi::xml_document xml;
	xml.load_file(xml_path_str->ptr);
    su_free(xml_path_str);

	anim_type->type = anim_tool_get_xml_type(xml);

	std::vector<uint32_t> entry_offsets_vec;
	std::vector<std::string> string_table_vec;
	std::vector<uint32_t> string_table_offsets_vec;
	std::vector<uint32_t> keyframe_sets_count_vec;
	std::vector<MIRAGE_KEYFRAME> keyframes_vec;

	switch(anim_type->type)
	{
		case ANIM_TOOL_TYPE_UV:
			anim_type->ptrs.uv = anim_tool_xml_to_uv(xml, entry_offsets_vec, string_table_vec,
												    string_table_offsets_vec, keyframe_sets_count_vec, keyframes_vec);
			break;
		case ANIM_TOOL_TYPE_CAM:
			anim_type->ptrs.cam = anim_tool_xml_to_cam(xml, entry_offsets_vec, string_table_vec,
													   string_table_offsets_vec, keyframe_sets_count_vec, keyframes_vec);
			break;
	}
}

UV_ANIM_FILE* anim_tool_xml_to_uv(pugi::xml_document& xml, std::vector<uint32_t>& entry_offsets_vec,
								  std::vector<std::string>& string_table_vec, std::vector<uint32_t>& string_table_offsets_vec,
								  std::vector<uint32_t>& keyframe_sets_count_vec, std::vector<MIRAGE_KEYFRAME>& keyframes_vec)
{
	UV_ANIM_FILE* uv = (UV_ANIM_FILE*)calloc(1, sizeof(UV_ANIM_FILE));

	/*
		Get all data from XML, specific for uv-anim
	*/
	pugi::xml_node uvanim_node = xml.child("UVAnimation");
	uv->header.root_node_type = uvanim_node.attribute("root_node_type").as_uint();
	uv->header.root_node_offset = MIRAGE_HEADER_SIZE;
	string_table_vec.push_back(std::string(uvanim_node.attribute("material_name").as_string()));
	string_table_vec.push_back(std::string(uvanim_node.attribute("texture_name").as_string()));
	uv->metadata.material_name_offset = 0;
	uv->metadata.texture_name_offset = string_table_vec[0].size() + 1;
	uv->info.string_table_size = uv->metadata.texture_name_offset + (string_table_vec[1].size() + 1);
	string_table_offsets_vec.push_back(0);
	string_table_offsets_vec.push_back(string_table_vec[0].size() + 1);
	printf("%u %u %s|%u %s|%u \n", uv->header.root_node_type, uv->info.string_table_size,
								   string_table_vec[0].c_str(), string_table_offsets_vec[0],
								   string_table_vec[1].c_str(), string_table_offsets_vec[1]);

	uv->metadata.anim_count = kwasutils_get_xml_child_count(&uvanim_node, "Animation");
	printf("Animations: %u\n", uv->metadata.anim_count);
	uv->entries = (UV_ANIM_ENTRY*)calloc(uv->metadata.anim_count, sizeof(UV_ANIM_ENTRY));
	/* all non-pointer fields in metadata struct and anim count times 4 bytes */
	uv->info.metadata_size = UV_ANIM_METADATA_FIXED_SIZE + (uv->metadata.anim_count*4);

	uint32_t anim_it = 0;
	for(pugi::xml_node anim_node : uvanim_node.children())
	{
		UV_ANIM_ENTRY* cur_entry = &uv->entries[anim_it];

		/* Info size is not included in metadata_size calculation, so we add it here for later */
		entry_offsets_vec.push_back(uv->info.metadata_size + MIRAGE_INFO_SIZE);

		string_table_offsets_vec.push_back(uv->info.string_table_size);
		cur_entry->name_offset = uv->info.string_table_size;
		string_table_vec.push_back(std::string(anim_node.attribute("name").as_string()));
		uv->info.string_table_size += string_table_vec[string_table_vec.size()-1].size() + 1;
		printf("Animation name: %s|%u | %u\n", string_table_vec[string_table_vec.size()-1].c_str(),
											   string_table_offsets_vec[string_table_offsets_vec.size()-1],
											   uv->info.string_table_size);

		cur_entry->frame_rate = anim_node.attribute("frame_rate").as_float();
		cur_entry->start_frame = anim_node.attribute("start_frame").as_float();
		cur_entry->end_frame = anim_node.attribute("end_frame").as_float();
		cur_entry->keyframe_set_count = kwasutils_get_xml_child_count(&anim_node, "KeyframeSet");
		cur_entry->keyframe_sets = (MIRAGE_KEYFRAME_SET*)calloc(cur_entry->keyframe_set_count, sizeof(MIRAGE_KEYFRAME_SET));

		/* all non-pointer fields in entry struct and keyframe_set_count times MIRAGE_KEYFRAME_SET_SIZE */
		uv->info.metadata_size += UV_ANIM_ENTRY_FIXED_SIZE + (cur_entry->keyframe_set_count*MIRAGE_KEYFRAME_SET_SIZE);
		
		uint32_t kfs_it = 0;
		for(pugi::xml_node kfs_node : anim_node.children())
		{
			MIRAGE_KEYFRAME_SET* cur_set = &cur_entry->keyframe_sets[kfs_it];
			anim_tool_xml_load_keyframe_set(kfs_node, cur_set, anim_node, keyframes_vec);
			kfs_it += 1;
		}
		anim_it += 1;
	}

	/* Calculate sizes */
	anim_tool_xml_calc_sizes(&uv->header, &uv->info, keyframes_vec);

	/* Allocate and populate leftover pointers */
	uv->keyframes = anim_tool_xml_load_keyframes(&uv->info, keyframes_vec);
	uv->string_table = anim_tool_xml_load_string_table(&uv->info, string_table_vec, string_table_offsets_vec);

	/* UV-Specific fields*/
	uv->metadata.material_name = (const char*)&uv->string_table[uv->metadata.material_name_offset];
	uv->metadata.texture_name = (const char*)&uv->string_table[uv->metadata.texture_name_offset];
	uv->metadata.anim_offsets = (uint32_t*)calloc(uv->metadata.anim_count, 4);
	
	for(uint32_t i = 0; i != uv->metadata.anim_count; ++i)
	{
		/* Assigning a pointer to helper pointer for entry name */
		uv->entries[i].name = (const char*)&uv->string_table[uv->entries[i].name_offset];
		/* Assigning offsets for entries in metadata */
		uv->metadata.anim_offsets[i] = entry_offsets_vec[i];
		printf("Offset[%u] %u\n", i, uv->metadata.anim_offsets[i]);
	}

	/* Figure out node offsets */
	anim_tool_xml_calc_node_offsets(&uv->info);

	/* Footer */
	anim_tool_xml_calc_footer(&uv->footer, uv->metadata.anim_count, UV_ANIM_METADATA_OFFSETS_OFFSET);

	/* Figure out the header */
	anim_tool_xml_calc_header(&uv->header, &uv->info, &uv->footer);

	return uv;
}

CAM_ANIM_FILE* anim_tool_xml_to_cam(pugi::xml_document& xml, std::vector<uint32_t>& entry_offsets_vec,
								   std::vector<std::string>& string_table_vec, std::vector<uint32_t>& string_table_offsets_vec,
								   std::vector<uint32_t>& keyframe_sets_count_vec, std::vector<MIRAGE_KEYFRAME>& keyframes_vec)
{
	CAM_ANIM_FILE* cam = (CAM_ANIM_FILE*)calloc(1, sizeof(CAM_ANIM_FILE));

	/*
		Get all data from XML, specific for cam-anim
	*/
	pugi::xml_node camanim_node = xml.child("CAMAnimation");
	cam->header.root_node_type = camanim_node.attribute("root_node_type").as_uint();
	cam->header.root_node_offset = MIRAGE_HEADER_SIZE;

	cam->metadata.anim_count = kwasutils_get_xml_child_count(&camanim_node, "Animation");
	printf("Animations: %u\n", cam->metadata.anim_count);
	cam->entries = (CAM_ANIM_ENTRY*)calloc(cam->metadata.anim_count, sizeof(CAM_ANIM_ENTRY));
	/* all non-pointer fields in metadata struct and anim count times 4 bytes */
	cam->info.metadata_size = CAM_ANIM_METADATA_FIXED_SIZE + (cam->metadata.anim_count*4);

	uint32_t anim_it = 0;
	for(pugi::xml_node anim_node : camanim_node.children())
	{
		CAM_ANIM_ENTRY* cur_entry = &cam->entries[anim_it];

		/* Info size is not included in metadata_size calculation, so we add it here for later */
		entry_offsets_vec.push_back(cam->info.metadata_size + MIRAGE_INFO_SIZE);

		string_table_offsets_vec.push_back(cam->info.string_table_size);
		cur_entry->name_offset = cam->info.string_table_size;
		string_table_vec.push_back(std::string(anim_node.attribute("name").as_string()));
		cam->info.string_table_size += string_table_vec[string_table_vec.size()-1].size() + 1;
		printf("Animation name: %s|%u | %u\n", string_table_vec[string_table_vec.size()-1].c_str(),
											   string_table_offsets_vec[string_table_offsets_vec.size()-1],
											   cam->info.string_table_size);

		cur_entry->rot_or_aim = anim_node.attribute("rot_or_aim").as_uint();
		cur_entry->flag2 = anim_node.attribute("flag2").as_uint();
		cur_entry->flag3 = anim_node.attribute("flag3").as_uint();
		cur_entry->flag4 = anim_node.attribute("flag4").as_uint();
		cur_entry->frame_rate = anim_node.attribute("frame_rate").as_float();
		cur_entry->start_frame = anim_node.attribute("start_frame").as_float();
		cur_entry->end_frame = anim_node.attribute("end_frame").as_float();

		cur_entry->cam_position.x = anim_node.attribute("cam_pos_x").as_float();
		cur_entry->cam_position.z = anim_node.attribute("cam_pos_z").as_float();
		cur_entry->cam_position.y = anim_node.attribute("cam_pos_y").as_float();
		cur_entry->cam_rotation.x = anim_node.attribute("cam_rot_x").as_float();
		cur_entry->cam_rotation.z = anim_node.attribute("cam_rot_z").as_float();
		cur_entry->cam_rotation.y = anim_node.attribute("cam_rot_y").as_float();
		cur_entry->aim_position.x = anim_node.attribute("aim_pos_x").as_float();
		cur_entry->aim_position.z = anim_node.attribute("aim_pos_z").as_float();
		cur_entry->aim_position.y = anim_node.attribute("aim_pos_y").as_float();

		cur_entry->twist = anim_node.attribute("twist").as_float();
		cur_entry->z_near = anim_node.attribute("z_near").as_float();
		cur_entry->z_far = anim_node.attribute("z_far").as_float();
		cur_entry->fov = anim_node.attribute("fov").as_float();
		cur_entry->aspect_ratio = anim_node.attribute("aspect_ratio").as_float();

		cur_entry->keyframe_set_count = kwasutils_get_xml_child_count(&anim_node, "KeyframeSet");
		cur_entry->keyframe_sets = (MIRAGE_KEYFRAME_SET*)calloc(cur_entry->keyframe_set_count, sizeof(MIRAGE_KEYFRAME_SET));

		/* all non-pointer fields in entry struct and keyframe_set_count times MIRAGE_KEYFRAME_SET_SIZE */
		cam->info.metadata_size += CAM_ANIM_ENTRY_FIXED_SIZE + (cur_entry->keyframe_set_count*MIRAGE_KEYFRAME_SET_SIZE);
		
		uint32_t kfs_it = 0;
		for(pugi::xml_node kfs_node : anim_node.children())
		{
			MIRAGE_KEYFRAME_SET* cur_set = &cur_entry->keyframe_sets[kfs_it];
			anim_tool_xml_load_keyframe_set(kfs_node, cur_set, anim_node, keyframes_vec);
			kfs_it += 1;
		}

		anim_it += 1;
	}

	/* Calculate sizes */
	anim_tool_xml_calc_sizes(&cam->header, &cam->info, keyframes_vec);

	/* Allocate and populate leftover pointers */
	cam->keyframes = anim_tool_xml_load_keyframes(&cam->info, keyframes_vec);
	cam->string_table = anim_tool_xml_load_string_table(&cam->info, string_table_vec, string_table_offsets_vec);

	/* -Specific fields */
	cam->metadata.anim_offsets = (uint32_t*)calloc(cam->metadata.anim_count, 4);
	
	for(uint32_t i = 0; i != cam->metadata.anim_count; ++i)
	{
		/* Assigning a pointer to helper pointer for entry name */
		cam->entries[i].name = (const char*)&cam->string_table[cam->entries[i].name_offset];
		/* Assigning offsets for entries in metadata */
		cam->metadata.anim_offsets[i] = entry_offsets_vec[i];
		printf("Offset[%u] %u\n", i, cam->metadata.anim_offsets[i]);
	}

	/* Figure out node offsets */
	anim_tool_xml_calc_node_offsets(&cam->info);

	/* Footer */
	anim_tool_xml_calc_footer(&cam->footer, cam->metadata.anim_count, CAM_ANIM_METADATA_OFFSETS_OFFSET);

	/* Figure out the header */
	anim_tool_xml_calc_header(&cam->header, &cam->info, &cam->footer);

	return cam;
}

/* Separated because all anim types share some structures */
void anim_tool_xml_load_keyframe_set(pugi::xml_node& kfs_node, MIRAGE_KEYFRAME_SET* set, pugi::xml_node& anim_node, std::vector<MIRAGE_KEYFRAME>& keyframes_vec)
{
	set->type = kfs_node.attribute("type").as_uint();
	set->flag2 = kfs_node.attribute("flag2").as_uint();
	set->interpolation = kfs_node.attribute("interpolation").as_uint();
	set->flag4 = kfs_node.attribute("flag4").as_uint();
	set->length = kwasutils_get_xml_child_count(&kfs_node, "Keyframe");
	set->start = keyframes_vec.size();

	for(pugi::xml_node kf_node : kfs_node.children())
	{
		MIRAGE_KEYFRAME temp_kf = {.index = kf_node.attribute("index").as_float(),
									.value = kf_node.attribute("value").as_float()};
		keyframes_vec.push_back(temp_kf);
	}

	printf("KFS %u %u %u %u %u %u\n", set->type, set->flag2, set->interpolation,
										set->flag4, set->length, set->start);

}

void anim_tool_xml_calc_sizes(MIRAGE_HEADER* header, MIRAGE_INFO* info, std::vector<MIRAGE_KEYFRAME>& keyframes_vec)
{
	info->keyframes_size = keyframes_vec.size() * MIRAGE_KEYFRAME_SIZE;
	info->string_table_size += bound_calc_leftover(4, info->string_table_size);
	header->root_node_size = MIRAGE_INFO_SIZE + info->metadata_size +
							 info->keyframes_size + info->string_table_size;

	printf("Final metadata_size: %u\n", info->metadata_size);
	printf("Final keyframes_size: %u\n", info->keyframes_size);
	printf("Final string_table_size: %u\n", info->string_table_size);
	printf("Final root_node_size: %u\n", header->root_node_size);
}

MIRAGE_KEYFRAME* anim_tool_xml_load_keyframes(MIRAGE_INFO* info, std::vector<MIRAGE_KEYFRAME>& keyframes_vec)
{
	MIRAGE_KEYFRAME* keyframes = (MIRAGE_KEYFRAME*)calloc(info->keyframes_size, 1);
	for(uint32_t i = 0; i != keyframes_vec.size(); ++i)
	{
		memcpy(&keyframes[i], &keyframes_vec[i], MIRAGE_KEYFRAME_SIZE);
	}

	return keyframes;
}

char* anim_tool_xml_load_string_table(MIRAGE_INFO* info, std::vector<std::string>& string_table_vec, std::vector<uint32_t>& string_table_offsets_vec)
{
	char* string_table = (char*)calloc(info->string_table_size, 1);
	for(uint32_t i = 0; i != string_table_vec.size(); ++i)
	{
		strncpy((char*)&string_table[string_table_offsets_vec[i]],
				&string_table_vec[i].c_str()[0], string_table_vec[i].size());
	}

	return string_table;
}

void anim_tool_xml_calc_node_offsets(MIRAGE_INFO* info)
{
	info->metadata_offset = MIRAGE_INFO_SIZE;
	info->keyframes_offset = info->metadata_offset + info->metadata_size;
	info->string_table_offset = info->keyframes_offset + info->keyframes_size;
	printf("Final metadata_offset: %u\n", info->metadata_offset);
	printf("Final keyframes_offset: %u\n", info->keyframes_offset);
	printf("Final string_table_offset: %u\n", info->string_table_offset);
}

void anim_tool_xml_calc_footer(MIRAGE_FOOTER* footer, const uint32_t anim_count, const uint32_t metadata_offsets_offset)
{
	footer->offset_count = MIRAGE_INFO_OFFSET_CNT + anim_count;
	footer->offsets = (uint32_t*)calloc(footer->offset_count, 4);
	for(uint32_t i = 0; i != 6; ++i) footer->offsets[i] = i*4; /* First 6 offsets are next to each other */
	for(uint32_t i = 0; i != 2; ++i) footer->offsets[6+i] = i*4 + metadata_offsets_offset;
}

void anim_tool_xml_calc_header(MIRAGE_HEADER* header, MIRAGE_INFO* info, MIRAGE_FOOTER* footer)
{
	header->footer_offset = MIRAGE_HEADER_SIZE + info->string_table_offset + info->string_table_size;
	header->file_size = header->footer_offset + (4 + (footer->offset_count*4));

	printf("Final footer_offset: %u\n", header->footer_offset);
	printf("Final file_size: %u\n", header->file_size);
}
