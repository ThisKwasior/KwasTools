#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/text/sexml.h>

#include <kwaslib/cri/audio/awb.h>

/*
    Globals
*/
#define XML_AFS2_NAME           (const char*)"AWB"
#define XML_AFS2_NAME_SIZE      3

/*
	Common
*/
void awb_tool_print_usage(char* program_name);
void awb_tool_print_afs2(AWB_FILE* awb);

/*
	Unpacker
*/
void awb_tool_to_xml(AWB_FILE* afs2, SU_STRING* out_file_str);
void awb_tool_afs2_to_xml(AWB_FILE* afs2, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* awb_name);

/*
	Packer
*/
AWB_FILE* awb_tool_xml_to_afs2(SEXML_ELEMENT* awb_root);

/* 
	Entry
*/
int main(int argc, char** argv)
{
	if(argc == 1)
	{
		awb_tool_print_usage(&argv[0][0]);
		return 0;
	}

	/* It's a file so let's process it */
	if(pu_is_file(argv[1]))
	{
		PU_PATH* input_file_path = pu_split_path(argv[1], strlen(argv[1]));

		/* It's an XML */
		if(su_cmp_string_char(input_file_path->ext, "xml", 3) == SU_STRINGS_MATCH)
		{
            SEXML_ELEMENT* xml_root = sexml_load_from_file(argv[1]);
            
            if(xml_root == NULL)
            {
                printf("Could not load the XML document.\n");
                return 0;
            }
            
            if(su_cmp_string_char(xml_root->name, XML_AFS2_NAME, XML_AFS2_NAME_SIZE)
               != SU_STRINGS_MATCH)
            {
                printf("Root node is not %*s.\n", XML_AFS2_NAME_SIZE, XML_AFS2_NAME);
                return 0;
            }
            
            /* Convert the AWB to SU_STRING* for saving */
			AWB_FILE* afs2 = awb_tool_xml_to_afs2(xml_root);
			SU_STRING* afs2_data = awb_to_data(afs2);

			/* Remove XML extension */
			su_remove(input_file_path->ext, 0, -1);
			SU_STRING* awb_out_str = pu_path_to_string(input_file_path);
            
            /* Add a default extension if one is missing (.awb) */
            SU_STRING* awb_out_str_ext = pu_get_ext_char(awb_out_str->ptr);
            
            if(awb_out_str_ext->size == 0)
            {
                su_insert_char(awb_out_str, -1, ".awb", 4);
            }

            /* Saving the AFS2 archive to disk */
			printf("\nAWB Path: %*s\n", awb_out_str->size, awb_out_str->ptr);
            fu_buffer_to_file(awb_out_str->ptr, afs2_data->ptr, afs2_data->size, 1);
            
            afs2 = awb_free(afs2);
            afs2_data = su_free(afs2_data);
            awb_out_str_ext = su_free(awb_out_str_ext);
            awb_out_str = su_free(awb_out_str);
		}
		else /* Check if the file is a valid AFS2 file */
		{
            FU_FILE awb_fu = {0};
            fu_open_file(argv[1], 1, &awb_fu);
            AWB_FILE* afs2 = awb_load_from_data((uint8_t*)awb_fu.buf, awb_fu.size);
            fu_close(&awb_fu);
            
            if(afs2 == NULL)
            {
                printf("File is not a valid AFS2 file.\n");
                return 0;
            }
            
			awb_tool_print_afs2(afs2);

			/* Change the path extension to xml */
			SU_STRING* out_str = pu_path_to_string(input_file_path);
            su_insert_char(out_str, -1, ".xml", 4);
            printf("\nSave Path: %*s\n", out_str->size, out_str->ptr);

			/* Save the AFS2 to xml */
			awb_tool_to_xml(afs2, out_str);

			/* Cleanup */
            out_str = su_free(out_str);
		}
        
        input_file_path = pu_free_path(input_file_path);
	}

	return 0;
}


/*
	Common
*/
void awb_tool_print_usage(char* program_name)
{
	printf("Converts CRIWARE AWB archive to XML and vice versa.\n");;
	printf("Usage:\n");
	printf("\tTo unpack: %s <file.awb>\n", program_name);
	printf("\tTo pack: %s <file.xml>\n", program_name);
}

void awb_tool_print_afs2(AWB_FILE* awb)
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

/*
	Unpacker
*/
void awb_tool_to_xml(AWB_FILE* afs2, SU_STRING* out_file_str)
{
    SU_STRING* work_dir = pu_get_parent_dir_char(out_file_str->ptr);
    SU_STRING* awb_name = pu_get_name_char(out_file_str->ptr);
	SEXML_ELEMENT* xml_root = sexml_create_root("root");
    
    awb_tool_afs2_to_xml(afs2, xml_root, work_dir, awb_name);
    
    SEXML_ELEMENT* awb_xml = sexml_get_element_by_id(xml_root, 0);
    awb_xml->parent = NULL; /* A little bit of trickery */
	sexml_save_to_file_formatted(out_file_str->ptr, awb_xml, 4);
    awb_xml->parent = xml_root;
    
    xml_root = sexml_destroy(xml_root);
    work_dir = su_free(work_dir);
    awb_name = su_free(awb_name);
}

void awb_tool_afs2_to_xml(AWB_FILE* afs2, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* awb_name)
{
    char buf[32] = {0};
    
    SU_STRING* xml_path = su_copy(work_dir);
    su_insert_char(xml_path, -1, "/", 1);
    su_insert_string(xml_path, -1, awb_name);
    
    SU_STRING* files_dir = su_copy(xml_path);
    const uint32_t buf_size = sprintf(buf, "_0x%08x", &afs2);
    su_insert_char(files_dir, -1, buf, buf_size);
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
        const uint32_t buf_id_size = sprintf(buf, "%05u", entry->id);
        su_insert_char(cur_file, -1, buf, buf_id_size);
        
        switch(entry->type)
        {
            case AWB_DATA_ADX:
                su_insert_char(cur_file, -1, ".adx", 4);
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
	Packer
*/
AWB_FILE* awb_tool_xml_to_afs2(SEXML_ELEMENT* awb_root)
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