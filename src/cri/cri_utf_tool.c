#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <kwaslib/core/io/path_utils.h>
#include <kwaslib/core/io/file_utils.h>
#include <kwaslib/core/data/text/sexml.h>
#include <kwaslib/core/data/vl.h>
#include <kwaslib/core/io/arg_parser.h>

#include <kwaslib/cri/utf/utf.h>
#include <kwaslib/cri/utf/utf_table.h>
#include <kwaslib/cri/utf/utf_common.h>

/* ACB command unpacking/packing */
#include "cri_acb_cmd.h"

/* AWB unpacking/packing stuff shared with cri_awb_tool */
#include "cri_awb.h"

/*
    Defines
*/
#define XML_UTF_NAME            (const char*)"CRIUTF"
#define XML_UTF_NAME_SIZE       6

/*
    Globals
*/
AP_DESC* g_arg_node = NULL;
uint8_t g_flag_verbose      = 0;
uint8_t g_flag_overwrite    = 0;
uint8_t g_xml_indent        = 4;
uint8_t g_afs2_counter      = 0; 

/*
	Common
*/
void utf_tool_parse_arguments(int argc, char** argv);

void utf_tool_print_usage(char* program_name);
void utf_tool_print_table(UTF_TABLE* utf);

/*
	Unpacker
*/
void utf_tool_to_xml(UTF_TABLE* utf, SU_STRING* out_file_str);
void utf_tool_table_to_xml(UTF_TABLE* utf, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* utf_name);

/*
	Packer
*/
UTF_TABLE* utf_tool_xml_to_utf(SEXML_ELEMENT* utf_root);

/* 
	Entry
*/
int main(int argc, char** argv)
{
    /* Setting up arguments */
    g_arg_node = ap_create();
    ap_append_desc_noval(g_arg_node, 0, "--verbose", "Print everything regarding the ACB/XML");
    ap_append_desc_noval(g_arg_node, 0, "--force", "Force overwrite of the output");
    ap_append_desc_uint(g_arg_node, 4, "--xml_indent", "Indentation for the XML file");
    
	if(argc == 1)
	{
		utf_tool_print_usage(&argv[0][0]);
		return 0;
	}
    
    utf_tool_parse_arguments(argc, argv);
    g_arg_node = ap_free(g_arg_node);

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
            
            if(su_cmp_string_char(xml_root->name, XML_UTF_NAME, XML_UTF_NAME_SIZE)
               != SU_STRINGS_MATCH)
            {
                printf("Root node is not %*s.\n", XML_UTF_NAME_SIZE, XML_UTF_NAME);
                return 0;
            }
            
            /* Convert the table to FU_FILE for saving */
			UTF_TABLE* utf = utf_tool_xml_to_utf(xml_root);
            FU_FILE* utf_fu = utf_save_file(utf);
            
            if(g_flag_verbose)
            {
                utf_tool_print_table(utf);
            }

			/* Remove XML extension */
			su_remove(input_file_path->ext, 0, -1);
			SU_STRING* utf_out_str = pu_path_to_string(input_file_path);
            
            /* Add a default extension if one is missing (.acb) */
            SU_STRING* utf_out_str_ext = pu_get_ext_char(utf_out_str->ptr);
            
            if(utf_out_str_ext->size == 0)
            {
                su_insert_char(utf_out_str, -1, ".acb", 4);
            }

            /* Saving the table to disk */
			printf("\nUTF Path: %*s\n", utf_out_str->size, utf_out_str->ptr);
            
            /* Check if the file exists before writing to it */
            if(g_flag_overwrite)
            {
                fu_to_file(utf_out_str->ptr, utf_fu, 1);
            }
            else
            {
                if(pu_is_file(utf_out_str->ptr))
                {
                    printf("File \"%s\" exists!\nAre you sure you want to overwrite? [Y/n] ", utf_out_str->ptr);
                    int decision = getc(stdin);
                    
                    switch(decision)
                    {
                        case 'Y':
                            printf("Overwriting...\n");
                            fu_to_file(utf_out_str->ptr, utf_fu, 1);
                            break;
                        default:
                            printf("Not overwriting\n");
                    }
                }
                else
                {
                    fu_to_file(utf_out_str->ptr, utf_fu, 1);
                }
            }
            
            utf_out_str_ext = su_free(utf_out_str_ext);
            utf_out_str = su_free(utf_out_str);
            fu_close(utf_fu);
            free(utf_fu);
		}
		else /* Check if the file is a valid @UTF file */
		{
            FU_FILE utf_fu = {0};
            fu_open_file(argv[1], 1, &utf_fu);
            UTF_TABLE* utf = utf_load_file(&utf_fu);
            fu_close(&utf_fu);
            
            if(utf == NULL)
            {
                printf("File is not a valid @UTF table.\n");
                return 0;
            }
            
			if(g_flag_verbose)
            {
                utf_tool_print_table(utf);
            }

			/* Change the path extension to xml */
			SU_STRING* out_str = pu_path_to_string(input_file_path);
            su_insert_char(out_str, -1, ".xml", 4);
            printf("\nSave Path: %*s\n", out_str->size, out_str->ptr);

			/* Save the UTF to xml */
            if(g_flag_overwrite)
            {
                utf_tool_to_xml(utf, out_str);
            }
            else
            {
                if(pu_is_file(out_str->ptr))
                {
                    printf("File \"%s\" exists!\nAre you sure you want to overwrite? [Y/n] ", out_str->ptr);
                    int decision = getc(stdin);
                    
                    switch(decision)
                    {
                        case 'Y':
                            printf("Overwriting...\n");
                            utf_tool_to_xml(utf, out_str);
                            break;
                        default:
                            printf("Not overwriting\n");
                    }
                }
                else
                {
                    utf_tool_to_xml(utf, out_str);
                }
            }

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
void utf_tool_print_usage(char* program_name)
{
	printf("Converts CRIWARE UTF to XML and vice versa.\n");
	printf("Also parses internal UTF tables, AWB archives and limited subset of ACB Commands.\n");
	printf("Usage:\n");
	printf("\tTo unpack: %s <file.acb> <options>\n", program_name);
	printf("\tTo pack: %s <file.xml> <options>\n", program_name);
    printf("\n");
    printf("Options:\n");

    for(uint32_t i = 0; i != ap_get_desc_count(g_arg_node); ++i)
    {
        AP_ARG_DESC* apd = ap_get_desc_by_id(g_arg_node, i);
        printf("\t%24s\t%s\n", apd->name, apd->description);
    }
}

void utf_tool_parse_arguments(int argc, char** argv)
{
    if(ap_parse(g_arg_node, argc-2, &argv[2]) != AP_STAT_SUCCESS)
    {
        return;
    }

    AP_ARG_VEC arg_verbose = ap_get_arg_vec_by_name(g_arg_node, "--verbose");
    AP_ARG_VEC arg_force = ap_get_arg_vec_by_name(g_arg_node, "--force");
    AP_ARG_VEC arg_xml_indent = ap_get_arg_vec_by_name(g_arg_node, "--xml_indent");
    
    if(arg_verbose)
    {
        g_flag_verbose = 1;
        arg_verbose = ap_free_arg_vec(arg_verbose);
    }
    
    if(arg_verbose)
    {
        g_flag_overwrite = 1;
        arg_force = ap_free_arg_vec(arg_force);
    }
    
    if(arg_xml_indent)
    {
        g_xml_indent = AP_GET_ARG_UINT(AP_ARG_FROM_VEC_BY_ID(arg_xml_indent, 0));
        arg_xml_indent = ap_free_arg_vec(arg_xml_indent);
    }
}

void utf_tool_print_table(UTF_TABLE* utf)
{
    const uint32_t width = utf_table_get_column_count(utf);
    const uint32_t height = utf_table_get_row_count(utf);
    printf("Table: %*s | W:%u H:%u\n", utf->name->size, utf->name->ptr, width, height);
    
    for(uint32_t x = 0; x != width; ++x)
    {
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, x);
        printf("|%31s|%7s|", col->name->ptr, utf_type_to_str(col->type));
        
        for(uint32_t y = 0; y != height; ++y)
        {
            UTF_ROW* row = utf_table_get_row_from_col_by_id(col, y);
            
            switch(col->type)
            {
                case UTF_COLUMN_TYPE_UINT8:
                case UTF_COLUMN_TYPE_SINT8:
                case UTF_COLUMN_TYPE_UINT16:
                case UTF_COLUMN_TYPE_SINT16:
                case UTF_COLUMN_TYPE_UINT32:
                case UTF_COLUMN_TYPE_SINT32:
                case UTF_COLUMN_TYPE_UINT64:
                case UTF_COLUMN_TYPE_SINT64:
                    printf("%8llu|", row->data.u64);
                    break;
                case UTF_COLUMN_TYPE_FLOAT:
                    printf("%8f|", row->data.f32);
                    break;
                case UTF_COLUMN_TYPE_DOUBLE:
                    printf("%8lf|", row->data.f64);
                    break;
                case UTF_COLUMN_TYPE_STRING:
                    for(uint32_t i = 0; i != row->data.str->size; ++i)
                    {
                        char c = row->data.str->ptr[i];
                        if(c < 32) c = 32;
                        if(c > 126) c = 126;
                        printf("%c", c);
                    }
                    printf("|");
                    break;
                case UTF_COLUMN_TYPE_VLDATA:
                    switch(row->embed_type)
                    {
                        case UTF_TABLE_VL_NONE:
                            printf("%08u|", row->data.vl->size);
                            break;
                        case UTF_TABLE_VL_UTF:
                            printf("    @UTF|");
                            break;
                        case UTF_TABLE_VL_AFS2:
                            printf("    AFS2|");
                            break;
                        case UTF_TABLE_VL_ACBCMD:
                            printf("  ACBCMD|");
                            break;
                    }
                    break;
                case UTF_COLUMN_TYPE_UINT128:
                    printf("%16s|", (const char*)&row->data.u128[0]);
                    break;
            }
        }
        
        printf("\n");
    }
}

/*
	Unpacker
*/
void utf_tool_to_xml(UTF_TABLE* utf, SU_STRING* out_file_str)
{
    SU_STRING* work_dir = pu_get_parent_dir_char(out_file_str->ptr);
    SU_STRING* utf_name = pu_get_name_char(out_file_str->ptr);
	SEXML_ELEMENT* xml_root = sexml_create_root("root");
    
    utf_tool_table_to_xml(utf, xml_root, work_dir, utf_name);
    
    SEXML_ELEMENT* utf_xml = sexml_get_element_by_id(xml_root, 0);
    utf_xml->parent = NULL; /* A little bit of trickery */
	sexml_save_to_file_formatted(out_file_str->ptr, utf_xml, g_xml_indent);
    utf_xml->parent = xml_root;
    
    xml_root = sexml_destroy(xml_root);
    work_dir = su_free(work_dir);
    utf_name = su_free(utf_name);
}

void utf_tool_table_to_xml(UTF_TABLE* utf, SEXML_ELEMENT* root, SU_STRING* work_dir, SU_STRING* utf_name)
{
	const uint32_t columns_count = utf_table_get_column_count(utf);
    const uint32_t rows_count = utf_table_get_row_count(utf);

	SEXML_ELEMENT* utf_node = sexml_append_element(root, XML_UTF_NAME);
    sexml_append_attribute(utf_node, "name", utf->name->ptr);
    //sexml_append_attribute_uint(utf_node, "version", 1);

	/* Write schema */
    SEXML_ELEMENT* schema = sexml_append_element(utf_node, "schema");

	for(uint32_t i = 0; i != columns_count; ++i)
	{
        UTF_COLUMN* column = utf_table_get_column_by_id(utf, i);
        SEXML_ELEMENT* column_xml = sexml_append_element(schema, "column");
        sexml_append_attribute(column_xml, "name", column->name->ptr);
        sexml_append_attribute(column_xml, "type", utf_type_to_str(column->type));
	}

	/* Write rows */
    SEXML_ELEMENT* rows_xml = sexml_append_element(utf_node, "rows");

	for(uint32_t i = 0; i != rows_count; ++i)
	{
        SEXML_ELEMENT* row_xml = sexml_append_element(rows_xml, "row");
        sexml_append_attribute_uint(row_xml, "index", i);
        
		for(uint32_t j = 0; j != columns_count; ++j)
		{
            UTF_COLUMN* column = utf_table_get_column_by_id(utf, j);
            UTF_ROW* record = utf_table_get_row_from_col_by_id(column, i);

			if(record->embed_type == UTF_TABLE_VL_UTF)
			{
				utf_tool_table_to_xml(record->embed.utf, row_xml, work_dir, utf_name);
			}
			else if(record->embed_type == UTF_TABLE_VL_AFS2)
            {
                cri_awb_afs2_to_xml(record->embed.afs2, row_xml, work_dir, utf_name, g_afs2_counter);
                g_afs2_counter += 1;
            }
			else if(record->embed_type == UTF_TABLE_VL_ACBCMD)
			{
				cri_acb_cmd_to_xml(record->embed.acbcmd, row_xml);
			}
			else /* Regular record or unknown VL data */
			{
                SEXML_ELEMENT* record_xml = sexml_append_element(row_xml, "record");
				SEXML_ATTRIBUTE* val_xml = sexml_append_attribute(record_xml, "value", "");

				switch(column->type)
				{
					case UTF_COLUMN_TYPE_UINT8:
                        sexml_set_attribute_value_uint(val_xml, record->data.u8);
                        break;
					case UTF_COLUMN_TYPE_SINT8:
                        sexml_set_attribute_value_int(val_xml, record->data.s8);
                        break;
					case UTF_COLUMN_TYPE_UINT16:
                        sexml_set_attribute_value_uint(val_xml, record->data.u16);
                        break;
					case UTF_COLUMN_TYPE_SINT16:
                        sexml_set_attribute_value_int(val_xml, record->data.s16);
                        break;
					case UTF_COLUMN_TYPE_UINT32:
                        sexml_set_attribute_value_uint(val_xml, record->data.u32);
                        break;
					case UTF_COLUMN_TYPE_SINT32:
                        sexml_set_attribute_value_int(val_xml, record->data.s32);
                        break;
					case UTF_COLUMN_TYPE_UINT64:
                        sexml_set_attribute_value_uint(val_xml, record->data.u64);
                        break;
					case UTF_COLUMN_TYPE_SINT64:
                        sexml_set_attribute_value_int(val_xml, record->data.s64);
                        break;
					case UTF_COLUMN_TYPE_FLOAT:
                        sexml_set_attribute_value_double(val_xml, record->data.f32, 8);
                        break;
					case UTF_COLUMN_TYPE_DOUBLE:
                        sexml_set_attribute_value_double(val_xml, record->data.f64, 16);
                        break;
					case UTF_COLUMN_TYPE_STRING: 
                        sexml_set_attribute_value(val_xml, record->data.str->ptr);
						break;
					case UTF_COLUMN_TYPE_VLDATA:
                        sexml_set_attribute_value_vl(val_xml, record->data.vl->ptr, record->data.vl->size);
						break;
					default: break;
				}
                
                sexml_append_attribute(record_xml, "name", column->name->ptr);
			}
		}
	}
}

/*
	Packer
*/
UTF_TABLE* utf_tool_xml_to_utf(SEXML_ELEMENT* utf_xml)
{
    SEXML_ATTRIBUTE* header_name_attr = sexml_get_attribute_by_name(utf_xml, "name");
    SEXML_ELEMENT* schema_xml = sexml_get_element_by_name(utf_xml, "schema");
    SEXML_ELEMENT* rows_xml = sexml_get_element_by_name(utf_xml, "rows");
    const uint32_t columns_count = cvec_size(schema_xml->elements);
    const uint32_t rows_count = cvec_size(rows_xml->elements);
    UTF_TABLE* utf = utf_table_create_by_size(header_name_attr->value->ptr,
                                              columns_count, rows_count);

	/* Set up columns */
    for(uint32_t i = 0; i != columns_count; ++i)
    {
        SEXML_ELEMENT* entry_xml = sexml_get_element_by_id(schema_xml, i);
        SEXML_ATTRIBUTE* col_name_attr = sexml_get_attribute_by_name(entry_xml, "name");
        SEXML_ATTRIBUTE* col_type_attr = sexml_get_attribute_by_name(entry_xml, "type");
        UTF_COLUMN* col = utf_table_get_column_by_id(utf, i);
        su_insert_string(col->name, -1, col_name_attr->value);
        col->type = utf_str_to_type(col_type_attr->value->ptr, col_type_attr->value->size);
    }

    /* Read row data */
    for(uint32_t i = 0; i != rows_count; ++i)
    {
        SEXML_ELEMENT* row_xml = sexml_get_element_by_id(rows_xml, i);
        for(uint32_t j = 0; j != columns_count; ++j)
        {
            SEXML_ELEMENT* record_xml = sexml_get_element_by_id(row_xml, j);
            UTF_COLUMN* col = utf_table_get_column_by_id(utf, j);
            UTF_ROW* record = utf_table_get_row_from_col_by_id(col, i);
            
            /* It's a regular value */
            if(su_cmp_char("record", 6, record_xml->name->ptr, record_xml->name->size) == SU_STRINGS_MATCH)
            {
                SEXML_ATTRIBUTE* val_attr = sexml_get_attribute_by_name(record_xml, "value");
                
				switch(col->type)
				{
					case UTF_COLUMN_TYPE_UINT8:
                        record->data.u8 = sexml_get_attribute_uint(val_attr);
                        break;
					case UTF_COLUMN_TYPE_SINT8:
                        record->data.s8 = sexml_get_attribute_int(val_attr);
                        break;
					case UTF_COLUMN_TYPE_UINT16:
                        record->data.u16 = sexml_get_attribute_uint(val_attr);
                        break;
					case UTF_COLUMN_TYPE_SINT16:
                        record->data.s16 = sexml_get_attribute_int(val_attr);
                        break;
					case UTF_COLUMN_TYPE_UINT32:
                        record->data.u32 = sexml_get_attribute_uint(val_attr);
                        break;
					case UTF_COLUMN_TYPE_SINT32:
                        record->data.s32 = sexml_get_attribute_int(val_attr);
                        break;
					case UTF_COLUMN_TYPE_UINT64:
                        record->data.u64 = sexml_get_attribute_uint(val_attr);
                        break;
					case UTF_COLUMN_TYPE_SINT64:
                        record->data.s64 = sexml_get_attribute_int(val_attr);
                        break;
					case UTF_COLUMN_TYPE_FLOAT:
                        record->data.f32 = sexml_get_attribute_double(val_attr);
                        break;
					case UTF_COLUMN_TYPE_DOUBLE:
                        record->data.f64 = sexml_get_attribute_double(val_attr);
                        break;
					case UTF_COLUMN_TYPE_STRING: 
                        record->data.str = su_copy(val_attr->value);
						break;
					case UTF_COLUMN_TYPE_VLDATA:
                        record->data.vl = sexml_get_attribute_vl(val_attr);
						break;
					default: break;
				}
            }
			else if(su_cmp_char(XML_UTF_NAME, XML_UTF_NAME_SIZE,
                                record_xml->name->ptr, record_xml->name->size)
                                == SU_STRINGS_MATCH)
			{
                /* UTF Table */
                record->embed_type = UTF_TABLE_VL_UTF;
                record->embed.utf = utf_tool_xml_to_utf(record_xml);
                
                if(g_flag_verbose)
                {
                    utf_tool_print_table(record->embed.utf);
                }
			}
			else if(su_cmp_char(XML_AFS2_NAME, XML_AFS2_NAME_SIZE,
                                record_xml->name->ptr, record_xml->name->size)
                                == SU_STRINGS_MATCH)
			{
                /* AFS2 */
                record->embed_type = UTF_TABLE_VL_AFS2;
                record->embed.afs2 = cri_awb_xml_to_afs2(record_xml);
                
                if(g_flag_verbose)
                {
                    cri_awb_print_afs2(record->embed.afs2);
                }
			}
			else if(su_cmp_char(XML_ACBCMD_NAME, XML_ACBCMD_NAME_SIZE,
                                record_xml->name->ptr, record_xml->name->size)
                                == SU_STRINGS_MATCH)
			{
                /* ACB Command */
                record->embed_type = UTF_TABLE_VL_ACBCMD;
                record->embed.acbcmd = cri_acb_cmd_xml_to_acbcmd(record_xml);
                
                if(g_flag_verbose)
                {
                    cri_acb_cmd_print_acbcmd(record->embed.acbcmd);
                }
			}
        }
    }
    
    return utf;
}