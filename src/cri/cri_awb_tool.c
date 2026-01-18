#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/text/sexml.h>

/* Unpacking/packing stuff shared with cri_utf_tool */
#include "cri_awb.h"

/*
    Globals
*/
uint8_t g_afs2_counter = 0; 

/*
	Common
*/
void awb_tool_print_usage(char* program_name);

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
			AWB_FILE* afs2 = cri_awb_xml_to_afs2(xml_root);
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
            
			cri_awb_print_afs2(afs2);

			/* Change the path extension to xml */
			SU_STRING* out_str = pu_path_to_string(input_file_path);
            su_insert_char(out_str, -1, ".xml", 4);
            printf("\nSave Path: %*s\n", out_str->size, out_str->ptr);

			/* Save the AFS2 to xml */
			cri_awb_to_xml(afs2, out_str, g_afs2_counter);
            g_afs2_counter += 1;

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
