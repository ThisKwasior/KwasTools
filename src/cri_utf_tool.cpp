#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <string>

#include <pugixml/pugixml.hpp>

#include <kwaslib/kwas_all.h>

#define AWB_TOOL_NO_MAIN
#include "cri_awb_tool.cpp"

/*
	Common
*/
void utf_tool_print_usage(char* exe_path);

std::string utf_tool_vl_to_hex(uint8_t* vl, uint32_t size);
uint8_t* utf_tool_hex_to_vl(uint8_t* hex, uint32_t size);

PU_STRING work_dir_str = {0};

/*
	Unpacker
*/
void utf_tool_to_xml(CRI_UTF_FILE* utf, PU_STRING* out);
void utf_tool_table_to_xml(CRI_UTF_FILE* utf, pugi::xml_node* node);

void utf_tool_acbcmd_to_xml(ACB_COMMAND* cmd, pugi::xml_node* node);

/*
	Packer
*/
CRI_UTF_FILE* utf_tool_xml_to_utf(pugi::xml_node* criutf);
ACB_COMMAND* utf_tool_xml_to_acbcmd(pugi::xml_node* acbcmd);

/* 
	Entry
*/
int main(int argc, char** argv)
{
	if(argc == 1)
	{
		utf_tool_print_usage(&argv[0][0]);
		return 0;
	}

	/* It's a file so let's process it */
	if(pu_is_file(argv[1]))
	{
		PU_PATH input_file_path = {0};
		pu_split_path(argv[1], strlen(argv[1]), &input_file_path);

		/* It's an XML */
		if(strncmp(input_file_path.ext.p, "xml", 3) == 0)
		{
			pugi::xml_document doc;
			doc.load_file(argv[1]);
			pugi::xml_node root = doc.root();
			pugi::xml_node criutf = root.child("CRIUTF");
			CRI_UTF_FILE* utf = utf_tool_xml_to_utf(&criutf);
			FU_FILE* cri = cri_utf_write_file(utf);

			/* Remove XML extension */
			pu_free_string(&input_file_path.ext);
			PU_STRING utf_out_str = {0};
			pu_path_to_string(&input_file_path, &utf_out_str);

			printf("UTF Path: %s\n", utf_out_str.p);
			fu_buffer_to_file(utf_out_str.p, cri->buf, cri->size, 1);
		}
		else /* Check if the file is a valid @UTF file */
		{
			FU_FILE cri = {0};
			fu_open_file(argv[1], 1, &cri);
			CRI_UTF_FILE* utf = cri_utf_read_file(&cri);
			cri_utf_print(utf);

			/* Change the path extension to xml */
			/*pu_free_string(&input_file_path.ext);*/

			pu_path_to_string(&input_file_path, &work_dir_str);

			pu_insert_char(".xml", 4, -1, &input_file_path.ext);
			PU_STRING out_str = {0};
			pu_path_to_string(&input_file_path, &out_str);

			/* Save the UTF to xml */
			utf_tool_to_xml(utf, &out_str);

			/* Close the file */
			fu_close(&cri);
			cri_utf_free(utf);
			free(utf);
		}
	}

	return 0;
}

/*
	Common
*/
void utf_tool_print_usage(char* exe_path)
{
	printf("Converts CRIWARE UTF to XML and vice versa.\n");
	printf("Also parses internal UTF tables, AWB archives and limited subset of ACB Commands.\n");
	printf("Usage:\n");
	printf("\tTo unpack: %s <file.acb>\n", exe_path);
	printf("\tTo pack: %s <file.xml>\n", exe_path);
}

std::string utf_tool_vl_to_hex(uint8_t* vl, uint32_t size)
{
	char buf[3] = {0};
	std::string out;
	
	for(uint32_t i = 0; i != size; ++i)
	{
		sprintf(buf, "%02x", vl[i]);
		out.append(&buf[0], 2);
	}

	return out;
}

uint8_t* utf_tool_hex_to_vl(uint8_t* hex, uint32_t size)
{
	const uint32_t size_converted = size/2;
	uint8_t* vl = (uint8_t*)calloc(1, size_converted);

	char buf[3] = {0};
	uint8_t buf_int = 0;

	uint32_t vl_pos = 0;
	for(uint32_t i = 0; i != size; i+=2)
	{
		buf[0] = hex[i];
		buf[1] = hex[i+1];
		sscanf(buf, "%02hhx", &buf_int);
		vl[vl_pos] = buf_int;
		vl_pos += 1;
	}

	return vl;
}

/*
	Unpacker
*/
void utf_tool_to_xml(CRI_UTF_FILE* utf, PU_STRING* out)
{
	pugi::xml_document doc;
	pugi::xml_node root = doc.root();
	utf_tool_table_to_xml(utf, &root);
	doc.save_file(out->p);
}

void utf_tool_table_to_xml(CRI_UTF_FILE* utf, pugi::xml_node* node)
{
	CRI_UTF_HEADER* header = &utf->header;
	CRI_UTF_COLUMN* cols = &utf->columns[0];

	pugi::xml_node cri = node->append_child("CRIUTF");

	cri.append_attribute("name").set_value(header->name);
	cri.append_attribute("version").set_value(header->version);

	/* Write schema */
	pugi::xml_node schema = cri.append_child("schema");

	for(uint32_t i = 0; i != header->columns; ++i)
	{
		pugi::xml_node column = schema.append_child("column");
		column.append_attribute("name").set_value(cols[i].name);
		column.append_attribute("type").set_value(cri_utf_type_to_str(cols[i].desc.type));
	}

	/* Write rows */
	pugi::xml_node rows = cri.append_child("rows");

	for(uint32_t i = 0; i != header->rows; ++i)
	{
		pugi::xml_node row = rows.append_child("row");
		for(uint32_t j = 0; j != header->columns; ++j)
		{
			CRI_UTF_RECORD* record = &cols[j].records[i];

			/* If it is an UTF table, parse it */
			if(record->utf)
			{
				utf_tool_table_to_xml(record->utf, &row);
			}
			else if(record->awb) /* internal AWB */
			{
				PU_STRING dir = {0};
				pu_create_string(work_dir_str.p, work_dir_str.s, &dir);
				char offset_str[32] = {0};
				sprintf(offset_str, "_0x%08x", record->awb->file_offset);
				pu_insert_char(offset_str, strlen(offset_str), -1, &dir);
				awb_tool_afs2_to_xml(record->awb, &row, &dir);
			}
			else if(record->acbcmd) // ACB command 
			{
				utf_tool_acbcmd_to_xml(record->acbcmd, &row);
			}
			else
			{
				pugi::xml_node record_node = row.append_child("record");
				pugi::xml_attribute val = record_node.append_attribute("value");
				record_node.append_attribute("name").set_value(cols[j].name);
				std::string vlhex;

				switch(cols[j].desc.type)
				{
					case COLUMN_TYPE_UINT8: val.set_value(record->data.u8); break;
					case COLUMN_TYPE_SINT8: val.set_value(record->data.s8); break;
					case COLUMN_TYPE_UINT16: val.set_value(record->data.u16); break;
					case COLUMN_TYPE_SINT16: val.set_value(record->data.s16); break;
					case COLUMN_TYPE_UINT32: val.set_value(record->data.u32); break;
					case COLUMN_TYPE_SINT32: val.set_value(record->data.s32); break;
					case COLUMN_TYPE_UINT64: val.set_value(record->data.u64); break;
					case COLUMN_TYPE_SINT64: val.set_value(record->data.s64); break;
					case COLUMN_TYPE_FLOAT: val.set_value(record->data.f32); break;
					case COLUMN_TYPE_DOUBLE: val.set_value(record->data.f64); break;
					case COLUMN_TYPE_STRING: 
						val.set_value(record->data.str, record->size);
						break;
					case COLUMN_TYPE_VLDATA:
						vlhex = utf_tool_vl_to_hex(record->data.vl, record->size);
						val.set_value(vlhex.c_str());
						break;
					default: break;
				}
			}
		}
	}
}

void utf_tool_acbcmd_to_xml(ACB_COMMAND* cmd, pugi::xml_node* node)
{
	pugi::xml_node acb = node->append_child("ACBCMD");

	for(uint32_t i = 0; i != cmd->opcodes_count; ++i)
	{
		ACB_COMMAND_OPCODE* cur_op = &cmd->opcodes[i];
		pugi::xml_node opcode_node = acb.append_child("op");
		opcode_node.append_attribute("opcode").set_value(cur_op->op);

		pugi::xml_attribute type_attr = opcode_node.append_attribute("type");
		pugi::xml_attribute val_attr = opcode_node.append_attribute("value");
		switch(cur_op->type)
		{
			case OPCODE_TYPE_NOVAL:
				type_attr.set_value("noval");
				val_attr.set_value(0);
				break;
			case OPCODE_TYPE_UINT8:
				type_attr.set_value("u8");
				val_attr.set_value(cur_op->data.u8);
				break;
			case OPCODE_TYPE_UINT16:
				type_attr.set_value("u16");
				val_attr.set_value(cur_op->data.u16);
				break;
			case OPCODE_TYPE_UINT32:
				type_attr.set_value("u32");
				val_attr.set_value(cur_op->data.u32);
				break;
			case OPCODE_TYPE_UINT64:
				type_attr.set_value("u64");
				val_attr.set_value(cur_op->data.u64);
				break;
			case OPCODE_TYPE_FLOAT:
				type_attr.set_value("f32");
				val_attr.set_value(cur_op->data.f32);
				break;
			case OPCODE_TYPE_DOUBLE:
				type_attr.set_value("f64");
				val_attr.set_value(cur_op->data.f64);
				break;
			case OPCODE_TYPE_UINT24:
				type_attr.set_value("u24");
				val_attr.set_value(cur_op->data.u24);
				break;
			case OPCODE_TYPE_UINT40:
				type_attr.set_value("u40");
				val_attr.set_value(cur_op->data.u40);
				break;
			case OPCODE_TYPE_UINT48:
				type_attr.set_value("u48");
				val_attr.set_value(cur_op->data.u48);
				break;
			case OPCODE_TYPE_UINT56:
				type_attr.set_value("u56");
				val_attr.set_value(cur_op->data.u56);
				break;
		}
	}
}

/*
	Packer
*/
CRI_UTF_FILE* utf_tool_xml_to_utf(pugi::xml_node* criutf)
{
	CRI_UTF_FILE* utf = cri_utf_alloc_file();

	/* Get relevant nodes */
	pugi::xml_node schema = criutf->child("schema");
	pugi::xml_node rows = criutf->child("rows");

	/* Create FU_FILEs for holding and writing string and data tables */
	FU_FILE* string_table_fu = fu_alloc_file();
	FU_FILE* data_table_fu = fu_alloc_file();
	fu_create_mem_file(string_table_fu);
	fu_create_mem_file(data_table_fu);

	/* Fill out needed utf header values */
	memcpy(&utf->header.magic[0], CRI_UTF_MAGIC, 4);
	utf->header.version = criutf->attribute("version").as_uint();
	utf->header.columns = kwasutils_get_xml_child_count(&schema, "column");
	utf->header.rows = kwasutils_get_xml_child_count(&rows, "row");
	utf->header.name_offset = fu_tell(string_table_fu);

	printf("columns: %u\n", utf->header.columns);
	printf("rows: %u\n", utf->header.rows);

	fu_write_data(string_table_fu,
				  (uint8_t*)criutf->attribute("name").as_string(),
				  strlen(criutf->attribute("name").as_string()));
	fu_write_u8(string_table_fu, 0); /* NULL terminator */

	/* Allocate columns and entries */
	utf->columns = cri_utf_alloc_columns(utf->header.columns);

	uint32_t columns_it = 0;
	for(pugi::xml_node column : schema.children())
	{
		utf->columns[columns_it].records = cri_utf_alloc_records(utf->header.rows);
		utf->columns[columns_it].desc.flag = COLUMN_FLAG_NAME_ROW;
		utf->columns[columns_it].desc.type = cri_utf_str_to_type(column.attribute("type").as_string());

		/* Get the name */
		utf->columns[columns_it].name_offset = fu_tell(string_table_fu);
		fu_write_data(string_table_fu,
					  (uint8_t*)column.attribute("name").as_string(),
					  strlen(column.attribute("name").as_string()));
		fu_write_u8(string_table_fu, 0); /* NULL terminator */

		/* Go to the next column */
		columns_it += 1;
	}

	/* Read record data */
	uint32_t row_it = 0;
	uint32_t records_it = 0;
	for(pugi::xml_node row : rows.children())
	{
		for(pugi::xml_node record : row.children())
		{
			/*printf("Record type: %s %u\n", record.name(), records_it);*/
			CRI_UTF_COLUMN* cur_col = &utf->columns[records_it];
			CRI_UTF_RECORD* cur_row = &cur_col->records[row_it];

			/* Just assign the data */
			if(strncmp(record.name(), "record", 6) == 0)
			{
				uint8_t* vl = 0;

				switch(cur_col->desc.type)
				{
					case COLUMN_TYPE_UINT8:
					case COLUMN_TYPE_SINT8:
						cur_row->data.u8 = record.attribute("value").as_uint();
						break;
					case COLUMN_TYPE_UINT16:
					case COLUMN_TYPE_SINT16:
						cur_row->data.u16 = record.attribute("value").as_uint();
						break;
					case COLUMN_TYPE_UINT32:
					case COLUMN_TYPE_SINT32:
						cur_row->data.u32 = record.attribute("value").as_uint();
						break;
					case COLUMN_TYPE_UINT64:
					case COLUMN_TYPE_SINT64:
						cur_row->data.u64 = record.attribute("value").as_uint();
						break;
					case COLUMN_TYPE_FLOAT:
						cur_row->data.f32 = record.attribute("value").as_float();
						break;
					case COLUMN_TYPE_DOUBLE:
						cur_row->data.f64 = record.attribute("value").as_double();
						break;
					case COLUMN_TYPE_STRING:
						cur_row->offset = fu_tell(string_table_fu);
						cur_row->size = strlen(record.attribute("value").as_string());
						fu_write_data(string_table_fu,
									  (uint8_t*)record.attribute("value").as_string(),
									  cur_row->size);
						fu_write_u8(string_table_fu, 0); /* NULL terminator */
						break;
					case COLUMN_TYPE_VLDATA:
						cur_row->offset = fu_tell(data_table_fu);
						cur_row->size = strlen(record.attribute("value").as_string())/2;
						vl = utf_tool_hex_to_vl((uint8_t*)record.attribute("value").as_string(),
												strlen(record.attribute("value").as_string()));

						/* 
							Without this, VLDATA record that has no size will overlap with 
							the next data
						*/
						if(cur_row->size == 0)
						{
							fu_write_u8(data_table_fu, 0);
						}
						else
						{
							fu_write_data(data_table_fu, vl, cur_row->size);
							free(vl);
						}
						break;
						
					default: break;
				}
			}
			else if(strncmp(record.name(), "CRIUTF", 6) == 0)
			{
				cur_row->utf = utf_tool_xml_to_utf(&record);
				FU_FILE* utf_fu = cri_utf_write_file(cur_row->utf);

				cur_row->offset = fu_tell(data_table_fu);
				cur_row->size = utf_fu->size;
				fu_write_data(data_table_fu, (uint8_t*)utf_fu->buf, utf_fu->size);
				
				//const uint64_t bound_add = bound_calc_leftover(16, fu_tell(data_table_fu));
				//fu_add_to_buf_size(data_table_fu, bound_add);

				fu_close(utf_fu);
				free(utf_fu);
			}
			else if(strncmp(record.name(), "AWB", 3) == 0)
			{
				cur_row->awb = awb_tool_xml_to_afs2(&record);
				FU_FILE* awb_fu = awb_write_file(cur_row->awb);

				cur_row->offset = fu_tell(data_table_fu);
				cur_row->size = awb_fu->size;
				fu_write_data(data_table_fu, (uint8_t*)awb_fu->buf, awb_fu->size);
				
				//const uint64_t bound_add = bound_calc_leftover(16, fu_tell(data_table_fu));
				//fu_add_to_buf_size(data_table_fu, bound_add);

				fu_close(awb_fu);
				free(awb_fu);
			}
			else if(strncmp(record.name(), "ACBCMD", 6) == 0)
			{
				cur_row->acbcmd = utf_tool_xml_to_acbcmd(&record);
				FU_FILE* acb_fu = acb_command_to_fu(cur_row->acbcmd);

				cur_row->offset = fu_tell(data_table_fu);
				cur_row->size = acb_fu->size;
				fu_write_data(data_table_fu, (uint8_t*)acb_fu->buf, acb_fu->size);

				fu_close(acb_fu);
				free(acb_fu);
			}

			records_it += 1;
		}

		records_it = 0;
		row_it += 1;
	}

	/* Allocate and copy memory to utf*/
	utf->string_table = (char*)calloc(1, string_table_fu->size);
	utf->data = (uint8_t*)calloc(1, data_table_fu->size);
	utf->string_table_size = string_table_fu->size;
	utf->data_size = data_table_fu->size;
	memcpy(utf->string_table, string_table_fu->buf, string_table_fu->size);
	memcpy(utf->data, data_table_fu->buf, data_table_fu->size);

	/* Free memory */
	fu_close(string_table_fu);
	fu_close(data_table_fu);

	return utf;
}

ACB_COMMAND* utf_tool_xml_to_acbcmd(pugi::xml_node* acbcmd)
{
	ACB_COMMAND* cmd = acb_command_alloc_command();

	cmd->opcodes_count = kwasutils_get_xml_child_count(acbcmd, "op");
	cmd->opcodes = acb_command_alloc_opcodes(cmd->opcodes_count);

	uint32_t cur_op_id = 0;
	for(pugi::xml_node op : acbcmd->children())
	{
		ACB_COMMAND_OPCODE* opcode = &cmd->opcodes[cur_op_id];
		opcode->op = op.attribute("opcode").as_uint();
		const pugi::char_t* type_str = op.attribute("type").as_string();

		if(strncmp("noval", type_str, 5) == 0)
		{
			opcode->type = OPCODE_TYPE_NOVAL;
			opcode->size = 0;
		}
		else if(strncmp("u8", type_str, 2) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT8;
			opcode->size = 1;
			opcode->data.u8 = op.attribute("value").as_uint();
		}
		else if(strncmp("u16", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT16;
			opcode->size = 2;
			opcode->data.u16 = op.attribute("value").as_uint();
		}
		else if(strncmp("u32", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT32;
			opcode->size = 4;
			opcode->data.u32 = op.attribute("value").as_uint();
		}
		else if(strncmp("u64", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT64;
			opcode->size = 8;
			opcode->data.u64 = op.attribute("value").as_ullong();
		}
		else if(strncmp("f32", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_FLOAT;
			opcode->size = 4;
			opcode->data.f32 = op.attribute("value").as_float();
		}
		else if(strncmp("f64", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_DOUBLE;
			opcode->size = 8;
			opcode->data.f64 = op.attribute("value").as_double();
		}
		else if(strncmp("u24", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT24;
			opcode->size = 3;
			opcode->data.u24 = op.attribute("value").as_uint();
		}
		else if(strncmp("u40", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT40;
			opcode->size = 5;
			opcode->data.u40 = op.attribute("value").as_ullong();
		}
		else if(strncmp("u48", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT48;
			opcode->size = 6;
			opcode->data.u48 = op.attribute("value").as_ullong();
		}
		else if(strncmp("u56", type_str, 3) == 0)
		{
			opcode->type = OPCODE_TYPE_UINT56;
			opcode->size = 7;
			opcode->data.u56 = op.attribute("value").as_ullong();
		}

		cur_op_id += 1;
	}

	acb_command_print(cmd);

	return cmd;
}
