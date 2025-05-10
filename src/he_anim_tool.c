#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/data/text/sexml.h>

#include <kwaslib/he/mirage.h>
#include <kwaslib/he/uv_anim.h>
#include <kwaslib/he/cam_anim.h>
#include <kwaslib/he/vis_anim.h>
#include <kwaslib/he/morph_anim.h>
#include <kwaslib/he/pt_anim.h>
#include <kwaslib/he/mat_anim.h>
#include <kwaslib/he/lit_anim.h>

/*
    Defines
*/
#define ANIM_TOOL_DBLP      12

#define ANIM_TOOL_UV        1
#define ANIM_TOOL_CAM       2
#define ANIM_TOOL_VIS       3
#define ANIM_TOOL_MORPH     4
#define ANIM_TOOL_PT        5
#define ANIM_TOOL_MAT       6
#define ANIM_TOOL_LIT       7

/*
    Unpacker
*/

/*
    Function that will load and parse specified animation by type.
    
    Returns an XML document to save.
*/
SEXML_ELEMENT* anim_tool_anim_to_xml(const char* file_path, const uint8_t anim_type);

/*
    Exporters to XML
*/
SEXML_ELEMENT* anim_tool_uv_to_xml(MIRAGE_FILE* mirage, UV_ANIM_FILE* uv);
SEXML_ELEMENT* anim_tool_cam_to_xml(MIRAGE_FILE* mirage, CAM_ANIM_FILE* cam);
SEXML_ELEMENT* anim_tool_vis_to_xml(MIRAGE_FILE* mirage, VIS_ANIM_FILE* vis);
SEXML_ELEMENT* anim_tool_morph_to_xml(MIRAGE_FILE* mirage, MORPH_ANIM_FILE* morph);
SEXML_ELEMENT* anim_tool_pt_to_xml(MIRAGE_FILE* mirage, PT_ANIM_FILE* pt);
SEXML_ELEMENT* anim_tool_mat_to_xml(MIRAGE_FILE* mirage, MAT_ANIM_FILE* mat);
SEXML_ELEMENT* anim_tool_lit_to_xml(MIRAGE_FILE* mirage, LIT_ANIM_FILE* lit);

/*
    Appends an XML node with keyframe set to the XML animation node.
*/
void anim_tool_append_kfs_to_xml(MIRAGE_KEYFRAME_SET* kfs, CVEC keyframes,
                                 SEXML_ELEMENT* xml, const char* name);

/*
    Packer
*/

/*
    Function that will load and parse specified XML by type.
    
    Returns FU_FILE* with data to save.
*/
FU_FILE* anim_tool_xml_to_anim(const char* file_path);

/*
    Exporters to *-anim
*/
FU_FILE* anim_tool_xml_to_uv(SEXML_ELEMENT* xml);
FU_FILE* anim_tool_xml_to_cam(SEXML_ELEMENT* xml);
FU_FILE* anim_tool_xml_to_vis(SEXML_ELEMENT* xml);
FU_FILE* anim_tool_xml_to_morph(SEXML_ELEMENT* xml);
FU_FILE* anim_tool_xml_to_pt(SEXML_ELEMENT* xml);
FU_FILE* anim_tool_xml_to_mat(SEXML_ELEMENT* xml);
FU_FILE* anim_tool_xml_to_lit(SEXML_ELEMENT* xml);

/*
    Reads keyframe set and its keyframes from XML node.
    
    Returns keyframes read.
*/
const uint32_t anim_tool_read_kfs_from_xml(SEXML_ELEMENT* kfs_xml, CVEC keyframes,
                                           CVEC keyframe_sets, const uint32_t kf_start);

/*
    
*/
FU_FILE* anim_tool_wrap_anim_in_mirage(const uint8_t* data, const uint32_t data_size,
                                       const uint32_t data_version, CVEC offset_table);

/*
    Common
*/
void anim_tool_print_usage(const char* exe_path);

/*
    Globals
*/
uint8_t g_animation_type = 0;

/*
    Entry point
*/
int main(int argc, char** argv)
{
    if(argc == 1)
    {
        anim_tool_print_usage(argv[0]);
        return 0;
    }
    
    /* Process a file */
    if(pu_is_file(argv[1]))
    {
        PU_PATH* file_path = pu_split_path(argv[1], strlen(argv[1]));
        SEXML_ELEMENT* out_xml = NULL;  /* anim -> XML */
        FU_FILE* out_anim = NULL;       /* XML -> anim */
        
        if(su_cmp_string_char(file_path->ext, "xml", 3) == 0)
        {
            printf("Processing XML\n");
            out_anim = anim_tool_xml_to_anim(argv[1]);
        }
        else if(su_cmp_string_char(file_path->ext, "uv-anim", 7) == 0)
        {
            printf("Processing uv-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_UV);
        }
        else if(su_cmp_string_char(file_path->ext, "cam-anim", 8) == 0)
        {
            printf("Processing cam-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_CAM);
        }
        else if(su_cmp_string_char(file_path->ext, "vis-anim", 8) == 0)
        {
            printf("Processing vis-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_VIS);
        }
        else if(su_cmp_string_char(file_path->ext, "morph-anim", 10) == 0)
        {
            printf("Processing morph-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_MORPH);
        }
        else if(su_cmp_string_char(file_path->ext, "pt-anim", 7) == 0)
        {
            printf("Processing pt-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_PT);
        }
        else if(su_cmp_string_char(file_path->ext, "mat-anim", 8) == 0)
        {
            printf("Processing mat-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_MAT);
        }
        else if(su_cmp_string_char(file_path->ext, "lit-anim", 8) == 0)
        {
            printf("Processing lit-anim\n");
            out_xml = anim_tool_anim_to_xml(argv[1], ANIM_TOOL_LIT);
        }
        
        /* Everything failed */
        if((out_xml == NULL) && (out_anim == NULL))
        {
            printf("Could not process the file %s\n", argv[1]);
        }
        
        /* Preparing save path */
        file_path->ext = su_free(file_path->ext);
        file_path->ext = su_create_string("", 0);
        SU_STRING* file_path_str = pu_path_to_string(file_path);
        
        /* Export anim file */
        if(out_anim)
        {
            switch(g_animation_type)
            {
                case ANIM_TOOL_UV:
                    su_insert_char(file_path_str, -1, ".uv-anim", 8);
                    break;
                case ANIM_TOOL_CAM:
                    su_insert_char(file_path_str, -1, ".cam-anim", 9);
                    break;
                case ANIM_TOOL_VIS:
                    su_insert_char(file_path_str, -1, ".vis-anim", 9);
                    break;
                case ANIM_TOOL_MORPH:
                    su_insert_char(file_path_str, -1, ".morph-anim", 11);
                    break;
                case ANIM_TOOL_PT:
                    su_insert_char(file_path_str, -1, ".pt-anim", 8);
                    break;
                case ANIM_TOOL_MAT:
                    su_insert_char(file_path_str, -1, ".mat-anim", 9);
                    break;
                case ANIM_TOOL_LIT:
                    su_insert_char(file_path_str, -1, ".lit-anim", 9);
                    break;
            }
            
            printf("Saving an animation as %*s\n", file_path_str->size, file_path_str->ptr);
            fu_to_file(file_path_str->ptr, out_anim, 1);
            fu_close(out_anim);
            free(out_anim);
        }
        
        /* Export XML */
        if(out_xml)
        {
            su_insert_char(file_path_str, -1, ".xml", 4);
            printf("Saving an XML as %*s\n", file_path_str->size, file_path_str->ptr);
            sexml_save_to_file_formatted(file_path_str->ptr, out_xml, 4);
            out_xml = sexml_destroy(out_xml);
        }
        
        file_path_str = su_free(file_path_str);
        file_path = pu_free_path(file_path);
    }
    
    return 0;
}

/*
    Unpacker
*/
SEXML_ELEMENT* anim_tool_anim_to_xml(const char* file_path, const uint8_t anim_type)
{
    FU_FILE mirage_fu = {0};
    fu_open_file(file_path, 1, &mirage_fu);
    
    /* Parse mirage */
    MIRAGE_FILE* mirage = mirage_load_from_data((const uint8_t*)mirage_fu.buf);
    
    /* Convert the anim to XML */
    SEXML_ELEMENT* xml = NULL;
    
    switch(anim_type)
    {
        case ANIM_TOOL_UV:
            UV_ANIM_FILE* uv = uv_anim_load_from_data(mirage->data);
            xml = anim_tool_uv_to_xml(mirage, uv);
            uv = uv_anim_free(uv);
            break;
        case ANIM_TOOL_CAM:
            CAM_ANIM_FILE* cam = cam_anim_load_from_data(mirage->data);
            xml = anim_tool_cam_to_xml(mirage, cam);
            cam = cam_anim_free(cam);
            break;
        case ANIM_TOOL_VIS:
            VIS_ANIM_FILE* vis = vis_anim_load_from_data(mirage->data);
            xml = anim_tool_vis_to_xml(mirage, vis);
            vis = vis_anim_free(vis);
            break;
        case ANIM_TOOL_MORPH:
            MORPH_ANIM_FILE* morph = morph_anim_load_from_data(mirage->data);
            xml = anim_tool_morph_to_xml(mirage, morph);
            morph = morph_anim_free(morph);
            break;
        case ANIM_TOOL_PT:
            PT_ANIM_FILE* pt = pt_anim_load_from_data(mirage->data);
            xml = anim_tool_pt_to_xml(mirage, pt);
            pt = pt_anim_free(pt);
            break;
        case ANIM_TOOL_MAT:
            MAT_ANIM_FILE* mat = mat_anim_load_from_data(mirage->data);
            xml = anim_tool_mat_to_xml(mirage, mat);
            mat = mat_anim_free(mat);
            break;
        case ANIM_TOOL_LIT:
            LIT_ANIM_FILE* lit = lit_anim_load_from_data(mirage->data);
            xml = anim_tool_lit_to_xml(mirage, lit);
            lit = lit_anim_free(lit);
            break;
    }

    /* Cleanup */
    mirage = mirage_free(mirage);
    fu_close(&mirage_fu);
    
    return xml;
}

SEXML_ELEMENT* anim_tool_uv_to_xml(MIRAGE_FILE* mirage, UV_ANIM_FILE* uv)
{
    const char* mat_name = mirage_get_ptr_in_table(uv->string_table,
                                                   uv->metadata.material_name_offset);
    const char* tex_name = mirage_get_ptr_in_table(uv->string_table,
                                                   uv->metadata.texture_name_offset);
    
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("UVAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    sexml_append_attribute(xml, "material_name", mat_name);
    sexml_append_attribute(xml, "texture_name", tex_name);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(uv->entries); ++i)
    {
        UV_ANIM_ENTRY* entry = uv_anim_get_entry_by_id(uv->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(uv->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            anim_tool_append_kfs_to_xml(kfs, uv->keyframes, anim, NULL);
        }
    }
    
    return xml;
}

SEXML_ELEMENT* anim_tool_cam_to_xml(MIRAGE_FILE* mirage, CAM_ANIM_FILE* cam)
{
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("CAMAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(cam->entries); ++i)
    {
        CAM_ANIM_ENTRY* entry = cam_anim_get_entry_by_id(cam->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(cam->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_uint(anim, "rot_or_aim", entry->rot_or_aim);
        sexml_append_attribute_uint(anim, "flag2", entry->flag2);
        sexml_append_attribute_uint(anim, "flag3", entry->flag3);
        sexml_append_attribute_uint(anim, "flag4", entry->flag4);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "cam_pos_x", entry->cam_pos_x, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "cam_pos_z", entry->cam_pos_z, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "cam_pos_y", entry->cam_pos_y, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "cam_rot_x", entry->cam_rot_x, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "cam_rot_z", entry->cam_rot_z, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "cam_rot_y", entry->cam_rot_y, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "aim_pos_x", entry->aim_pos_x, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "aim_pos_z", entry->aim_pos_z, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "aim_pos_y", entry->aim_pos_y, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "twist", entry->twist, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "z_near", entry->z_near, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "z_far", entry->z_far, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "fov", entry->fov, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "aspect_ratio", entry->aspect_ratio, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            anim_tool_append_kfs_to_xml(kfs, cam->keyframes, anim, NULL);
        }
    }
    
    return xml;
}

SEXML_ELEMENT* anim_tool_vis_to_xml(MIRAGE_FILE* mirage, VIS_ANIM_FILE* vis)
{
    const char* mdl_name = mirage_get_ptr_in_table(vis->string_table,
                                                   vis->metadata.model_name_offset);
    const char* unk_name = mirage_get_ptr_in_table(vis->string_table,
                                                   vis->metadata.unk_name_offset);
    
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("VISAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    sexml_append_attribute(xml, "model_name", mdl_name);
    sexml_append_attribute(xml, "unk_name", unk_name);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(vis->entries); ++i)
    {
        VIS_ANIM_ENTRY* entry = vis_anim_get_entry_by_id(vis->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(vis->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            anim_tool_append_kfs_to_xml(kfs, vis->keyframes, anim, NULL);
        }
    }
    
    return xml;
}

SEXML_ELEMENT* anim_tool_morph_to_xml(MIRAGE_FILE* mirage, MORPH_ANIM_FILE* morph)
{
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("MORPHAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(morph->entries); ++i)
    {
        MORPH_ANIM_ENTRY* entry = morph_anim_get_entry_by_id(morph->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(morph->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            const uint32_t morph_name_offset = *(uint32_t*)cvec_at(entry->morph_names_offsets, j);
            const char* name = mirage_get_ptr_in_table(morph->string_table, morph_name_offset);
            anim_tool_append_kfs_to_xml(kfs, morph->keyframes, anim, name);
        }
    }
    
    return xml;
}

SEXML_ELEMENT* anim_tool_pt_to_xml(MIRAGE_FILE* mirage, PT_ANIM_FILE* pt)
{
    const char* mat_name = mirage_get_ptr_in_table(pt->string_table,
                                                   pt->metadata.material_name_offset);
    const char* tex_name = mirage_get_ptr_in_table(pt->string_table,
                                                   pt->metadata.texture_name_offset);
    
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("PTAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    sexml_append_attribute(xml, "material_name", mat_name);
    sexml_append_attribute(xml, "texture_name", tex_name);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(pt->entries); ++i)
    {
        PT_ANIM_ENTRY* entry = pt_anim_get_entry_by_id(pt->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(pt->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            anim_tool_append_kfs_to_xml(kfs, pt->keyframes, anim, NULL);
        }
    }
    
    /* Textures */
    SEXML_ELEMENT* textures = sexml_append_element(xml, "Textures");
    
    uint32_t tex_ptr_it = 0;
    uint32_t tex_it = 0;
    while(tex_ptr_it != pt->texture_table->size)
    {
        const char* cur_str_ptr = &pt->texture_table->ptr[tex_ptr_it];
        const uint32_t cur_str_len = strlen(cur_str_ptr);
        
        if(cur_str_len)
        {
            SEXML_ELEMENT* tex = sexml_append_element(textures, "Texture");
            sexml_append_attribute_uint(tex, "id", tex_it);
            sexml_append_attribute(tex, "name", cur_str_ptr);
            
            tex_it += 1;
        }
        
        tex_ptr_it += cur_str_len + 1;
    }
    
    return xml;
}

SEXML_ELEMENT* anim_tool_mat_to_xml(MIRAGE_FILE* mirage, MAT_ANIM_FILE* mat)
{
    const char* mat_name = mirage_get_ptr_in_table(mat->string_table,
                                                   mat->metadata.material_name_offset);
    
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("MATAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    sexml_append_attribute(xml, "material_name", mat_name);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(mat->entries); ++i)
    {
        MAT_ANIM_ENTRY* entry = mat_anim_get_entry_by_id(mat->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(mat->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            anim_tool_append_kfs_to_xml(kfs, mat->keyframes, anim, NULL);
        }
    }
    
    return xml;
}

SEXML_ELEMENT* anim_tool_lit_to_xml(MIRAGE_FILE* mirage, LIT_ANIM_FILE* lit)
{
    /* root name and params */
    SEXML_ELEMENT* xml = sexml_create_root("LITAnimation");
    sexml_append_attribute_uint(xml, "data_version", mirage->header.data_version);
    
    /* Entries */
    for(uint32_t i = 0; i != cvec_size(lit->entries); ++i)
    {
        LIT_ANIM_ENTRY* entry = lit_anim_get_entry_by_id(lit->entries, i);
        SEXML_ELEMENT* anim = sexml_append_element(xml, "Animation");
        
        /* Animation params */
        const char* anim_name = mirage_get_ptr_in_table(lit->string_table, entry->name_offset);
        sexml_append_attribute(anim, "name", anim_name);
        sexml_append_attribute_uint(anim, "light_type", entry->light_type);
        sexml_append_attribute_uint(anim, "flag2", entry->flag2);
        sexml_append_attribute_uint(anim, "flag3", entry->flag3);
        sexml_append_attribute_uint(anim, "flag4", entry->flag4);
        sexml_append_attribute_double(anim, "frame_rate", entry->frame_rate, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "start_frame", entry->start_frame, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "end_frame", entry->end_frame, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "unk_00", entry->unk_00, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_01", entry->unk_01, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_02", entry->unk_02, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_03", entry->unk_03, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "color_red", entry->color_red, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "color_green", entry->color_green, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "color_blue", entry->color_blue, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_07", entry->unk_07, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "unk_08", entry->unk_08, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_09", entry->unk_09, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_0A", entry->unk_0A, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_0B", entry->unk_0B, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "unk_0C", entry->unk_0C, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_0D", entry->unk_0D, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_0E", entry->unk_0E, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "intensity", entry->intensity, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "unk_10", entry->unk_10, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_11", entry->unk_11, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_12", entry->unk_12, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_13", entry->unk_13, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "unk_14", entry->unk_14, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_15", entry->unk_15, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_16", entry->unk_16, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_17", entry->unk_17, ANIM_TOOL_DBLP);
        
        sexml_append_attribute_double(anim, "unk_18", entry->unk_18, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_19", entry->unk_19, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_1A", entry->unk_1A, ANIM_TOOL_DBLP);
        sexml_append_attribute_double(anim, "unk_1B", entry->unk_1B, ANIM_TOOL_DBLP);
        
        /* Keyframe sets */
        for(uint32_t j = 0; j != entry->keyframe_set_count; ++j)
        {
            MIRAGE_KEYFRAME_SET* kfs = mirage_get_kfs_by_id(entry->keyframe_sets, j);
            anim_tool_append_kfs_to_xml(kfs, lit->keyframes, anim, NULL);
        }
    }
    
    return xml;
}

void anim_tool_append_kfs_to_xml(MIRAGE_KEYFRAME_SET* kfs, CVEC keyframes,
                                 SEXML_ELEMENT* xml, const char* name)
{
    /* Appends new keyframe set to XML animation node */
    SEXML_ELEMENT* kfs_elem = sexml_append_element(xml, "KeyframeSet");
    
    /* Used in morph-anims */
    if(name)
    {
        sexml_append_attribute(kfs_elem, "name", name);
    }

    sexml_append_attribute_uint(kfs_elem, "type", kfs->type);
    sexml_append_attribute_uint(kfs_elem, "flag2", kfs->flag2);
    sexml_append_attribute_uint(kfs_elem, "interpolation", kfs->interpolation);
    sexml_append_attribute_uint(kfs_elem, "flag4", kfs->flag4);
    
    /* Appends Keyframe elements to the keyframe set node */
    for(uint32_t i = kfs->start; i != (kfs->start+kfs->length); ++i)
    {
        MIRAGE_KEYFRAME* kf = mirage_get_kf_by_id(keyframes, i);
        SEXML_ELEMENT* kf_elem = sexml_append_element(kfs_elem, "Keyframe");
        sexml_append_attribute_uint(kf_elem, "index", kf->index);
        sexml_append_attribute_double(kf_elem, "value", kf->value, ANIM_TOOL_DBLP);
    }
}

/*
    Packer
*/
FU_FILE* anim_tool_xml_to_anim(const char* file_path)
{
    FU_FILE* anim_fu = NULL;
    SEXML_ELEMENT* xml = sexml_load_from_file(file_path);
    
    if(su_cmp_string_char(xml->name, "UVAnimation", 11) == 0)
    {
        printf("Found UVAnimation node\n");
        g_animation_type = ANIM_TOOL_UV;
        anim_fu = anim_tool_xml_to_uv(xml);
    }
    else if(su_cmp_string_char(xml->name, "CAMAnimation", 12) == 0)
    {
        printf("Found CAMAnimation node\n");
        g_animation_type = ANIM_TOOL_CAM;
        anim_fu = anim_tool_xml_to_cam(xml);
    }
    else if(su_cmp_string_char(xml->name, "VISAnimation", 12) == 0)
    {
        printf("Found VISAnimation node\n");
        g_animation_type = ANIM_TOOL_VIS;
        anim_fu = anim_tool_xml_to_vis(xml);
    }
    else if(su_cmp_string_char(xml->name, "MORPHAnimation", 14) == 0)
    {
        printf("Found MORPHAnimation node\n");
        g_animation_type = ANIM_TOOL_MORPH;
        anim_fu = anim_tool_xml_to_morph(xml);
    }
    else if(su_cmp_string_char(xml->name, "PTAnimation", 11) == 0)
    {
        printf("Found PTAnimation node\n");
        g_animation_type = ANIM_TOOL_PT;
        anim_fu = anim_tool_xml_to_pt(xml);
    }
    else if(su_cmp_string_char(xml->name, "MATAnimation", 12) == 0)
    {
        printf("Found MATAnimation node\n");
        g_animation_type = ANIM_TOOL_MAT;
        anim_fu = anim_tool_xml_to_mat(xml);
    }
    else if(su_cmp_string_char(xml->name, "LITAnimation", 12) == 0)
    {
        printf("Found LITAnimation node\n");
        g_animation_type = ANIM_TOOL_LIT;
        anim_fu = anim_tool_xml_to_lit(xml);
    }
    else
    {
        printf("Unknown anim XML type \"%*s\"\n", xml->name->size, xml->name->ptr);
    }
    
    xml = sexml_destroy(xml);
    
    return anim_fu;
}

FU_FILE* anim_tool_xml_to_uv(SEXML_ELEMENT* xml)
{
    UV_ANIM_FILE* uv = uv_anim_alloc();
    UV_ANIM_METADATA* m = &uv->metadata;
    SU_STRING* st = uv->string_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");
    SEXML_ATTRIBUTE* mat = sexml_get_attribute_by_name(xml, "material_name");
    SEXML_ATTRIBUTE* tex = sexml_get_attribute_by_name(xml, "texture_name");
    
    m->material_name_offset = mirage_add_str_to_table(st, mat->value->ptr, mat->value->size);
    m->texture_name_offset = mirage_add_str_to_table(st, tex->value->ptr, tex->value->size);
                                                     
    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(uv->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        UV_ANIM_ENTRY* entry = uv_anim_get_entry_by_id(uv->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 uv->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* uvf = uv_anim_export_to_fu(uv);
    FU_FILE* uvaf = anim_tool_wrap_anim_in_mirage((const uint8_t*)uvf->buf, uvf->size,
                                                  data_version, uv_anim_calc_offsets(uv));
    
    /* Cleanup */
    uv = uv_anim_free(uv);
    fu_close(uvf);
    free(uvf);
    
    return uvaf;
}

FU_FILE* anim_tool_xml_to_cam(SEXML_ELEMENT* xml)
{
    CAM_ANIM_FILE* cam = cam_anim_alloc();
    SU_STRING* st = cam->string_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");
    
    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(cam->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        CAM_ANIM_ENTRY* entry = cam_anim_get_entry_by_id(cam->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->rot_or_aim = sexml_get_attribute_uint_by_name(entry_xml, "rot_or_aim");
        entry->flag2 = sexml_get_attribute_uint_by_name(entry_xml, "flag2");
        entry->flag3 = sexml_get_attribute_uint_by_name(entry_xml, "flag3");
        entry->flag4 = sexml_get_attribute_uint_by_name(entry_xml, "flag4");
        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        entry->cam_pos_x = sexml_get_attribute_double_by_name(entry_xml, "cam_pos_x");
        entry->cam_pos_z = sexml_get_attribute_double_by_name(entry_xml, "cam_pos_z");
        entry->cam_pos_y = sexml_get_attribute_double_by_name(entry_xml, "cam_pos_y");
        entry->cam_rot_x = sexml_get_attribute_double_by_name(entry_xml, "cam_rot_x");
        entry->cam_rot_z = sexml_get_attribute_double_by_name(entry_xml, "cam_rot_z");
        entry->cam_rot_y = sexml_get_attribute_double_by_name(entry_xml, "cam_rot_y");
        entry->aim_pos_x = sexml_get_attribute_double_by_name(entry_xml, "aim_pos_x");
        entry->aim_pos_z = sexml_get_attribute_double_by_name(entry_xml, "aim_pos_z");
        entry->aim_pos_y = sexml_get_attribute_double_by_name(entry_xml, "aim_pos_y");
        entry->twist = sexml_get_attribute_double_by_name(entry_xml, "twist");
        entry->z_near = sexml_get_attribute_double_by_name(entry_xml, "z_near");
        entry->z_far = sexml_get_attribute_double_by_name(entry_xml, "z_far");
        entry->fov = sexml_get_attribute_double_by_name(entry_xml, "fov");
        entry->aspect_ratio = sexml_get_attribute_double_by_name(entry_xml, "aspect_ratio");
        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 cam->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* camf = cam_anim_export_to_fu(cam);
    FU_FILE* camaf = anim_tool_wrap_anim_in_mirage((const uint8_t*)camf->buf, camf->size,
                                                   data_version, cam_anim_calc_offsets(cam));
    
    /* Cleanup */
    cam = cam_anim_free(cam);
    fu_close(camf);
    free(camf);
    
    return camaf;
}

FU_FILE* anim_tool_xml_to_vis(SEXML_ELEMENT* xml)
{
    VIS_ANIM_FILE* vis = vis_anim_alloc();
    VIS_ANIM_METADATA* m = &vis->metadata;
    SU_STRING* st = vis->string_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");
    SEXML_ATTRIBUTE* mdl = sexml_get_attribute_by_name(xml, "model_name");
    SEXML_ATTRIBUTE* unk = sexml_get_attribute_by_name(xml, "unk_name");
    
    m->model_name_offset = mirage_add_str_to_table(st, mdl->value->ptr, mdl->value->size);
    m->unk_name_offset = mirage_add_str_to_table(st, unk->value->ptr, unk->value->size);
    
    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(vis->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        VIS_ANIM_ENTRY* entry = vis_anim_get_entry_by_id(vis->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 vis->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* visf = vis_anim_export_to_fu(vis);
    FU_FILE* visaf = anim_tool_wrap_anim_in_mirage((const uint8_t*)visf->buf, visf->size,
                                                  data_version, vis_anim_calc_offsets(vis));
    
    /* Cleanup */
    vis = vis_anim_free(vis);
    fu_close(visf);
    free(visf);
    
    return visaf;
}

FU_FILE* anim_tool_xml_to_morph(SEXML_ELEMENT* xml)
{
    MORPH_ANIM_FILE* morph = morph_anim_alloc();
    SU_STRING* st = morph->string_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");
    
    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(morph->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        MORPH_ANIM_ENTRY* entry = morph_anim_get_entry_by_id(morph->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->morph_names_offsets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            
            /* Morph name */
            SEXML_ATTRIBUTE* morph_name = sexml_get_attribute_by_name(kfs_xml, "name");
            uint32_t morph_name_offset = mirage_add_str_to_table(st, morph_name->value->ptr,
                                                                 morph_name->value->size);
            cvec_push_back(entry->morph_names_offsets, &morph_name_offset);
            
            /* Keyframe set */
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 morph->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* morphf = morph_anim_export_to_fu(morph);
    FU_FILE* morphaf = anim_tool_wrap_anim_in_mirage((const uint8_t*)morphf->buf, morphf->size,
                                                     data_version, morph_anim_calc_offsets(morph));
    
    /* Cleanup */
    morph = morph_anim_free(morph);
    fu_close(morphf);
    free(morphf);
    
    return morphaf;
}

FU_FILE* anim_tool_xml_to_pt(SEXML_ELEMENT* xml)
{
    PT_ANIM_FILE* pt = pt_anim_alloc();
    PT_ANIM_METADATA* m = &pt->metadata;
    SU_STRING* st = pt->string_table;
    SU_STRING* tt = pt->texture_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");
    SEXML_ATTRIBUTE* mat = sexml_get_attribute_by_name(xml, "material_name");
    SEXML_ATTRIBUTE* tex = sexml_get_attribute_by_name(xml, "texture_name");
    
    m->material_name_offset = mirage_add_str_to_table(st, mat->value->ptr, mat->value->size);
    m->texture_name_offset = mirage_add_str_to_table(st, tex->value->ptr, tex->value->size);
                                                     
    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(pt->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        PT_ANIM_ENTRY* entry = pt_anim_get_entry_by_id(pt->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 pt->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Texture table */
    SEXML_ELEMENT* textures_xml = sexml_get_element_by_name(xml, "Textures");
    const uint32_t textures_count = sexml_get_child_count(textures_xml, "Texture");
    
    for(uint32_t i = 0; i != textures_count; ++i)
    {
        SEXML_ELEMENT* texture_xml = sexml_get_element_by_id(textures_xml, i);
        SEXML_ATTRIBUTE* tex_name = sexml_get_attribute_by_name(texture_xml, "name");
        mirage_add_str_to_table(tt, tex_name->value->ptr, tex_name->value->size);
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* ptf = pt_anim_export_to_fu(pt);
    FU_FILE* ptaf = anim_tool_wrap_anim_in_mirage((const uint8_t*)ptf->buf, ptf->size,
                                                  data_version, pt_anim_calc_offsets(pt));
    
    /* Cleanup */
    pt = pt_anim_free(pt);
    fu_close(ptf);
    free(ptf);
    
    return ptaf;
}

FU_FILE* anim_tool_xml_to_mat(SEXML_ELEMENT* xml)
{
    MAT_ANIM_FILE* mat = mat_anim_alloc();
    MAT_ANIM_METADATA* m = &mat->metadata;
    SU_STRING* st = mat->string_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");
    SEXML_ATTRIBUTE* mat_name = sexml_get_attribute_by_name(xml, "material_name");
    
    m->material_name_offset = mirage_add_str_to_table(st, mat_name->value->ptr, mat_name->value->size);
                                                     
    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(mat->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        MAT_ANIM_ENTRY* entry = mat_anim_get_entry_by_id(mat->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 mat->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* matf = mat_anim_export_to_fu(mat);
    FU_FILE* mataf = anim_tool_wrap_anim_in_mirage((const uint8_t*)matf->buf, matf->size,
                                                  data_version, mat_anim_calc_offsets(mat));
    
    /* Cleanup */
    mat = mat_anim_free(mat);
    fu_close(matf);
    free(matf);
    
    return mataf;
}

FU_FILE* anim_tool_xml_to_lit(SEXML_ELEMENT* xml)
{
    LIT_ANIM_FILE* lit = lit_anim_alloc();
    SU_STRING* st = lit->string_table;
    
    /* Header values */
    const uint8_t data_version = sexml_get_attribute_uint_by_name(xml, "data_version");

    /* Preparing entries */
    const uint32_t entries_count = sexml_get_child_count(xml, "Animation");
    cvec_resize(lit->entries, entries_count);
    
    /* Start value for keyframe sets */
    uint32_t kf_start = 0;
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(xml, i);
        LIT_ANIM_ENTRY* entry = lit_anim_get_entry_by_id(lit->entries, i);
        
        /* Entry values */
        SEXML_ATTRIBUTE* name = sexml_get_attribute_by_name(entry_xml, "name");
        entry->name_offset = mirage_add_str_to_table(st, name->value->ptr, name->value->size);

        entry->light_type = sexml_get_attribute_uint_by_name(entry_xml, "light_type");
        entry->flag2 = sexml_get_attribute_uint_by_name(entry_xml, "flag2");
        entry->flag3 = sexml_get_attribute_uint_by_name(entry_xml, "flag3");
        entry->flag4 = sexml_get_attribute_uint_by_name(entry_xml, "flag4");
        entry->frame_rate = sexml_get_attribute_double_by_name(entry_xml, "frame_rate");
        entry->start_frame = sexml_get_attribute_double_by_name(entry_xml, "start_frame");
        entry->end_frame = sexml_get_attribute_double_by_name(entry_xml, "end_frame");
        
        entry->unk_00 = sexml_get_attribute_double_by_name(entry_xml, "unk_00");
        entry->unk_01 = sexml_get_attribute_double_by_name(entry_xml, "unk_01");
        entry->unk_02 = sexml_get_attribute_double_by_name(entry_xml, "unk_02");
        entry->unk_03 = sexml_get_attribute_double_by_name(entry_xml, "unk_03");
        
        entry->color_red = sexml_get_attribute_double_by_name(entry_xml, "color_red");
        entry->color_green = sexml_get_attribute_double_by_name(entry_xml, "color_green");
        entry->color_blue = sexml_get_attribute_double_by_name(entry_xml, "color_blue");
        entry->unk_07 = sexml_get_attribute_double_by_name(entry_xml, "unk_07");
        
        entry->unk_08 = sexml_get_attribute_double_by_name(entry_xml, "unk_08");
        entry->unk_09 = sexml_get_attribute_double_by_name(entry_xml, "unk_09");
        entry->unk_0A = sexml_get_attribute_double_by_name(entry_xml, "unk_0A");
        entry->unk_0B = sexml_get_attribute_double_by_name(entry_xml, "unk_0B");
        
        entry->unk_0C = sexml_get_attribute_double_by_name(entry_xml, "unk_0C");
        entry->unk_0D = sexml_get_attribute_double_by_name(entry_xml, "unk_0D");
        entry->unk_0E = sexml_get_attribute_double_by_name(entry_xml, "unk_0E");
        entry->intensity = sexml_get_attribute_double_by_name(entry_xml, "intensity");
        
        entry->unk_10 = sexml_get_attribute_double_by_name(entry_xml, "unk_10");
        entry->unk_11 = sexml_get_attribute_double_by_name(entry_xml, "unk_11");
        entry->unk_12 = sexml_get_attribute_double_by_name(entry_xml, "unk_12");
        entry->unk_13 = sexml_get_attribute_double_by_name(entry_xml, "unk_13");
        
        entry->unk_14 = sexml_get_attribute_double_by_name(entry_xml, "unk_14");
        entry->unk_15 = sexml_get_attribute_double_by_name(entry_xml, "unk_15");
        entry->unk_16 = sexml_get_attribute_double_by_name(entry_xml, "unk_16");
        entry->unk_17 = sexml_get_attribute_double_by_name(entry_xml, "unk_17");
        
        entry->unk_18 = sexml_get_attribute_double_by_name(entry_xml, "unk_18");
        entry->unk_19 = sexml_get_attribute_double_by_name(entry_xml, "unk_19");
        entry->unk_1A = sexml_get_attribute_double_by_name(entry_xml, "unk_1A");
        entry->unk_1B = sexml_get_attribute_double_by_name(entry_xml, "unk_1B");

        
        /* Keyframe sets */
        const uint32_t kfs_count = sexml_get_child_count(entry_xml, "KeyframeSet");
        entry->keyframe_sets = cvec_create(sizeof(MIRAGE_KEYFRAME_SET));
        entry->keyframe_set_count = kfs_count;
        
        for(uint32_t j = 0; j != kfs_count; ++j)
        {
            SEXML_ELEMENT* kfs_xml = sexml_get_element_by_id(entry_xml, j);
            const uint32_t kf_read = anim_tool_read_kfs_from_xml(kfs_xml,
                                                                 lit->keyframes,
                                                                 entry->keyframe_sets,
                                                                 kf_start);
            kf_start += kf_read;
        }
    }
    
    /* Wrapping anim data in mirage and then exporting to FU_FILE */
    FU_FILE* litf = lit_anim_export_to_fu(lit);
    FU_FILE* litaf = anim_tool_wrap_anim_in_mirage((const uint8_t*)litf->buf, litf->size,
                                                  data_version, lit_anim_calc_offsets(lit));
    
    /* Cleanup */
    lit = lit_anim_free(lit);
    fu_close(litf);
    free(litf);
    
    return litaf;
}

const uint32_t anim_tool_read_kfs_from_xml(SEXML_ELEMENT* kfs_xml, CVEC keyframes,
                                           CVEC keyframe_sets, const uint32_t kf_start)
{
    /* kfs header values */
    const uint8_t type = sexml_get_attribute_uint_by_name(kfs_xml, "type");
    const uint8_t flag2 = sexml_get_attribute_uint_by_name(kfs_xml, "flag2");
    const uint8_t interpolation = sexml_get_attribute_uint_by_name(kfs_xml, "interpolation");
    const uint8_t flag4 = sexml_get_attribute_uint_by_name(kfs_xml, "flag4");
    
    const uint32_t kf_length = sexml_get_child_count(kfs_xml, "Keyframe");
    
    mirage_push_kfs(keyframe_sets, type, flag2, interpolation, flag4, kf_length, kf_start);
    
    /* Pushing keyframes */            
    for(uint32_t k = 0; k != kf_length; ++k)
    {
        SEXML_ELEMENT* kf_xml = sexml_get_element_by_id(kfs_xml, k);
        const double index = sexml_get_attribute_double_by_name(kf_xml, "index");
        const double value = sexml_get_attribute_double_by_name(kf_xml, "value");
        mirage_push_keyframe(keyframes, index, value);
    }
    
    return kf_length;
}

FU_FILE* anim_tool_wrap_anim_in_mirage(const uint8_t* data, const uint32_t data_size,
                                       const uint32_t data_version, CVEC offset_table)
{
    MIRAGE_FILE* mirage = mirage_alloc();
    
    mirage_set_data(mirage, data, data_size);
    mirage->header.data_version = data_version;
    mirage->offset_table = cvec_destroy(mirage->offset_table);
    mirage->offset_table = offset_table;
    
    FU_FILE* mirage_fu = mirage_export_to_fu(mirage);
    
    mirage = mirage_free(mirage);
    
    return mirage_fu;
}

/*
    Common
*/
void anim_tool_print_usage(const char* exe_path)
{
	printf("Converts Hedgehog Engine *-anim to XML and vice versa.\n");
	printf("Currently supports:\n");
	printf("\t- uv-anim\n");
	printf("\t- cam-anim\n");
	printf("\t- vis-anim\n");
	printf("\t- morph-anim\n");
	printf("\t- pt-anim\n");
	printf("\t- mat-anim\n");
	printf("\t- lit-anim\n");
	printf("Usage:\n");
	printf("\tTo unpack:\t%s <file.*-anim>\n", exe_path);
	printf("\tTo pack:\t%s <file.xml>\n", exe_path);
}