#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/cpu/endianness.h>
#include <kwaslib/core/data/text/sexml.h>
#include <kwaslib/platinum/dat.h>

/*
    Arguments
*/
const AP_ARG_DESC arg_list[] =
{
    {"--be", AP_TYPE_NOV},
    {"--skip_ext_check", AP_TYPE_NOV},
    {"--block_size", AP_TYPE_U32},
};
const uint32_t arg_list_size = 3;

AP_VALUE_NODE* arg_node = NULL;

/* Flags */
uint8_t dat_tool_endian = FU_HOST_ENDIAN;
uint8_t flag_skip_ext_check = 0;

uint32_t val_block_size = DAT_DEFAULT_BLOCK_SIZE;

void dat_tool_parse_arguments(int argc, char** argv);

/*
    Common
*/
void dat_tool_print_usage(char* program_name);
void dat_tool_print_dat(DAT_FILE* dat);

/* 
    Unpacker
*/
DAT_FILE* dat_tool_parse_file(const char* path);
void dat_tool_extract(DAT_FILE* dat, SU_STRING* out_dir_path);
void dat_tool_write_info(DAT_FILE* dat, SU_STRING* out_path);

/* 
    Packer
*/
uint8_t dat_tool_check_kwasinfo(const char* dir_path);
DAT_FILE* dat_tool_from_kwasinfo(const char* input_dir);
DAT_FILE* dat_tool_from_folder(const char* input_dir);

/*
    Entry point
*/
int main(int argc, char** argv)
{
    if(argc < 2)
    {
        dat_tool_print_usage(argv[0]);
        return 0;
    }

    dat_tool_parse_arguments(argc, argv);
    
    /* It's a file so let's process a DAT file */
    if(pu_is_file(argv[1]))
    {
        DAT_FILE* dat = dat_tool_parse_file(argv[1]);
        
        if(dat == NULL)
        {
            printf("Couldn't process the DAT file.\n");
        }
        else
        {
            dat_tool_print_dat(dat);
            
            PU_PATH* dat_path = pu_split_path(argv[1], strlen(argv[1]));
            
            /* I love hacking my own code */
            su_insert_char(dat_path->name, -1, "_", 1);
            su_insert_char(dat_path->name, -1, dat_path->ext->ptr, 3);
            su_remove(dat_path->ext, 0, -1);
            dat_path->type = PU_PATH_TYPE_DIR;
            
            SU_STRING* path_str = pu_path_to_string(dat_path);
            printf("Dir path: %s\n", path_str->ptr);
            
            dat_tool_extract(dat, path_str);
            
            printf("\nUnpacking done without issues (I hope)\n");
        }
    }
    else if(pu_is_dir(argv[1])) /* It's a directory so let's create a DAT file */
    {
        DAT_FILE* dat = NULL;
        
        if(dat_tool_check_kwasinfo(argv[1]))
        {
            printf("Loading kwasinfo.xml\n");
            dat = dat_tool_from_kwasinfo(argv[1]);
        }
        else
        {
            printf("Couldn't find kwasinfo.xml.\nDAT file might not work properly.\n");
            dat = dat_tool_from_folder(argv[1]);
        }
        
        printf("Preparing the file. This might take a moment.\n");
        
        dat_update(dat, val_block_size);
        
        FU_FILE* fdat = dat_to_fu_file(dat, val_block_size, dat_tool_endian);
        SU_STRING* output_str = su_create_string(argv[1], strlen(argv[1]));

        /* converting the suffix in the folder name to the 3-letter extension */
        if(flag_skip_ext_check)
        {
            su_insert_char(output_str, -1, ".dat", 4);
        }
        else
        {
            if(output_str->ptr[output_str->size-4] == '_')
            {
                output_str->ptr[output_str->size-4] = '.';
            }
        }
        
        /* Saving to the file and overwriting */
        fu_to_file(output_str->ptr, fdat, 1);
        
        /* Cleanup */
        su_free(output_str);
        fu_close(fdat);
        free(fdat);
        dat_destroy(dat);
        
        printf("\nPacking done without issues (I hope)\n");
    }
    else /* Everything failed oops */
    {
        printf("Couldn't do anything.\n");
    }
    
    return 0;
}

/*
    Common
*/

void dat_tool_parse_arguments(int argc, char** argv)
{
    arg_node = ap_parse_argv(argv, argc, arg_list, arg_list_size);

    AP_VALUE_NODE* arg_be = ap_get_node_by_arg(arg_node, "--be");
    AP_VALUE_NODE* arg_skip_ext_check = ap_get_node_by_arg(arg_node, "--skip_ext_check");
    AP_VALUE_NODE* arg_block_size = ap_get_node_by_arg(arg_node, "--block_size");
    
    if(arg_be != NULL) dat_tool_endian = FU_BIG_ENDIAN;
    if(arg_skip_ext_check != NULL) flag_skip_ext_check = 1;
    if(arg_block_size != NULL) val_block_size = arg_block_size->data.value.u32;
}

void dat_tool_print_usage(char* program_name)
{
    printf("Usage:\n");
    printf("\tTo unpack:\t\t%s <dat file>\n", program_name);
    printf("\tTo pack:\t\t%s <directory> <options>\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("\tPacking:\n");
    printf("\t\t%24s\t%s\n", "--be", "Pack the DAT as Big Endian (X360/PS3/WiiU)");
    printf("\t\t%24s\t%s\n", "--skip_ext_check", "Do not get the extension from the folder suffix");
    printf("\t\t%24s\t%s\n", "--block_size", "Block size to be used in the resulting DAT (default is 16)");
}

void dat_tool_print_dat(DAT_FILE* dat)
{    
    printf("File count: %u\n", dat->header.file_count);
    printf("Positions offset: 0x%x\n", dat->header.positions_offset);
    printf("Extensions offset: 0x%x\n", dat->header.extensions_offset);
    printf("Names offset: 0x%x\n", dat->header.names_offset);
    printf("Sizes offset: 0x%x\n", dat->header.sizes_offset);
    printf("Hashtable offset: 0x%x\n", dat->header.hashtable_offset);
    printf("\n");
    printf("Entry name size: %u\n", dat->entry_name_size);
    printf("\n");

    /*
    for(uint32_t i = 0; i != cvec_size(dat->entries); ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        printf("[%05u/%05llu]\n", i, cvec_size(dat->entries)-1);
        printf("\tPosition: %u\n", entry->position);
        printf("\tSize: %u\n", entry->size);
        printf("\tExtension: %.4s\n", entry->extension);
        printf("\tName: %.*s\n", entry->name->size, entry->name->ptr);
        printf("\tData: %.*s\n\n", 4, &entry->data[0]);
    }
    */
}

/*
    Unpacker
*/
DAT_FILE* dat_tool_parse_file(const char* path)
{
    FU_FILE fdat = {0};
    fu_open_file(path, 1, &fdat);
    DAT_FILE* dat = dat_parse_file(&fdat);
    fu_close(&fdat);
    return dat;
}

void dat_tool_extract(DAT_FILE* dat, SU_STRING* out_dir_path)
{
    pu_create_dir_char(out_dir_path->ptr);
    
    for(uint32_t i = 0; i != cvec_size(dat->entries); ++i)
    {
        DAT_FILE_ENTRY* entry = dat_get_entry_by_id(dat->entries, i);
        
        SU_STRING* str_temp = su_copy(out_dir_path);
        su_insert_char(str_temp, -1, "/", 1);
        su_insert_char(str_temp, -1, entry->name->ptr, entry->name->size);
        
        FILE* cur_file = fopen(str_temp->ptr, "wb");
        fwrite(entry->data, entry->size, 1, cur_file);
        fclose(cur_file);
        
        str_temp = su_free(str_temp);
    }
    
    SU_STRING* xml_path = su_copy(out_dir_path);
    su_insert_char(xml_path, -1, "/", 1);
    su_insert_char(xml_path, -1, "kwasinfo.xml", 12);
    
    dat_tool_write_info(dat, xml_path);

    xml_path = su_free(xml_path);
}

void dat_tool_write_info(DAT_FILE* dat, SU_STRING* out_path)
{
    SEXML_ELEMENT* root = sexml_create_root("PlatinumDAT");
    
    for(uint32_t i = 0; i != cvec_size(dat->entries); ++i)
    {
        SEXML_ELEMENT* xml_entry = sexml_append_element(root, "file");
        DAT_FILE_ENTRY* dat_entry = dat_get_entry_by_id(dat->entries, i);
        sexml_append_attribute(xml_entry, "path", dat_entry->name->ptr);
    }
    
    sexml_save_to_file_formatted(out_path->ptr, root, 4);
    root = sexml_destroy(root);
}

/*
    Packer
*/
uint8_t dat_tool_check_kwasinfo(const char* dir_path)
{
    SU_STRING* kwasinfo_xml = su_create_string(dir_path, strlen(dir_path));
    su_insert_char(kwasinfo_xml, -1, "/", 1);
    su_insert_char(kwasinfo_xml, -1, "kwasinfo.xml", 12);
    const uint8_t is_xml = pu_is_file(kwasinfo_xml->ptr);
    kwasinfo_xml = su_free(kwasinfo_xml);
    return is_xml;
}

DAT_FILE* dat_tool_from_kwasinfo(const char* input_dir)
{
    SU_STRING* dir = su_create_string(input_dir, strlen(input_dir));
    DAT_FILE* dat = dat_alloc_dat();
    
    /* kwasinfo.xml */
    SU_STRING* kwasinfo_xml = su_copy(dir);
    su_insert_char(kwasinfo_xml, -1, "/", 1);
    su_insert_char(kwasinfo_xml, -1, "kwasinfo.xml", 12);
    SEXML_ELEMENT* root = sexml_load_from_file(kwasinfo_xml->ptr);
    
    if(su_cmp_string_char(root->name, "PlatinumDAT", 11) == 0)
    {
        const uint64_t file_count = sexml_get_child_count(root, "file");
        printf("File count: %llu\n", file_count);
        
        for(uint32_t i = 0; i != file_count; ++i)
        {
            SEXML_ELEMENT* file = sexml_get_element_by_id(root, i);
            SEXML_ATTRIBUTE* path = sexml_get_attribute_by_name(file, "path");
            SU_STRING* filestr = su_copy(dir);
            su_insert_char(filestr, -1, "/", 1);
            su_insert_string(filestr, -1, path->value);
            PU_PATH* filepath = pu_split_path(filestr->ptr, filestr->size);
            
            FU_FILE file_data = {0};
            fu_open_file(filestr->ptr, 1, &file_data);
           
            dat_append_entry(dat->entries,
                             filepath->ext->ptr, path->value->ptr,
                             file_data.size, (uint8_t*)file_data.buf);
            
            pu_free_path(filepath);
            fu_close(&file_data);
            su_free(filestr);
        }
    }
    
    sexml_destroy(root);
    su_free(kwasinfo_xml);
    su_free(dir);
    return dat;
}

DAT_FILE* dat_tool_from_folder(const char* input_dir)
{
    DAT_FILE* dat = dat_alloc_dat();
    
    DL_DIR_LIST dirlist = {0};
    dl_parse_directory(input_dir, &dirlist);

    for(uint32_t i = 0; i != dirlist.size; ++i)
    {
        if(dirlist.entries[i].type == DL_TYPE_FILE)
        {    
            SU_STRING* file_name = pu_path_to_string(dirlist.entries[i].path);
            
            SU_STRING* file_dir_path = pu_path_to_string(dirlist.path);
            su_insert_char(file_dir_path, -1, "/", 1);
            su_insert_char(file_dir_path, -1, file_name->ptr, file_name->size);

            /* Loading the file */
            FU_FILE file_data = {0};
            fu_open_file(file_dir_path->ptr, 1, &file_data);
            
            /* Append new entry */
            dat_append_entry(dat->entries,
                             dirlist.entries[i].path->ext->ptr, file_name->ptr,
                             file_data.size, (uint8_t*)file_data.buf);
            
            /* Cleanup */
            fu_close(&file_data);
            su_free(file_name);
            su_free(file_dir_path);
        }
    }
    
    dl_free_list(&dirlist);
    
    return dat;
}