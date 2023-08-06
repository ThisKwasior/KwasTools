#pragma once

#include <stdint.h>

#include <kwaslib/utils/io/file_utils.h>
#include <kwaslib/utils/math/half.h>
#include <kwaslib/utils/math/vec.h>

#define WMB4_VERTEX_BUFFER_COUNT	(uint32_t)(4)
#define WMB4_MESH_SLOT_COUNT		(uint32_t)(4)

/*
	3D model format used in Platinum Games games (lol).
	
	Format reverse-engineered by MisterCardholder
	(Thank you a lot for providing the source code to Noesis plugin!)
	https://www.vg-resource.com/thread-26460.html
	
	More up-to-date spec by TGE, thanks!
	https://github.com/tge-was-taken/010-Editor-Templates
*/

/*
	Vertex format used.
	Details are currently unknown.
*/
typedef struct
{
    uint8_t flag_1 : 1;
    uint8_t flag_2 : 1;
    uint8_t flag_4 : 1;
    uint8_t flag_8 : 1;
    uint8_t flag_10 : 1;
    uint8_t flag_20 : 1;
    uint8_t flag_40 : 1;
    uint8_t flag_80 : 1;
    uint8_t flag_100 : 1;
    uint8_t flag_200 : 1;
    uint8_t flag_400 : 1;
    uint8_t flag_800 : 1;
    uint8_t flag_1000 : 1;
    uint8_t flag_2000 : 1;
    uint8_t flag_4000 : 1;
    uint8_t flag_8000 : 1;
    uint8_t flag_10000 : 1;
    uint8_t flag_20000 : 1;
    uint8_t flag_40000 : 1;
    uint8_t flag_80000 : 1;
    uint8_t flag_100000 : 1;
    uint8_t flag_200000 : 1;
    uint8_t flag_400000 : 1;
    uint8_t flag_800000 : 1;
    uint8_t flag_1000000 : 1;
    uint8_t flag_2000000 : 1;
    uint8_t flag_4000000 : 1;
    uint8_t flag_8000000 : 1;
    uint8_t flag_10000000 : 1;
    uint8_t flag_20000000 : 1;
    uint8_t flag_40000000 : 1;
    uint8_t flag_80000000 : 1;
} WMB4_VERTEX_FLAGS;

typedef struct
{
	VEC3_FLOAT min;
	VEC3_FLOAT max;
} WMB4_BOUNDING_BOX;

typedef struct
{
	uint8_t magic[4];			/* "WMB4" */
	uint32_t unk04;				/* 0x00000000 */
	WMB4_VERTEX_FLAGS flags;
	uint16_t primitive_type;	/* 0 - tri list; 1 - tri strip; 2 - tri fan*/
	uint16_t unk0E;				/* Sometimes 0, sometimes 0xFFFF */
	WMB4_BOUNDING_BOX extents;
	
	uint32_t buffer_group_array_offset;
	uint32_t buffer_group_count;
	
	uint32_t sub_mesh_array_offset;
	uint32_t sub_mesh_count;
	
	uint32_t mesh_array_offset;
	
	uint32_t bone_array_offset;
	uint32_t bone_count;
	
	uint32_t bone_hierarchy_map_offset;
	uint32_t bone_hierarchy_map_size;
	
	uint32_t bone_palette_array_offset;
	uint32_t bone_palette_count;
	
	uint32_t material_array_offset;
	uint32_t material_count;
	
	uint32_t texture_array_offset;
	uint32_t texture_count;
	
	uint32_t mesh_group_array_offset;
	uint32_t mesh_group_count;
	
	uint8_t padding[0x14]; /* 0 */
} WMB4_HEADER;

typedef struct
{
	uint32_t vertex_array_offset;
	uint32_t vertex_info_array_offset; /* Sometimes doesn't exist */
	uint32_t unk08_offset; /* 0 */
	uint32_t unk0C_offset; /* 0 */
	uint32_t vertex_count;
	uint32_t index_buffer_offset; /* Faces */
	uint32_t index_count;
} WMB4_BUFFER_GROUP;  /* Only one? */

typedef struct
{
	uint8_t id0; /* Bone index */
	uint8_t id1;
	uint8_t id2;
	uint8_t id3;
	uint8_t w0; /* Bone weight */
	uint8_t w1;
	uint8_t w2;
	uint8_t w3;
} WMB4_VERTEX_BONE_INFO;

typedef struct
{
	VEC3_FLOAT position;
	float16_t uv[2];
	uint32_t normal; /* 10.11.11 packed normal */
	uint32_t tangent; /* 10.11.11 packed tangent */
	
	VEC2_FLOAT uv_f32;
	VEC3_FLOAT normal_f32;
	VEC3_FLOAT tangent_f32;

	/* flag_10 */
	WMB4_VERTEX_BONE_INFO bone;
	
	/* flag_10 == 0 && flag_10000 */
	uint32_t color;
	
	/* flag_10 == 0 && flag_200 */
	float16_t uv2[2];
	
} WMB4_VERTEX; /* vertex_array */

typedef struct
{
	/* flag_10 == 0 && flag_10000 */
	uint32_t color;
	
	/* flag_10 == 0 && flag_200 */
	float16_t uv[2];
} WMB4_VERTEX_2; /* vertex_info_array */

typedef struct
{
	uint32_t buffer_group_index;
	uint32_t vertex_buffer_start_index;
	uint32_t index_buffer_start_index;
	uint32_t vertex_count;
	uint32_t face_count;
} WMB4_SUB_MESH;

typedef struct
{
	uint32_t offset;
	uint32_t count;
} WMB4_MESH_SLOT;

typedef struct
{
	uint32_t sub_mesh_index;
	uint32_t group_index; /* Mesh group */
	uint16_t material_index;
	uint16_t bone_palette_index;
	uint32_t version;
} WMB4_MESH;

typedef struct
{
	uint16_t global_index;
	uint16_t local_index;
	uint16_t parent_index;
	uint16_t unk08;
	VEC3_FLOAT local;
	VEC3_FLOAT world;
} WMB4_BONE;

typedef struct
{
	uint16_t first[16];
	uint16_t* second;
	uint16_t* third;
} WMB4_BONE_INDEX_TRANS_TABLE; /* bone index translate table */

typedef struct
{
	uint32_t bone_index_list_offset;
	uint32_t bone_index_count;
	uint8_t* bone_indices; /* bone_index_count */
} WMB4_BONE_PALETTE;

typedef struct
{
	uint32_t shader_name_offset;
	uint32_t material_texture_map_array_offset;
	uint32_t unk08;
	uint32_t shader_parameter_array_offset;
	uint16_t unk10;
	uint16_t unk12;
	uint16_t unk14;
	uint16_t shader_param_count;
} WMB4_MATERIAL;

typedef struct
{
	uint32_t unk00;
	uint32_t albedo_index;
	uint32_t unk08;
	uint32_t unk0C;
	uint32_t unk10;
	uint32_t unk14;
	uint32_t unk18;
	uint32_t normal_index;
} WMB4_TEXTURE_MAP;

typedef struct
{
	uint32_t unk00;
	uint32_t id;
} WMB4_TEXTURE_REF;

typedef struct
{
	uint32_t name_offset;
	WMB4_BOUNDING_BOX extents;
	struct
	{
		uint32_t offset;
		uint32_t count;
	} mesh_ids[5];
} WMB4_MESH_GROUP;

typedef struct
{
	WMB4_HEADER header;
	uint8_t vertex_has_bone : 1;
	uint8_t vertex_has_color : 1;
	uint8_t vertex_has_uv : 1;
	
	/* Global arrays */
	WMB4_BUFFER_GROUP* buffer_groups;
	
	WMB4_VERTEX* vertices;
	WMB4_VERTEX_2* vertices_2;
	uint16_t* indices;
	
	WMB4_SUB_MESH* sub_meshes;
	WMB4_MESH_SLOT mesh_slots[WMB4_MESH_SLOT_COUNT];
	WMB4_MESH* meshes;
	
	WMB4_BONE* bones;
	uint16_t* bone_hierarchy_map;
	WMB4_BONE_INDEX_TRANS_TABLE bone_translate_table;
	WMB4_BONE_PALETTE* bone_palettes;
	
	WMB4_MATERIAL* materials;
	WMB4_TEXTURE_REF* texture_refs;
	
	
} WMB4_FILE;

/*
	Functions
*/

WMB4_FILE* wmb4_parse_wmb4(FU_FILE* f);

void wmb4_load_header(FU_FILE* f, WMB4_FILE* wmb);
void wmb4_load_buffer_groups(FU_FILE* f, WMB4_FILE* wmb);
void wmb4_load_vertices_indices(FU_FILE* f, WMB4_FILE* wmb);
void wmb4_load_sub_meshes(FU_FILE* f, WMB4_FILE* wmb);
void wmb4_load_slots_meshes(FU_FILE* f, WMB4_FILE* wmb);

void wmb4_free(WMB4_FILE* wmb);

/*
	UNSIGNED_INT_10F_11F_11F_REV bit alignment for normals
	normals are 10.11.11 packed normalized floats
	OpenGL Spec https://www.opengl.org/registry/doc/glspec45.core.pdf
	
	Normal unpack and pack
*/
VEC3_FLOAT wmb4_unpack_normal(uint32_t normal);
uint32_t wmb4_pack_normal(VEC3_FLOAT normal);