#pragma once

#include <kwaslib/core/data/text/sexml.h>
#include <kwaslib/cri/audio/awb.h>

/*
    Defines
*/
#define XML_AFS2_NAME           (const char*)"AWB"
#define XML_AFS2_NAME_SIZE      3

/*
	Unpacking
*/
inline static void cri_awb_to_xml(AWB_FILE* afs2, SU_STRING* out_file_str, const uint8_t awb_number);
inline static void cri_awb_afs2_to_xml(AWB_FILE* afs2, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* awb_name, const uint8_t awb_number);

/*
	Packing
*/
inline static AWB_FILE* cri_awb_xml_to_afs2(SEXML_ELEMENT* awb_root);

/*
    Misc
*/
inline static void cri_awb_print_afs2(AWB_FILE* awb);

/*
    Implementation
*/

/*
    Unpacking
*/
inline static void cri_awb_to_xml(AWB_FILE* afs2, SU_STRING* out_file_str, const uint8_t awb_number)
{
    SU_STRING* work_dir = pu_get_parent_dir_char(out_file_str->ptr);
    SU_STRING* awb_name = pu_get_name_char(out_file_str->ptr);
	SEXML_ELEMENT* xml_root = sexml_create_root("root");
    
    cri_awb_afs2_to_xml(afs2, xml_root, work_dir, awb_name, awb_number);
    
    SEXML_ELEMENT* awb_xml = sexml_get_element_by_id(xml_root, 0);
    awb_xml->parent = NULL; /* A little bit of trickery */
	sexml_save_to_file_formatted(out_file_str->ptr, awb_xml, 4);
    awb_xml->parent = xml_root;
    
    xml_root = sexml_destroy(xml_root);
    work_dir = su_free(work_dir);
    awb_name = su_free(awb_name);
}

inline static void cri_awb_afs2_to_xml(AWB_FILE* afs2, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* awb_name, const uint8_t awb_number)
{
    SU_STRING* xml_path = su_copy(work_dir);
    su_insert_char(xml_path, -1, "/", 1);
    su_insert_string(xml_path, -1, awb_name);
    
    SU_STRING* files_dir = su_copy(xml_path);
    char dir_suffix_buf[4] = {0};
    sprintf(dir_suffix_buf, "_%02x", awb_number);
    su_insert_char(files_dir, -1, dir_suffix_buf, 3);
    su_insert_char(files_dir, -1, "/", 1);

    su_insert_char(xml_path, -1, ".xml", 4);
    
    /* Create the directory */
    pu_create_dir_char(files_dir->ptr);
    
    /* Writing XML stuff */
    SEXML_ELEMENT* afs2_node = sexml_append_element(root, XML_AFS2_NAME);
    sexml_append_attribute_uint(afs2_node, "version", afs2->header.version);
    sexml_append_attribute_uint(afs2_node, "offset_size", afs2->header.offset_size);
    sexml_append_attribute_uint(afs2_node, "id_size", afs2->header.id_size);
    sexml_append_attribute_uint(afs2_node, "alignment", afs2->header.alignment);
    sexml_append_attribute_uint(afs2_node, "subkey", afs2->header.subkey);
    
    for(uint32_t i = 0; i != awb_get_file_count(afs2); ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_append_element(afs2_node, "entry");
        AWB_ENTRY* entry = awb_get_entry_by_id(afs2, i);
        SU_STRING* cur_file = su_copy(files_dir);
        char buf[32] = {0};
        const uint32_t buf_id_size = sprintf(buf, "%05u", entry->id);
        su_insert_char(cur_file, -1, buf, buf_id_size);
        
        switch(entry->type)
        {
            case AWB_DATA_ADX:
                su_insert_char(cur_file, -1, ".adx", 4);
                break;
            case AWB_DATA_AHX:
                su_insert_char(cur_file, -1, ".ahx", 4);
                break;
            case AWB_DATA_HCA:
                su_insert_char(cur_file, -1, ".hca", 4);
                break;
            case AWB_DATA_BCWAV:
                su_insert_char(cur_file, -1, ".bcwav", 6);
                break;
            default:
                su_insert_char(cur_file, -1, ".bin", 4);
        }
        
        sexml_append_attribute_uint(entry_xml, "id", entry->id);
        sexml_append_attribute(entry_xml, "path", cur_file->ptr);
        
        fu_buffer_to_file(cur_file->ptr, (char*)entry->data, entry->size, 1);
        
        cur_file = su_free(cur_file);
    }
    
    files_dir = su_free(files_dir);
    xml_path = su_free(xml_path);
}

/*
    Packing
*/
inline static AWB_FILE* cri_awb_xml_to_afs2(SEXML_ELEMENT* awb_root)
{
    AWB_FILE* afs2 = awb_alloc();
    
    afs2->header.version = sexml_get_attribute_int_by_name(awb_root, "version");
    afs2->header.offset_size = sexml_get_attribute_int_by_name(awb_root, "offset_size");
    afs2->header.id_size = sexml_get_attribute_int_by_name(awb_root, "id_size");
    afs2->header.alignment = sexml_get_attribute_int_by_name(awb_root, "alignment");
    afs2->header.subkey = sexml_get_attribute_int_by_name(awb_root, "subkey");
    const uint32_t entries_count = cvec_size(awb_root->elements);
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(awb_root, i);
        
        /* Check if it really is an entry */
        if(su_cmp_char(entry_xml->name->ptr, entry_xml->name->size, "entry", 5) == SU_STRINGS_MATCH)
        {
            SEXML_ATTRIBUTE* id_attr = sexml_get_attribute_by_name(entry_xml, "id");
            SEXML_ATTRIBUTE* file_path = sexml_get_attribute_by_name(entry_xml, "path");
            
            /* Only proceed if both values exist */
            if(id_attr && file_path)
            {
                const uint32_t id = sexml_get_attribute_uint(id_attr);
                FU_FILE audio_file = {0};
                fu_open_file(file_path->value->ptr, 1, &audio_file);
                
                if(audio_file.size != 0)
                    awb_append_entry(afs2, id, (uint8_t*)audio_file.buf, audio_file.size);
                
                fu_close(&audio_file);
            }
        }
    }

    return afs2;
}

/*
    Misc
*/
inline static void cri_awb_print_afs2(AWB_FILE* awb)
{
    printf("### AFS2 ###\n");
    for(uint32_t i = 0; i != awb_get_file_count(awb); ++i)
    {
        AWB_ENTRY* entry = awb_get_entry_by_id(awb, i);
        printf("ID: %u\n", entry->id);
        printf("\tSize: %u\n", entry->size);
        printf("\tOffset: %u\n", entry->offset);
        printf("\tType: %u\n", entry->type);
    }
    printf("### AFS2 END ###\n");
}