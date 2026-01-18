#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/kwas_all.h>

#include <assimp/scene.h>
#include <assimp/Exporter.hpp>

/*
	Common
*/
void wmb4_tool_print_wmb4(WMB4_FILE* wmb);

/* 
	Unpacker
*/
void wmb4_prepare_scene(WMB4_FILE* wmb);

/* 
	Packer
*/

/*
	Entry point
*/
int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: %s file.wmb/file.gltf\n", argv[0]);
		return 0;
	}
	
	/* It's a file so let's process a WMB/GLTF file */
	if(pu_is_file(argv[1]))
	{
		FU_FILE file = {0};
		fu_open_file(argv[1], 1, &file);
		
		WMB4_FILE* wmb = wmb4_parse_wmb4(&file);
		
		if(wmb == NULL)
		{
			printf("Couldn't process the WMB file.\n");
		}
		else
		{			
			/* Print wmb info */
			//wmb4_tool_print_wmb4(wmb);
			
			wmb4_prepare_scene(wmb);

		}
	}
	else /* Everything failed oops */
	{
		printf("Couldn't do anything.\n");
	}
	
	return 0;
}

void wmb4_tool_print_wmb4(WMB4_FILE* wmb)
{	
	WMB4_HEADER* wmbh = &wmb->header;
	
	printf("Magic: %.4s\n", wmbh->magic);
	printf("Unk04: %u\n", wmbh->unk04);
	
	printf("flags_10: %u\n", wmbh->flags.flag_10);
	printf("flags_10000: %u\n", wmbh->flags.flag_10000);
	printf("flags_200: %u\n", wmbh->flags.flag_200);
	
	printf("vertex_has_bone: %u\n", wmb->vertex_has_bone);
	printf("vertex_has_color: %u\n", wmb->vertex_has_color);
	printf("vertex_has_uv: %u\n", wmb->vertex_has_uv);
	
	printf("Primitive type: %x (%s)\n", wmbh->primitive_type,
	       wmbh->primitive_type == 0 ? "Triangle list" : "Triangle strip");
	printf("Unk0E: %x\n", wmbh->unk0E);
	printf("Extents: (%f, %f, %f)-(%f, %f, %f)\n", wmbh->extents.min.x, wmbh->extents.min.y,
												   wmbh->extents.min.z, wmbh->extents.max.x,
												   wmbh->extents.max.y, wmbh->extents.max.z);
												   
	printf("buffer_group_array_offset: %x\n", wmbh->buffer_group_array_offset);
	printf("buffer_group_count: %u\n", wmbh->buffer_group_count);
	printf("sub_mesh_array_offset: %x\n", wmbh->sub_mesh_array_offset);
	printf("sub_mesh_count: %u\n", wmbh->sub_mesh_count);
	printf("mesh_array_offset: %x\n", wmbh->mesh_array_offset);
	printf("bone_array_offset: %x\n", wmbh->bone_array_offset);
	printf("bone_count: %u\n", wmbh->bone_count);
	printf("bone_hierarchy_map_offset: %x\n", wmbh->bone_hierarchy_map_offset);
	printf("bone_hierarchy_map_size: %u\n", wmbh->bone_hierarchy_map_size);
	printf("bone_palette_array_offset: %x\n", wmbh->bone_palette_array_offset);
	printf("bone_palette_count: %u\n", wmbh->bone_palette_count);
	printf("material_array_offset: %x\n", wmbh->material_array_offset);
	printf("material_count: %u\n", wmbh->material_count);
	printf("texture_array_offset: %x\n", wmbh->texture_array_offset);
	printf("texture_count: %u\n", wmbh->texture_count);
	printf("mesh_group_array_offset: %x\n", wmbh->mesh_group_array_offset);
	printf("mesh_group_count: %u\n", wmbh->mesh_group_count);

	if(wmbh->buffer_group_array_offset && wmbh->buffer_group_count)
	{
		for(uint32_t i = 0; i != wmbh->buffer_group_count; ++i)
		{
			WMB4_BUFFER_GROUP* group = &wmb->buffer_groups[i];
			
			printf("\nBuffer group[%u/%u]\n", i+1, wmbh->buffer_group_count);
			
			printf("vertex_array_offset: %x\n", group->vertex_array_offset);
			printf("vertex_info_array_offset: %x\n", group->vertex_info_array_offset);
			printf("unk08_offset: %x\n", group->unk08_offset);
			printf("unk0C_offset: %x\n", group->unk0C_offset);
			printf("vertex_count: %u\n", group->vertex_count);
			printf("index_buffer_offset: %x\n", group->index_buffer_offset);
			printf("index_count: %u\n", group->index_count);
			
			/* Vertices and indices */
			for(uint32_t i = 0; i != group->vertex_count; ++i)
			{
				WMB4_VERTEX* vert = &wmb->vertices[i];
				printf("Vertex[%06u/%06u]:", i+1, group->vertex_count);
				printf("(%f, %f, %f) | ", vert->position.x, vert->position.y, vert->position.z);				  
				printf("UV:(%x, %x)(%f, %f) | ", vert->uv[0], vert->uv[1],
												 vert->uv_f32.x, vert->uv_f32.y);
				printf("NRM:(%x)(%f, %f, %f) | ", vert->normal, vert->normal_f32.x,
												  vert->normal_f32.y, vert->normal_f32.z);
				printf("TNG:(%x)(%f, %f, %f) | ", vert->tangent, vert->tangent_f32.x,
												  vert->tangent_f32.y, vert->tangent_f32.z);
						
				if(wmb->vertex_has_bone)
				{
					printf("BN:(%u,%u,%u,%u)(%u,%u,%u,%u) | ", vert->bone.id0, vert->bone.id1,
															  vert->bone.id2, vert->bone.id3,
															  vert->bone.w0, vert->bone.w1,
															  vert->bone.w2, vert->bone.w3);
				}
				
				if(wmb->vertex_has_color)
				{
					printf("COL: %u | ", vert->color);
				}
				
				if(wmb->vertex_has_color)
				{
					printf("UV2:(%x, %x) ", vert->uv2[0], vert->uv2[1]);
				}
						
				printf("\n");
			}
		}
	}
}

/* 
	Unpacker
*/
void wmb4_prepare_scene(WMB4_FILE* wmb)
{	
	aiMesh* mesh = new aiMesh();
	mesh->mNumVertices = wmb->buffer_groups[0].vertex_count;
	mesh->mVertices = new aiVector3D[mesh->mNumVertices];
	mesh->mNormals = new aiVector3D[mesh->mNumVertices];
	mesh->mTangents = new aiVector3D[mesh->mNumVertices];
	
	mesh->mNumUVComponents[0] = wmb->buffer_groups[0].vertex_count;
	mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumUVComponents[0]];
	
	mesh->mNumFaces = wmb->buffer_groups[0].vertex_count/3;
	mesh->mFaces = new aiFace[mesh->mNumFaces];
	mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
	
	for(uint32_t i = 0; i != mesh->mNumVertices; ++i)
	{
		WMB4_VERTEX* vert = &wmb->vertices[i];
		
		mesh->mVertices[i] = aiVector3D(vert->position.x, vert->position.y, vert->position.z);
		mesh->mNormals[i] = aiVector3D(vert->normal_f32.x, vert->normal_f32.y, vert->normal_f32.z);
		mesh->mTangents[i] = aiVector3D(vert->tangent_f32.x, vert->tangent_f32.y, vert->tangent_f32.z);
		mesh->mTextureCoords[0][i] = aiVector3D(vert->uv_f32.x, vert->uv_f32.y, 0);
	}

	for(uint32_t i = 0; i != mesh->mNumFaces; ++i)
	{
		mesh->mFaces[0].mNumIndices = 3;
		mesh->mFaces[0].mIndices = new unsigned[] { 0, 1, 2 };
	}

	aiMaterial* material = new aiMaterial();

    aiNode* root = new aiNode();
    root->mNumMeshes = 1;
    root->mMeshes = new unsigned [] { 0 };

    aiScene* out = new aiScene();
    out->mNumMeshes = 1;
    out->mMeshes = new aiMesh * [] { mesh };
    out->mNumMaterials = 1;
    out->mMaterials = new aiMaterial * [] { material };
    out->mRootNode = root; 
    out->mMetaData = new aiMetadata();

	Assimp::Exporter exporter;
	exporter.Export(out, "collada", "E:/GIT/ThisKwasior/KwasTools/bin/test.dae");
}

/* 
	Packer
*/