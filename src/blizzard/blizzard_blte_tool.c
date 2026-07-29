#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>

#include <kwaslib/blizzard/blte.h>

/*
    Globals
*/

/*
	Common
*/
void blte_tool_print_usage(char* program_name);

/*
	Unpacker
*/

/*
    Packer
*/

/* 
	Entry
*/
int main(int argc, char** argv)
{
	if(argc == 1)
	{
		blte_tool_print_usage(argv[0]);
		return 0;
	}
    
	/* It's a file so let's process it */
	if(pu_is_file(argv[1]))
	{
        FU_FILE* blte_fu = fu_open(argv[1], 1);

        BLTE_FILE* blte = blte_read_file((uint8_t*)&blte_fu->buf[fu_tell(blte_fu)]);
        
        /* Create a directory for output */
        SU_STRING* arc_path_str = NULL;

        if(blte)
        {
            PU_PATH* arc_path = pu_split_path(argv[1], strlen(argv[1]));
            su_insert_char(arc_path->ext, -1, "_d", 2);
            arc_path_str = pu_path_to_string(arc_path);
            arc_path = pu_free_path(arc_path);
            pu_create_dir_char(arc_path_str->ptr);
        }
        else
        {
            goto exit;
        }

        blte = blte_free(blte);
        
        char nameptr_buf[32] = {0};

        while(1)
        {
            blte = blte_read_file((uint8_t*)&blte_fu->buf[fu_tell(blte_fu)]);
            
            if(blte == NULL)
                break;
            
            const uint64_t raw_data_size = blte_get_raw_data_size(&blte->chunkinfo);
            const uint64_t data_size = blte_get_logical_data_size(&blte->chunkinfo);
            uint8_t* data = blte_data_to_raw(blte);

            sprintf(nameptr_buf, "0x%08x_%c\0", fu_tell(blte_fu), blte->data[0]);
            SU_STRING* arc_file_path = su_copy(arc_path_str);
            su_insert_char(arc_file_path, -1, "/", 1);
            su_insert_char(arc_file_path, -1, nameptr_buf, strlen(nameptr_buf));
            printf("%s %u %u\n", nameptr_buf, raw_data_size, data_size);
            fu_buffer_to_file(arc_file_path->ptr, data, raw_data_size, 1);
            arc_file_path = su_free(arc_file_path);
            
            /* Seek the fu */
            fu_seek(blte_fu, blte->header.header_size, FU_SEEK_CUR);
            fu_seek(blte_fu, raw_data_size, FU_SEEK_CUR);
            
            /* Free current blte */
            free(data);
            blte = blte_free(blte);
        }
        
        arc_path_str = su_free(arc_path_str);
        fu_close(blte_fu);
    }

exit:
	return 0;
}

/*
	Common
*/
void blte_tool_print_usage(char* program_name)
{
	printf("Extractor for Overwatch 1 archives with BLTE files.\n");
	printf("Usage:\n");
	printf("\tTo unpack: %s <data.xxx.xxx>\n", program_name);
}