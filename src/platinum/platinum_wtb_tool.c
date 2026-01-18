#include <stdio.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <kwaslib/ext/stb_image_write.h>

#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/io/arg_parser.h>
#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/dir_list.h>
#include <kwaslib/core/data/text/sexml.h>
#include <kwaslib/core/data/image/image.h>
#include <kwaslib/core/data/image/gtf.h>
#include <kwaslib/platinum/wtb.h>

/*
    Function declarations
*/

/* Common */
void wtb_tool_print_usage(const char* exe_name);
void wtb_tool_parse_arguments(int argc, char** argv);

/* Unpacking */
void wtb_tool_to_dir(const char* dir_path, WTB_FILE* wtb);
SU_STRING* wtb_tool_ext_by_data(const uint8_t* data);

/* Packing */
SEXML_ELEMENT* wtb_tool_check_kwasinfo(const char* dir_path);
WTB_FILE* wtb_tool_kwasinfo_to_wtb(const char* dir_path, SEXML_ELEMENT* xml);
WTB_FILE* wtb_tool_dir_to_wtb(const char* dir_path);
void wtb_tool_to_wtb(SU_STRING* wtb_path_str, WTB_FILE* wtb);
void wtb_tool_to_wta_wtp(SU_STRING* wta_path_str, WTB_FILE* wtb);

/*
    Globals
*/
AP_DESC* g_arg_node = NULL;

/*
    Flags
*/
uint8_t flag_skip_ext_check = 0;
uint8_t flag_wtb = 0;
uint8_t flag_wta = 0;

/*
    Entry point
*/
int main(int argc, char** argv)
{
    /* Setting up arguments */
    g_arg_node = ap_create();
    
    ap_append_desc_noval(g_arg_node, 0, "--skip_ext_check", "Don't get a file type from directory suffix");
    ap_append_desc_noval(g_arg_node, 1, "--wtb", "Force the creation of standalone WTB file (default)");
    ap_append_desc_noval(g_arg_node, 0, "--wta", "Force the creation of WTA and WTP files");
    
    /* No arguments, print usage */
    if(argc == 1)
    {
        wtb_tool_print_usage(argv[0]);
        return 0;
    }
    
    wtb_tool_parse_arguments(argc, argv);
    g_arg_node = ap_free(g_arg_node);
    
    if(pu_is_dir(argv[1])) /* It's a directory */
    {
        printf("Packing files.\n");
        
        /* Looking for kwasinfo.xml */
        SEXML_ELEMENT* wtb_xml = wtb_tool_check_kwasinfo(argv[1]);
        WTB_FILE* wtb_file = NULL; 
        
        if(wtb_xml) /* XML exists and is valid */
        {
            printf("kwasinfo.xml found\n");
            wtb_file = wtb_tool_kwasinfo_to_wtb(argv[1], wtb_xml);
        }
        else
        {
            printf("Could not find valid kwasinfo.xml\n"
                   "Repacked file might not work properly.\n");
                   
            wtb_file = wtb_tool_dir_to_wtb(argv[1]);
        }
        
        /*
            Preparing the output path for a file.
            WTA+WTP have a forced extension, but we need to check for `_wta` and `_wtp`.
            WTB export is the default, but we can get an extension from suffix.
        */
        uint8_t is_wta = 0;
        uint32_t arg1_len = strlen(argv[1]);

        /* Checking flags */
        if(flag_skip_ext_check == 0) /* Get the extension */
        {
            if(argv[1][arg1_len-4] == '_')
            {
                argv[1][arg1_len-4] = '.';
            }
        }
        
        if(flag_wta)
        {
            is_wta = 1;
        }
        
        if(flag_wtb)
        {
            is_wta = 0;
        }
        
        PU_PATH* dir_path = pu_split_path(argv[1], arg1_len);
        
        if(dir_path->ext->size == PU_PATH_NO_VALUE)
        {
            if(is_wta) dir_path->ext = su_create_string("wta", 3);
            else dir_path->ext = su_create_string("wtb", 3);
        }
        
        if((su_cmp_string_char(dir_path->ext, "wta", 3) == 0)
        || (su_cmp_string_char(dir_path->ext, "wtp", 3) == 0))
        {
            is_wta = 1;
            dir_path->ext->ptr[2] = 'a';
            
            if(flag_wtb)
            {
                is_wta = 0;
                dir_path->ext->ptr[2] = 'b';
            }
        }
        else
        {
            is_wta = 0;
            
            if(flag_wta)
            {
                is_wta = 1;
                dir_path->ext->ptr[0] = 'w';
                dir_path->ext->ptr[1] = 't';
                dir_path->ext->ptr[2] = 'a';
            }
        }
        
        /* Output string, .wta for wta, any other for wtb */
        SU_STRING* dir_path_str = pu_path_to_string(dir_path);
        printf("Output file: %*s\n", dir_path_str->size, dir_path_str->ptr);
        
        if(is_wta) /* Export the WTA+WTP */
        {
            wtb_tool_to_wta_wtp(dir_path_str, wtb_file);
        }
        else /* Export the WTB */
        {
            wtb_tool_to_wtb(dir_path_str, wtb_file);
        }
        
        dir_path = pu_free_path(dir_path);
        dir_path_str = su_free(dir_path_str);
    }
    else if(pu_is_file(argv[1])) /* Could be a WTB or WTA+WTP */
    {
        printf("Extracting files.\n");
        
        FU_FILE fwtb = {0};
        FU_FILE fwta = {0};
        FU_FILE fwtp = {0};
        
        uint8_t wtb_arg_it = 0;
        uint8_t wta_arg_it = 0;
        const uint32_t ext_pos_arg1 = strlen(argv[1])-4;
        
        if(strncmp(&argv[1][ext_pos_arg1], ".wtb", 4) == 0)
        {
            wtb_arg_it = 1;
            fu_open_file(argv[1], 1, &fwtb);
        }
        else if(strncmp(&argv[1][ext_pos_arg1], ".wta", 4) == 0)
        {
            wta_arg_it = 1;
            fu_open_file(argv[1], 1, &fwta);
        }
        else if(strncmp(&argv[1][ext_pos_arg1], ".wtp", 4) == 0)
        {
            fu_open_file(argv[1], 1, &fwtp);
        }
        
        if(pu_is_file(argv[2])) /* Second file exists */
        {
            const uint32_t ext_pos_arg2 = strlen(argv[2])-4;
            
            if(strncmp(&argv[2][ext_pos_arg2], ".wtb", 4) == 0)
            {
                wtb_arg_it = 2;
                fu_open_file(argv[2], 1, &fwtb);
            }
            else if(strncmp(&argv[2][ext_pos_arg2], ".wta", 4) == 0)
            {
                wta_arg_it = 2;
                fu_open_file(argv[2], 1, &fwta);
            }
            else if(strncmp(&argv[2][ext_pos_arg2], ".wtp", 4) == 0)
            {
                fu_open_file(argv[2], 1, &fwtp);
            }
        }
        
        WTB_FILE* wtb = NULL;
        
        if(fwtb.size)
        {
            wtb = wtb_parse_wtb(&fwtb);
            wtb_tool_to_dir(argv[wtb_arg_it], wtb);
        }
        else if(fwta.size && fwtp.size)
        {
            wtb = wtb_parse_wta_wtp(&fwta, &fwtp);
            wtb_tool_to_dir(argv[wta_arg_it], wtb);
        }
        else
        {
            printf("No clue what happened.\n"
                   "Are you sure you are trying to unpack wtb/wta+wtp?\n");

            return 0;
        }
        
        fu_close(&fwtb);
        fu_close(&fwta);
        fu_close(&fwtp);
    }
    else /* Everything failed, print usage again */
    {
        wtb_tool_print_usage(argv[0]);
    }
    
    return 0;
}

/*
    Functions
*/

/* Common */

void wtb_tool_print_usage(const char* exe_name)
{
	printf("Usage:\n");
	printf("\tTo unpack:\t%s <WTB/WTA+WTP file>\n", exe_name);
	printf("\tTo pack:\t%s <directory with DDS files/kwasinfo.xml> <options>\n", exe_name);
	printf("\n");
	printf("Options:\n");
	printf("\tPacking:\n");
    
    for(uint32_t i = 0; i != ap_get_desc_count(g_arg_node); ++i)
    {
        AP_ARG_DESC* apd = ap_get_desc_by_id(g_arg_node, i);
        printf("\t\t%24s\t%s\n", apd->name, apd->description);
    }
    
	printf("\n");
	printf("DDS filenames in the unpacked directory are the texture IDs in decimal.\n");
	printf("Only change them if you know what you are doing.\n");
}

void wtb_tool_parse_arguments(int argc, char** argv)
{
    if(ap_parse(g_arg_node, argc-2, &argv[2]) != AP_STAT_SUCCESS)
    {
        return;
    }

    AP_ARG_VEC arg_ext = ap_get_arg_vec_by_name(g_arg_node, "--skip_ext_check");
    AP_ARG_VEC arg_wtb = ap_get_arg_vec_by_name(g_arg_node, "--wtb");
    AP_ARG_VEC arg_wta = ap_get_arg_vec_by_name(g_arg_node, "--wta");

	if(arg_ext)
	{
		flag_skip_ext_check = 1;
        arg_ext = ap_free_arg_vec(arg_ext);
	}

    if(arg_wtb)
    {
        flag_wtb = 1;
        flag_wta = 0;
        arg_wtb = ap_free_arg_vec(arg_wtb);
    }
    
    if(arg_wta)
    {
        flag_wtb = 0;
        flag_wta = 1;
        arg_wta = ap_free_arg_vec(arg_wta);
    }
}


/* Unpacking */

void wtb_tool_to_dir(const char* dir_path, WTB_FILE* wtb)
{
    SU_STRING* out_str = su_create_string(dir_path, strlen(dir_path));
    
    out_str->ptr[out_str->size-4] = '_';
    pu_create_dir_char(out_str->ptr);
    
    /* kwasinfo.xml */
    SU_STRING* kwasinfo_xml = su_copy(out_str);
    su_insert_char(kwasinfo_xml, -1, "/kwasinfo.xml", 13);
    SEXML_ELEMENT* xml = sexml_create_root("PlatinumWTB");
    
    for(uint32_t i = 0; i != wtb->header.tex_count; ++i)
    {
        WTB_ENTRY* entry = wtb_get_entry_by_id(wtb->entries, i);
        
        char id_buf[11] = {0};
        const uint8_t id_buf_size = sprintf(&id_buf[0], "%u", entry->id);
        
        SU_STRING* file_name = su_create_string(&id_buf[0], id_buf_size);

        SU_STRING* full_file_name = su_copy(out_str);
        su_insert_char(full_file_name, -1, "/", 1);
        
        /* Convert any X360/PS3 texture to PNG */
        if(wtb->platform == WTB_PLATFORM_BE)
        {
			IMAGE* img = NULL;

			if(wtb->header.xpr_info_offset) /* X360 */
			{
				img = x360_texture_to_image(entry->x360, &entry->data[0], entry->size);
			}
			else /* PS3 */
			{
                FU_FILE temp_file = {0};
                fu_create_mem_file(&temp_file);
                fu_write_data(&temp_file, entry->data, entry->size);
                fu_seek(&temp_file, 0, FU_SEEK_SET);
                
				GTF_FILE gtf = gtf_read_file(&temp_file);
				img = gtf_to_image(&gtf);
                
                fu_close(&temp_file);
			}
            
            uint64_t img_size = 0;
            uint8_t* img_data = img_to_raw_data(img, &img_size);
            
            su_insert_char(file_name, -1, ".png", 4);
            su_insert_string(full_file_name, -1, file_name);
            
			stbi_write_png(full_file_name->ptr,
                           img->width, img->height,
                           img->bpp, img_data,
                           img->width*img->bpp);
            
            free(img_data);
			img_free_image(img);
        }
        else
        {
            /* Extension by type */
            SU_STRING* file_ext = wtb_tool_ext_by_data(&entry->data[0]);
            su_insert_string(file_name, -1, file_ext);
            su_insert_string(full_file_name, -1, file_name);
            su_free(file_ext);
            
            FILE* fout = fopen(full_file_name->ptr, "wb");
            fwrite(entry->data, entry->size, 1, fout);
            fclose(fout);
        }
        
        SEXML_ELEMENT* xml_file = sexml_append_element(xml, "file");
        sexml_append_attribute(xml_file, "path", file_name->ptr);
        sexml_append_attribute_bool(xml_file, "atlas", entry->flags.data.atlas);
        
        printf("File name: %s\n", file_name->ptr);
        printf("File size: %u\n", entry->size);
        printf("File id: %u\n", entry->id);
        printf("Flags:\n");
        printf("\tnoncomplex - %u\n", entry->flags.data.noncomplex);
        printf("\talways_set - %u\n", entry->flags.data.always_set);
        printf("\talphaonly - %u\n", entry->flags.data.alphaonly);
        printf("\tdxt1a - %u\n", entry->flags.data.dxt1a);
        printf("\tatlas - %u\n", entry->flags.data.atlas);
        printf("\tcubemap - %u\n", entry->flags.data.cubemap);
        printf("\n");
        
        su_free(file_name);
        su_free(full_file_name);
    }
    
    sexml_save_to_file_formatted(kwasinfo_xml->ptr, xml, 4);
    xml = sexml_destroy(xml);
}

SU_STRING* wtb_tool_ext_by_data(const uint8_t* data)
{
    SU_STRING* ext = NULL;

    if((strncmp((const char*)&data[0], "II*\0", 4) == 0)
        || (strncmp((const char*)&data[0], "MM\0*", 4) == 0)) /* TIFF */
    {
        ext = su_create_string(".tif", 4);
    }
    else if(strncmp((const char*)&data[0], "‰PNG", 4) == 0)   /* PNG */
    {
        ext = su_create_string(".png", 4);
    }
    else if(strncmp((const char*)&data[6], "JFIF", 4) == 0)  /* JPEG */
    {
        ext = su_create_string(".jpg", 4);
    }
    else if(strncmp((const char*)&data[0], "DDS ", 4) == 0)  /* DDS */
    {
        ext = su_create_string(".dds", 4);
    }
    else /* Unknown file */
    {
        ext = su_create_string(".bin", 4);
    }
    
    return ext;
}

/* Packing */

SEXML_ELEMENT* wtb_tool_check_kwasinfo(const char* dir_path)
{
    SEXML_ELEMENT* root = NULL;
    SU_STRING* str_kwasinfo_xml = su_create_string(dir_path, strlen(dir_path));
    su_insert_char(str_kwasinfo_xml, -1, "/kwasinfo.xml", 13); 

    if(pu_is_file(str_kwasinfo_xml->ptr))
    {
        root = sexml_load_from_file(str_kwasinfo_xml->ptr);
        
        if(su_cmp_string_char(root->name, "PlatinumWTB", 11) != 0)
        {
            printf("kwasinfo.xml is not valid PlatinumWTB\n"
                   "Got %*s\n", root->name->size, root->name->ptr);
            root = sexml_destroy(root);
        }
    }
    
    str_kwasinfo_xml = su_free(str_kwasinfo_xml);

    return root;
}

WTB_FILE* wtb_tool_kwasinfo_to_wtb(const char* dir_path, SEXML_ELEMENT* xml)
{
    SU_STRING* dir_str = su_create_string(dir_path, strlen(dir_path));
    WTB_FILE* wtb = wtb_alloc_wtb();
    
    const uint64_t file_count = sexml_get_child_count(xml, "file");
    printf("File count: %llu\n", file_count);
    
    for(uint32_t i = 0; i != file_count; ++i)
    {
        SEXML_ELEMENT* file = sexml_get_element_by_id(xml, i);
        SEXML_ATTRIBUTE* path = sexml_get_attribute_by_name(file, "path");
        SEXML_ATTRIBUTE* atlas_attr = sexml_get_attribute_by_name(file, "atlas");
        const uint8_t atlas_value = atlas_attr ? sexml_get_attribute_bool(atlas_attr) : 0;
        
        if(path == NULL)
        {
            printf("Malformed file entry at index %u\n", i);
            continue;
        }
        
        /* Getting the entry ID */
        PU_PATH* file_path = pu_split_path(path->value->ptr, path->value->size);
        uint32_t entry_id = 0;
        sscanf(file_path->name->ptr, "%u", &entry_id);
        file_path = pu_free_path(file_path);
        
        /* Reading the file contents */
        SU_STRING* file_path_str = su_copy(dir_str);
        su_insert_char(file_path_str, -1, "/", 1);
        su_insert_string(file_path_str, -1, path->value);
        
        FU_FILE file_data = {0};
        fu_open_file(file_path_str->ptr, 1, &file_data);
        
        /* Appending new entry to WTB file */
        wtb_append_entry(wtb->entries, file_data.size, entry_id, (uint8_t*)file_data.buf);
        WTB_ENTRY* new_entry = wtb_get_entry_by_id(wtb->entries, i);
        wtb_set_entry_flags(new_entry, atlas_value);
        
        /* Cleanup */
        fu_close(&file_data);
        file_path_str = su_free(file_path_str);
    }
    
    dir_str = su_free(dir_str);
    
    wtb_update(wtb, 0);
    return wtb;
}

WTB_FILE* wtb_tool_dir_to_wtb(const char* dir_path)
{
    SU_STRING* dir_str = su_create_string(dir_path, strlen(dir_path));
    WTB_FILE* wtb = wtb_alloc_wtb();
    
    DL_DIR_LIST dirlist = {0};
    dl_parse_directory(dir_path, &dirlist);
    
    uint32_t dds_it = 0;
    for(uint32_t i = 0; i != dirlist.size; ++i)
    {
        if(dirlist.entries[i].type == DL_TYPE_FILE)
        {
            SU_STRING* file_path_str = dl_get_full_entry_path(&dirlist, i);
            
            /* Getting the entry ID */
            uint32_t entry_id = 0;
            sscanf(dirlist.entries[i].path->name->ptr, "%u", &entry_id);
            
            /* Reading the file contents */
            FU_FILE file_data = {0};
            fu_open_file(file_path_str->ptr, 1, &file_data);
            
            /* Appending new entry to WTB file */
            wtb_append_entry(wtb->entries, file_data.size, entry_id, (uint8_t*)file_data.buf);
            WTB_ENTRY* new_entry = wtb_get_entry_by_id(wtb->entries, dds_it);
            wtb_set_entry_flags(new_entry, 0);
            
            /* Cleanup */
            fu_close(&file_data);
            file_path_str = su_free(file_path_str);
            
            dds_it += 1;
        }
    }
    
    dir_str = su_free(dir_str);
    
    wtb_update(wtb, 0);
    return wtb;
}

void wtb_tool_to_wtb(SU_STRING* wtb_path_str, WTB_FILE* wtb)
{
    wtb_update(wtb, 0);
    
    FU_FILE* fuwta = wtb_header_to_fu_file(wtb);
    FU_FILE* fuwtp = wtb_data_to_fu_file(wtb);
    
    fu_seek(fuwtp, 0, FU_SEEK_SET);
    fu_write_data(fuwtp, (const uint8_t*)&fuwta->buf[0], fuwta->size);
    
    fu_to_file(wtb_path_str->ptr, fuwtp, 1);
    
    fu_close(fuwta);
    fu_close(fuwtp);
}

void wtb_tool_to_wta_wtp(SU_STRING* wta_path_str, WTB_FILE* wtb)
{
    wtb_update(wtb, 1);
    
    FU_FILE* fuwta = wtb_header_to_fu_file(wtb);
    FU_FILE* fuwtp = wtb_data_to_fu_file(wtb);
    
    fu_to_file(wta_path_str->ptr, fuwta, 1);
    
    wta_path_str->ptr[wta_path_str->size-1] = 'p';
    fu_to_file(wta_path_str->ptr, fuwtp, 1);
    
    fu_close(fuwta);
    fu_close(fuwtp);
}