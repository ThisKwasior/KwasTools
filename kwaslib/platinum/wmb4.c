#include "wmb4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

WMB4_FILE* wmb4_parse_wmb4(FU_FILE* f)
{
	WMB4_FILE* wmb = (WMB4_FILE*)calloc(1, sizeof(WMB4_FILE));
	if(wmb == NULL)
	{
		printf("Couldn't allocate the WMB structure.\n");
		return NULL;
	}
	
	wmb4_load_header(f, wmb);
	wmb4_load_buffer_groups(f, wmb);
	wmb4_load_vertices_indices(f, wmb);
	
	return wmb;
}

void wmb4_load_header(FU_FILE* f, WMB4_FILE* wmb)
{
	WMB4_HEADER* h = &wmb->header;
	
	uint64_t bytes_read = 0;
	uint8_t status = 0;
	
	fu_read_data(f, &h->magic[0], 4, &bytes_read);
	
	if(strncmp("WMB4", (const char*)&h->magic[0], 4) != 0)
	{
		printf("File is not a WMB4.\n");
		return;
	}
	
	h->unk04 = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	
	fu_read_data(f, (uint8_t*)&h->flags, 4, &bytes_read);
	
	h->primitive_type = fu_read_u16(f, &status, FU_HOST_ENDIAN);
	h->unk0E = fu_read_u16(f, &status, FU_HOST_ENDIAN);
	h->extents.min.x = fu_read_f32(f, &status, FU_HOST_ENDIAN);
	h->extents.min.y = fu_read_f32(f, &status, FU_HOST_ENDIAN);
	h->extents.min.z = fu_read_f32(f, &status, FU_HOST_ENDIAN);
	h->extents.max.x = fu_read_f32(f, &status, FU_HOST_ENDIAN);
	h->extents.max.y = fu_read_f32(f, &status, FU_HOST_ENDIAN);
	h->extents.max.z = fu_read_f32(f, &status, FU_HOST_ENDIAN);
	h->buffer_group_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->buffer_group_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->sub_mesh_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->sub_mesh_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->mesh_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->bone_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->bone_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->bone_hierarchy_map_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->bone_hierarchy_map_size = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->bone_palette_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->bone_palette_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->material_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->material_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->texture_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->texture_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->mesh_group_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	h->mesh_group_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	
	fu_read_data(f, &h->padding[0], 0x14, &bytes_read);
	
	wmb->vertex_has_bone = h->flags.flag_10;
	wmb->vertex_has_color = ((h->flags.flag_10 == 0) && (h->flags.flag_10000));
	wmb->vertex_has_uv = ((h->flags.flag_10 == 0) && (h->flags.flag_200));
}

void wmb4_load_buffer_groups(FU_FILE* f, WMB4_FILE* wmb)
{
	WMB4_HEADER* h = &wmb->header;
	
	wmb->buffer_groups = (WMB4_BUFFER_GROUP*)calloc(h->buffer_group_count, sizeof(WMB4_BUFFER_GROUP));
	
	if(wmb->buffer_groups == NULL)
	{
		printf("Could not allocate buffer groups array.\n");
		return;
	}
	
	fu_seek(f, h->buffer_group_array_offset, FU_SEEK_SET);
	
	uint8_t status = 0;
	
	for(uint32_t i = 0; i != h->buffer_group_count; ++i)
	{
		WMB4_BUFFER_GROUP* group = &wmb->buffer_groups[i];
		
		group->vertex_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		group->vertex_info_array_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		group->unk08_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		group->unk0C_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		group->vertex_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		group->index_buffer_offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		group->index_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	}
}

void wmb4_load_vertices_indices(FU_FILE* f, WMB4_FILE* wmb)
{
	WMB4_HEADER* h = &wmb->header;
	WMB4_BUFFER_GROUP* group = &wmb->buffer_groups[0];
	
	uint8_t status = 0;
	
	/* Vertex */
	if(group->vertex_array_offset && group->vertex_count)
	{
		wmb->vertices = (WMB4_VERTEX*)calloc(group->vertex_count, sizeof(WMB4_VERTEX));
		
		fu_seek(f, group->vertex_array_offset, FU_SEEK_SET);
		
		for(uint32_t i = 0; i != group->vertex_count; ++i)
		{
			WMB4_VERTEX* vert = &wmb->vertices[i];
			
			vert->position.x = fu_read_f32(f, &status, FU_HOST_ENDIAN);
			vert->position.y = fu_read_f32(f, &status, FU_HOST_ENDIAN);
			vert->position.z = fu_read_f32(f, &status, FU_HOST_ENDIAN);
			vert->uv[0] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
			vert->uv[1] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
			vert->normal = fu_read_u32(f, &status, FU_HOST_ENDIAN);
			vert->tangent = fu_read_u32(f, &status, FU_HOST_ENDIAN);
			
			vert->uv_f32.x = half_to_float32(vert->uv[0]);
			vert->uv_f32.y = half_to_float32(vert->uv[0]);
			vert->normal_f32 = wmb4_unpack_normal(vert->normal);
			vert->tangent_f32 = wmb4_unpack_normal(vert->tangent);
			
			if(h->flags.flag_10)
			{
				vert->bone.id0 = fu_read_u8(f, &status);
				vert->bone.id1 = fu_read_u8(f, &status);
				vert->bone.id2 = fu_read_u8(f, &status);
				vert->bone.id3 = fu_read_u8(f, &status);
				vert->bone.w0 = fu_read_u8(f, &status);
				vert->bone.w1 = fu_read_u8(f, &status);
				vert->bone.w2 = fu_read_u8(f, &status);
				vert->bone.w3 = fu_read_u8(f, &status);
			}
			
			if((h->flags.flag_10 == 0) && (h->flags.flag_10000))
			{
				vert->color = fu_read_u32(f, &status, FU_HOST_ENDIAN);
			}
			
			if((h->flags.flag_10 == 0) && (h->flags.flag_200))
			{
				vert->uv2[0] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
				vert->uv2[1] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
			}
		}
	}
	
	/* Vertex 2 */
	if(group->vertex_info_array_offset && group->vertex_count)
	{
		wmb->vertices_2 = (WMB4_VERTEX_2*)calloc(group->vertex_count, sizeof(WMB4_VERTEX_2));
		
		fu_seek(f, group->vertex_info_array_offset, FU_SEEK_SET);
		
		for(uint32_t i = 0; i != group->vertex_count; ++i)
		{
			WMB4_VERTEX_2* vert = &wmb->vertices_2[i];
			
			if((h->flags.flag_10 == 0) && (h->flags.flag_10000))
			{
				vert->color = fu_read_u32(f, &status, FU_HOST_ENDIAN);
			}
			
			if((h->flags.flag_10 == 0) && (h->flags.flag_200))
			{
				vert->uv[0] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
				vert->uv[1] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
			}
		}
	}
	
	/* Indices */
	if(group->index_buffer_offset && group->index_count)
	{
		wmb->indices = (uint16_t*)calloc(group->index_count, sizeof(uint16_t));
		
		for(uint32_t i = 0; i != group->index_count; ++i)
		{
			wmb->indices[i] = fu_read_u16(f, &status, FU_HOST_ENDIAN);
		}
	}
}

void wmb4_load_sub_meshes(FU_FILE* f, WMB4_FILE* wmb)
{
	WMB4_HEADER* h = &wmb->header;
	
	wmb->sub_meshes = (WMB4_SUB_MESH*)calloc(h->sub_mesh_count, sizeof(WMB4_SUB_MESH));
	if(wmb->sub_meshes == NULL)
	{
		printf("Could not allocate sub_meshes array.\n");
		return;
	}
	
	uint8_t status = 0;
	
	/* Loading sub_meshes */
	fu_seek(f, h->sub_mesh_array_offset, FU_SEEK_SET);
	
	for(uint32_t i = 0; i != h->sub_mesh_count; ++i)
	{
		wmb->sub_meshes[i].buffer_group_index = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		wmb->sub_meshes[i].vertex_buffer_start_index = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		wmb->sub_meshes[i].index_buffer_start_index = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		wmb->sub_meshes[i].vertex_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		wmb->sub_meshes[i].face_count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	}
}

void wmb4_load_slots_meshes(FU_FILE* f, WMB4_FILE* wmb)
{
	WMB4_HEADER* h = &wmb->header;
	uint8_t status = 0;
	
	fu_seek(f, h->mesh_array_offset, FU_SEEK_SET);
	
	for(uint32_t i = 0; i != WMB4_MESH_SLOT_COUNT; ++i)
	{
		wmb->mesh_slots[i].offset = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		wmb->mesh_slots[i].count = fu_read_u32(f, &status, FU_HOST_ENDIAN);
	}
	
	/* First mesh slot usually contains all meshes */
	if(wmb->mesh_slots[0].offset && wmb->mesh_slots[0].count)
	{
		wmb->meshes = (WMB4_MESH*)calloc(wmb->mesh_slots[0].count, sizeof(WMB4_MESH));
		if(wmb->sub_meshes == NULL)
		{
			printf("Could not allocate sub_meshes array.\n");
			return;
		}
		
		fu_seek(f, wmb->mesh_slots[0].offset, FU_SEEK_SET);
		
		for(uint32_t i = 0; i != wmb->mesh_slots[0].count; ++i)
		{
			wmb->meshes[i].sub_mesh_index = fu_read_u32(f, &status, FU_HOST_ENDIAN);
			wmb->meshes[i].group_index = fu_read_u32(f, &status, FU_HOST_ENDIAN);
			wmb->meshes[i].material_index = fu_read_u16(f, &status, FU_HOST_ENDIAN);
			wmb->meshes[i].bone_palette_index = fu_read_u16(f, &status, FU_HOST_ENDIAN);
			wmb->meshes[i].version = fu_read_u32(f, &status, FU_HOST_ENDIAN);
		}
	}
}

void wmb4_free(WMB4_FILE* wmb)
{
	if(wmb)
	{
		if(wmb->buffer_groups) free(wmb->buffer_groups);
		if(wmb->vertices) free(wmb->vertices);
		if(wmb->vertices_2) free(wmb->vertices_2);
		if(wmb->indices) free(wmb->indices);
		if(wmb->sub_meshes) free(wmb->sub_meshes);
		if(wmb->meshes) free(wmb->meshes);
		
		free(wmb);
	}
}

/*
	Normal unpack and pack
*/
VEC3_FLOAT wmb4_unpack_normal(uint32_t normal)
{
	const uint16_t xy_mask = 0x7FF;
	const uint16_t z_mask = 0x3FF;
	
	VEC3_FLOAT unpacked;
	unpacked.x = (normal&xy_mask);
	normal >>= 11;
	unpacked.y = normal&xy_mask;
	normal >>= 11;
	unpacked.z = normal&z_mask;
	
	unpacked.x /= (float)xy_mask;
	unpacked.y /= (float)xy_mask;
	unpacked.z /= (float)z_mask;

	return unpacked;
}

uint32_t wmb4_pack_normal(VEC3_FLOAT normal)
{
	const uint16_t xy_mask = 0x7FF;
	const uint16_t z_mask = 0x3FF;
	
	const uint32_t xn = ((uint32_t)(normal.x*xy_mask))&xy_mask;
	const uint32_t yn = ((uint32_t)(normal.y*xy_mask))&xy_mask;
	const uint32_t zn = ((uint32_t)(normal.z*z_mask))&z_mask;
	
	const uint32_t packed = xn + (yn<<11) + (zn<<22);
	
	return packed;
}