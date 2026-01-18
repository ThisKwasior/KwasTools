#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/date_utils.h>
#include <kwaslib/core/data/text/sexml.h>

#include <kwaslib/cri/archive/afs.h>

/*
    Globals
*/
#define XML_AFS_NAME            (const char*)"AFS"
#define XML_AFS_NAME_SIZE       3

uint8_t g_afs_counter = 0; 

/*
	Common
*/
void afs_tool_print_usage(char* program_name);
void afs_tool_print_afs(AFS_FILE* afs);

/*
	Unpacker
*/
void afs_tool_to_xml(AFS_FILE* afs, SU_STRING* out_file_str);
void afs_tool_afs_to_xml(AFS_FILE* afs, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* afs_name);

/*
    Packer
*/
AFS_FILE* afs_tool_xml_to_afs(SEXML_ELEMENT* afs_root);

/* 
	Entry
*/
int main(int argc, char** argv)
{
	if(argc == 1)
	{
		afs_tool_print_usage(argv[0]);
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
            
            if(su_cmp_string_char(xml_root->name, XML_AFS_NAME, XML_AFS_NAME_SIZE)
               != SU_STRINGS_MATCH)
            {
                printf("Root node is not %*s.\n", XML_AFS_NAME_SIZE, XML_AFS_NAME);
                return 0;
            }
            
            /* Convert the AFS to FU_FILE* for saving */
			AFS_FILE* afs = afs_tool_xml_to_afs(xml_root);
            FU_FILE* fafs = afs_write_to_fu(afs, AFS_BLOCK_SIZE_DEFAULT);
        
			/* Remove XML extension */
			su_remove(input_file_path->ext, 0, -1);
			SU_STRING* afs_out_str = pu_path_to_string(input_file_path);
            
            /* Add a default extension if one is missing (.afs) */
            SU_STRING* afs_out_str_ext = pu_get_ext_char(afs_out_str->ptr);
            
            if(afs_out_str_ext->size == 0)
            {
                su_insert_char(afs_out_str, -1, ".afs", 4);
            }
        
            /* Saving the AFS archive to disk */
			printf("\nAFS Path: %*s\n", afs_out_str->size, afs_out_str->ptr);
            fu_to_file(afs_out_str->ptr, fafs, 1);
            
            afs = afs_free(afs);
            fu_close(fafs);
            free(fafs);
            afs_out_str_ext = su_free(afs_out_str_ext);
            afs_out_str = su_free(afs_out_str);
		}
		else /* Check if the file is a valid AFS file */
		{
            FU_FILE* afs_fu = fu_open(argv[1], 1);
            AFS_FILE* afs = afs_read_from_fu(afs_fu);
            fu_close(afs_fu);
            free(afs_fu);
            
            if(afs == NULL)
            {
                printf("File is not a valid AFS file.\n");
                return 0;
            }
            
			afs_tool_print_afs(afs);

			/* Change the path extension to xml */
			SU_STRING* out_str = pu_path_to_string(input_file_path);
            su_insert_char(out_str, -1, ".xml", 4);
            printf("\nSave Path: %*s\n", out_str->size, out_str->ptr);

			/* Save the AFS to xml */
			afs_tool_to_xml(afs, out_str);

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
void afs_tool_print_usage(char* program_name)
{
	printf("Converts CRI Middleware's AFS archive to XML and vice versa.\n");;
	printf("Usage:\n");
	printf("\tTo unpack: %s <file.afs>\n", program_name);
	printf("\tTo pack: %s <file.afs.xml>\n", program_name);
}

void afs_tool_print_afs(AFS_FILE* afs)
{
    printf("### AFS ###\n");
    for(uint32_t i = 0; i != AFS_MAX_FILES; ++i)
    {
        AFS_ENTRY* entry = afs_get_entry_by_id(afs, i);
        
        if(entry->data)
        {
            printf("[%05u]", i);
            
            switch(entry->data_type)
            {
                case AFS_DATA_ADX:  printf("[ADX] "); break;
                case AFS_DATA_AHX:  printf("[AHX] "); break;
                case AFS_DATA_AFS:  printf("[AFS] "); break;
                default:            printf("[BIN] ");
            }
            
            printf("Size: %*u", 10, entry->size);

            if(afs->has_metadata)
            {
                printf(" | ");
                printf("%04d-%02d-%02d ", entry->metadata.year,
                                          entry->metadata.month,
                                          entry->metadata.day);
                printf("%02d:%02d:%02d | ", entry->metadata.hour,
                                            entry->metadata.minute,
                                            entry->metadata.second);
                printf("%.*s", AFS_ENTRY_METADATA_NAME_SIZE,
                               entry->metadata.name);
            }
            
            printf("\n");
        }
    }
    printf("### AFS END ###\n");
}

/*
	Unpacker
*/
void afs_tool_to_xml(AFS_FILE* afs, SU_STRING* out_file_str)
{
    SU_STRING* work_dir = pu_get_parent_dir_char(out_file_str->ptr);
    SU_STRING* afs_name = pu_get_name_char(out_file_str->ptr);
	SEXML_ELEMENT* xml_root = sexml_create_root("root");
    
    afs_tool_afs_to_xml(afs, xml_root, work_dir, afs_name);
    
    SEXML_ELEMENT* afs_xml = sexml_get_element_by_id(xml_root, 0);
    afs_xml->parent = NULL; /* A little bit of trickery */
	sexml_save_to_file_formatted(out_file_str->ptr, afs_xml, 4);
    afs_xml->parent = xml_root;
    
    xml_root = sexml_destroy(xml_root);
    work_dir = su_free(work_dir);
    afs_name = su_free(afs_name);
    
    g_afs_counter += 1;
}

void afs_tool_afs_to_xml(AFS_FILE* afs, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* afs_name)
{
    SU_STRING* xml_path = su_copy(work_dir);
    su_insert_char(xml_path, -1, "/", 1);
    su_insert_string(xml_path, -1, afs_name);
    
    SU_STRING* files_dir = su_copy(xml_path);
    char dir_suffix_buf[4] = {0};
    sprintf(dir_suffix_buf, "_%02x", g_afs_counter);
    su_insert_char(files_dir, -1, dir_suffix_buf, 3);
    su_insert_char(files_dir, -1, "/", 1);

    su_insert_char(xml_path, -1, ".xml", 4);
    
    /* Create the directory */
    pu_create_dir_char(files_dir->ptr);
    
    /* Writing XML stuff */
    SEXML_ELEMENT* afs_node = sexml_append_element(root, XML_AFS_NAME);
    
    for(uint32_t i = 0; i != AFS_MAX_FILES; ++i)
    {
        AFS_ENTRY* cur_entry = afs_get_entry_by_id(afs, i);
        
        if(cur_entry->data)
        {
            SEXML_ELEMENT* entry_xml = sexml_append_element(afs_node, "entry");
            SU_STRING* cur_file = su_copy(files_dir);
            
            /* Use the name from metadata. Otherwise use the ID. */
            if(afs->has_metadata)
            {
                su_insert_char(cur_file, -1, cur_entry->metadata.name, AFS_ENTRY_METADATA_NAME_SIZE);            
            }
            else
            {
                char buf[6] = {0};
                sprintf(buf, "%05u", i);
                su_insert_char(cur_file, -1, buf, 5);     
                
                switch(cur_entry->data_type)
                {
                    case AFS_DATA_ADX:
                        su_insert_char(cur_file, -1, ".adx", 4);
                        break;
                    case AFS_DATA_AHX:
                        su_insert_char(cur_file, -1, ".ahx", 4);
                        break;
                    case AFS_DATA_AFS:
                        su_insert_char(cur_file, -1, ".afs", 4);
                        break;
                    default:
                        su_insert_char(cur_file, -1, ".bin", 4);
                }
            }
            
            sexml_append_attribute_uint(entry_xml, "id", i);
            sexml_append_attribute(entry_xml, "path", cur_file->ptr);
            fu_buffer_to_file(cur_file->ptr, (char*)cur_entry->data, cur_entry->size, 1);
            
            /* Set the date for the file */
            if(afs->has_metadata)
            {
                AFS_ENTRY_METADATA* m = &cur_entry->metadata;
                const time_t file_date = du_values_to_epoch(m->year, m->month, m->day,
                                                            m->hour, m->minute, m->second);
                du_set_file_time(cur_file->ptr, file_date);
            }
            
            cur_file = su_free(cur_file);
        }
    }
    
    files_dir = su_free(files_dir);
    xml_path = su_free(xml_path);
}

/*
	Packer
*/
AFS_FILE* afs_tool_xml_to_afs(SEXML_ELEMENT* afs_root)
{
    AFS_FILE* afs = afs_alloc();
    
    afs->has_metadata = AFS_HAS_METADATA;
    const uint32_t entries_count = cvec_size(afs_root->elements);
    
    for(uint32_t i = 0; i != entries_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(afs_root, i);
        
        /* Check if it really is an entry */
        if(su_cmp_char(entry_xml->name->ptr, entry_xml->name->size, "entry", 5) == SU_STRINGS_MATCH)
        {
            SEXML_ATTRIBUTE* id_attr = sexml_get_attribute_by_name(entry_xml, "id");
            SEXML_ATTRIBUTE* file_path = sexml_get_attribute_by_name(entry_xml, "path");
            
            /* Only proceed if both values exist */
            if(id_attr && file_path)
            {
                const uint32_t id = sexml_get_attribute_uint(id_attr);
                
                /* We don't care for files that are over the max */
                if(id < AFS_MAX_FILES)
                {
                    FU_FILE* f = fu_open(file_path->value->ptr, 1);
                    
                    if(f)
                    {
                        SU_STRING* name = pu_get_basename_char(file_path->value->ptr);
                        const time_t timestamp = du_get_file_time(file_path->value->ptr);
                        afs_set_entry_data(afs, id, (uint8_t*)f->buf, f->size, name->ptr, timestamp);
                        
                        name = su_free(name);
                        fu_close(f);
                        free(f);
                    }
                }
            }
        }
    }

    return afs;
}