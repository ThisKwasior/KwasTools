#include "utf.h"

#include <string.h>
#include <stdlib.h>

static const char* CRI_UTF_FLAG_STR[] = 
{
	"NONE (?)", "NAME", "DEFAULT", "NAME|DEFAULT",
	"ROW", "NAME|ROW", "ROW|DEFAULT (?)", "NAME|DEFAULT|ROW (?)",
	"UNDEFINED"
};

#define CRI_UTF_FLAG_STR_SIZE	8

static const char* CRI_UTF_TYPE_STR[] = 
{
	"UINT8", "SINT8", "UINT16", "SINT16",
	"UINT32", "SINT32", "UINT64", "SINT64",
	"FLOAT", "DOUBLE", "STRING", "VLDATA",
	"UINT128", "UNDEFINED"
};

#define CRI_UTF_TYPE_STR_SIZE	13

static const char* CRI_UTF_STREAMING_STR[] = 
{
	"Memory", "Streaming", "Memory (Prefetch) + Stream"
};

/* Reading */

CRI_UTF_FILE* cri_utf_read_file(FU_FILE* cri)
{
	CRI_UTF_FILE* utf = cri_utf_alloc_file();
	
	cri_utf_read_header(cri, utf);
	
	if(strncmp(utf->header.magic, "@UTF", 4) != 0)
	{
		free(utf);
		return NULL;
	}
	
	cri_utf_calc_sizes(utf);
	cri_utf_read_string_data_table(cri, utf);
	cri_utf_parse_schema(cri, utf);
	cri_utf_read_rows(cri, utf);
	
	return utf;
}

void cri_utf_read_header(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	utf->file_offset = fu_tell(cri);
	
	CRI_UTF_HEADER* header = &utf->header;
	
	uint8_t status = 0;
	uint64_t bytes_read = 0;
	fu_read_data(cri, (uint8_t*)&header->magic[0], 4, &bytes_read);
	header->table_size = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
	header->version = fu_read_u16(cri, &status, FU_BIG_ENDIAN);
	header->rows_offset = fu_read_u16(cri, &status, FU_BIG_ENDIAN);
	header->string_table_offset = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
	header->data_offset = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
	header->name_offset = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
	header->columns = fu_read_u16(cri, &status, FU_BIG_ENDIAN);
	header->row_width = fu_read_u16(cri, &status, FU_BIG_ENDIAN);
	header->rows = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
}

void cri_utf_calc_sizes(CRI_UTF_FILE* utf)
{
	CRI_UTF_HEADER* h = &utf->header;
	utf->rows_size = h->rows * h->row_width;
	utf->string_table_size = h->data_offset - h->string_table_offset;
	utf->data_size = h->table_size - h->data_offset;
}

void cri_utf_read_string_data_table(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	/* Allocate and read the string table and data for later */
	uint64_t bytes_read = 0;
	
	utf->string_table = (char*)calloc(utf->string_table_size, 1);
	fu_seek(cri, utf->file_offset + utf->header.string_table_offset + 8, FU_SEEK_SET);
	fu_read_data(cri, (uint8_t*)utf->string_table, utf->string_table_size, &bytes_read);
	
	utf->data = (uint8_t*)calloc(utf->data_size, 1);
	fu_seek(cri, utf->file_offset + utf->header.data_offset + 8, FU_SEEK_SET);
	fu_read_data(cri, utf->data, utf->data_size, &bytes_read);
	
	/* Also get the name for the header */
	utf->header.name = (char*)&utf->string_table[utf->header.name_offset];
}

void cri_utf_parse_schema(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	CRI_UTF_HEADER* h = &utf->header;
	utf->columns = cri_utf_alloc_columns(h->columns);
	
	fu_seek(cri, utf->file_offset + CRI_UTF_SCHEMA_OFFSET, FU_SEEK_SET);
	
	for(uint32_t i = 0; i != h->columns; ++i)
	{
		uint8_t status = 0;
		uint64_t bytes_read = 0;
		
		CRI_UTF_COLUMN* cur_col = &utf->columns[i];
		
		fu_read_data(cri, (uint8_t*)&cur_col->desc, 1, &bytes_read);
								
		cur_col->name_offset = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
		cur_col->name = &utf->string_table[cur_col->name_offset];

		/* Alloc records */
		cur_col->records = cri_utf_alloc_records(utf->header.rows);

		/* Data for all rows in this column is in the schema */
		/* Read and assign to all rows */
		if(cur_col->desc.flag == COLUMN_FLAG_NAME_DEFAULT)
		{
			CRI_UTF_RECORD record = cri_utf_read_by_type(cri, utf, cur_col);
			
			for(uint32_t j = 0; j != h->rows; ++j)
			{
				cur_col->records[j] = record;
			}
		}
	}
}

void cri_utf_read_rows(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	CRI_UTF_HEADER* h = &utf->header;
	fu_seek(cri, utf->file_offset + h->rows_offset + 8, FU_SEEK_SET);
	
	for(uint32_t i = 0; i != h->rows; ++i)
	{
		for(uint32_t j = 0; j != h->columns; ++j)
		{
			CRI_UTF_COLUMN* cur_col = &utf->columns[j];
			
			if(cur_col->desc.flag == COLUMN_FLAG_NAME_ROW)
			{
				cur_col->records[i] = cri_utf_read_by_type(cri, utf, cur_col);
			}
		}
	}
}

CRI_UTF_RECORD cri_utf_read_by_type(FU_FILE* cri, CRI_UTF_FILE* utf, CRI_UTF_COLUMN* col)
{
	CRI_UTF_RECORD record = {0};
	uint8_t status = 0;

	switch(col->desc.type)
	{
		case COLUMN_TYPE_UINT8:
		case COLUMN_TYPE_SINT8:
			record.data.u8 = fu_read_u8(cri, &status);
			break;
		case COLUMN_TYPE_UINT16:
		case COLUMN_TYPE_SINT16:
			record.data.u16 = fu_read_u16(cri, &status, FU_BIG_ENDIAN);
			break;
		case COLUMN_TYPE_UINT32:
		case COLUMN_TYPE_SINT32:
			record.data.u32 = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
			break;
		case COLUMN_TYPE_UINT64:
		case COLUMN_TYPE_SINT64:
			record.data.u64 = fu_read_u64(cri, &status, FU_BIG_ENDIAN);
			break;
		case COLUMN_TYPE_FLOAT:
			record.data.f32 = fu_read_f32(cri, &status, FU_BIG_ENDIAN);
			break;
		case COLUMN_TYPE_DOUBLE:
			record.data.f64 = fu_read_f64(cri, &status, FU_BIG_ENDIAN);
			break;
		case COLUMN_TYPE_STRING:
			record.offset = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
			record.data.str = &utf->string_table[record.offset];
			record.size = strlen(record.data.str);
			/*printf("%s\n", record.data.str);*/
			break;
		case COLUMN_TYPE_VLDATA:
			record.offset = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
			record.size = fu_read_u32(cri, &status, FU_BIG_ENDIAN);
			record.data.vl = &utf->data[record.offset];
			
			/* Load embedded UTF, AWB (AFS2) or ACB Command */
			const uint64_t cur_pos_rows = fu_tell(cri);
			
			fu_seek(cri, utf->file_offset + utf->header.data_offset + 8 + record.offset, FU_SEEK_SET);
			
			if(strncmp((char*)record.data.vl, CRI_UTF_MAGIC, 4) == 0)
			{
				printf("Found UTF at 0x%llx\n", fu_tell(cri));
				record.utf = cri_utf_read_file(cri);
			}
			else if(strncmp((char*)record.data.vl, AWB_MAGIC, 4) == 0)
			{
				printf("Found AWB at 0x%llx\n", fu_tell(cri));
				FU_FILE* internal_awb = fu_create_mem_file_data(record.data.vl, record.size);
				record.awb = awb_read_file(internal_awb, fu_tell(cri));
				awb_print(record.awb);
				fu_close(internal_awb);
				free(internal_awb);
			}
			else if(strncmp("Command", col->name, 7) == 0)
			{
				printf("Found ACB Command at 0x%llx\n", fu_tell(cri));
				record.acbcmd = acb_command_parse_data(record.data.vl, record.size);
			}
			
			fu_seek(cri, cur_pos_rows, FU_SEEK_SET);
			
			break;
			
		default: break;
	}
	
	return record;
}

/* Writing */
FU_FILE* cri_utf_write_file(CRI_UTF_FILE* utf)
{
	FU_FILE* cri = fu_alloc_file();
	
	fu_create_mem_file(cri);
	
	cri_utf_write_header(cri, utf);
	cri_utf_write_schema(cri, utf);
	cri_utf_write_rows(cri, utf);
	
	fu_write_data(cri, (uint8_t*)utf->string_table, utf->string_table_size);
	fu_write_data(cri, utf->data, utf->data_size);
	
	return cri;
}

void cri_utf_write_header(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	cri_utf_recalc_for_write(utf);
	
	CRI_UTF_HEADER* header = &utf->header;
	
	fu_write_data(cri, (uint8_t*)header->magic, 4);
	fu_write_u32(cri, header->table_size, FU_BIG_ENDIAN);
	fu_write_u16(cri, header->version, FU_BIG_ENDIAN);
	fu_write_u16(cri, header->rows_offset, FU_BIG_ENDIAN);
	fu_write_u32(cri, header->string_table_offset, FU_BIG_ENDIAN);
	fu_write_u32(cri, header->data_offset, FU_BIG_ENDIAN);
	fu_write_u32(cri, header->name_offset, FU_BIG_ENDIAN);
	fu_write_u16(cri, header->columns, FU_BIG_ENDIAN);
	fu_write_u16(cri, header->row_width, FU_BIG_ENDIAN);
	fu_write_u32(cri, header->rows, FU_BIG_ENDIAN);
}

void cri_utf_write_schema(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	CRI_UTF_HEADER* header = &utf->header;
	
	for(uint32_t i = 0; i != header->columns; ++i)
	{
		fu_write_data(cri, (uint8_t*)&utf->columns[i].desc, 1);
		fu_write_u32(cri, utf->columns[i].name_offset, FU_BIG_ENDIAN);
	}
}

void cri_utf_write_rows(FU_FILE* cri, CRI_UTF_FILE* utf)
{
	CRI_UTF_HEADER* h = &utf->header;
	
	for(uint32_t i = 0; i != h->rows; ++i)
	{
		for(uint32_t j = 0; j != h->columns; ++j)
		{
			CRI_UTF_COLUMN* cur_col = &utf->columns[j];
			CRI_UTF_RECORD* cur_row = &cur_col->records[i];
			
			switch(cur_col->desc.type)
			{
				case COLUMN_TYPE_UINT8:
				case COLUMN_TYPE_SINT8:
					fu_write_u8(cri, cur_row->data.u8);
					break;
				case COLUMN_TYPE_UINT16:
				case COLUMN_TYPE_SINT16:
					fu_write_u16(cri, cur_row->data.u16, FU_BIG_ENDIAN);
					break;
				case COLUMN_TYPE_UINT32:
				case COLUMN_TYPE_SINT32:
					fu_write_u32(cri, cur_row->data.u32, FU_BIG_ENDIAN);
					break;
				case COLUMN_TYPE_UINT64:
				case COLUMN_TYPE_SINT64:
					fu_write_u64(cri, cur_row->data.u64, FU_BIG_ENDIAN);
					break;
				case COLUMN_TYPE_FLOAT:
					fu_write_f32(cri, cur_row->data.f32, FU_BIG_ENDIAN);
					break;
				case COLUMN_TYPE_DOUBLE:
					fu_write_f64(cri, cur_row->data.f64, FU_BIG_ENDIAN);
					break;
				case COLUMN_TYPE_STRING:
					fu_write_u32(cri, cur_row->offset, FU_BIG_ENDIAN);
					break;
				case COLUMN_TYPE_VLDATA:
					fu_write_u32(cri, cur_row->offset, FU_BIG_ENDIAN);
					fu_write_u32(cri, cur_row->size, FU_BIG_ENDIAN);
					break;
					
				default: break;
			}
		}
	}
}

void cri_utf_recalc_for_write(CRI_UTF_FILE* utf)
{
	/* 5 because of 1 byte flag/type and uint32_t for name offset */
	const uint32_t schema_size = utf->header.columns * 5;
	
	utf->header.rows_offset = (CRI_UTF_SCHEMA_OFFSET - 8) + schema_size;
	utf->header.row_width = cri_utf_calc_row_width(utf);
	
	const uint32_t rows_size = utf->header.row_width * utf->header.rows;
	utf->header.string_table_offset = utf->header.rows_offset + rows_size;
	utf->header.data_offset = utf->header.string_table_offset + utf->string_table_size;
	utf->header.table_size = utf->header.data_offset + utf->data_size;
}

/* Other */

uint32_t cri_utf_calc_row_width(CRI_UTF_FILE* utf)
{
	uint32_t row_width = 0;
	
	for(uint32_t i = 0; i != utf->header.columns; ++i)
	{
		switch(utf->columns[i].desc.type)
		{
			case COLUMN_TYPE_UINT8:
			case COLUMN_TYPE_SINT8:
				row_width += 1;
				break;
			case COLUMN_TYPE_UINT16:
			case COLUMN_TYPE_SINT16:
				row_width += 2;
				break;
			case COLUMN_TYPE_UINT32:
			case COLUMN_TYPE_SINT32:
			case COLUMN_TYPE_FLOAT:
			case COLUMN_TYPE_STRING:
				row_width += 4;
				break;
			case COLUMN_TYPE_UINT64:
			case COLUMN_TYPE_SINT64:
			case COLUMN_TYPE_DOUBLE:
			case COLUMN_TYPE_VLDATA:
				row_width += 8;
				break;
				
			default: break;
		}
	}

	return row_width;
}

const char* cri_utf_flag_to_str(const CRI_UTF_FLAG flag)
{
	return CRI_UTF_FLAG_STR[flag];
}

const char* cri_utf_type_to_str(const CRI_UTF_TYPE type)
{
	return CRI_UTF_TYPE_STR[type];
}

const char* cri_utf_streaming_to_str(const uint8_t streaming)
{
	return CRI_UTF_STREAMING_STR[streaming];
}

const CRI_UTF_FLAG cri_utf_str_to_flag(const char* str)
{
	CRI_UTF_FLAG flag = CRI_UTF_FLAG_STR_SIZE;

	for(uint8_t i = 0; i != CRI_UTF_FLAG_STR_SIZE; ++i)
	{
		const uint32_t flag_len = strlen(CRI_UTF_FLAG_STR[i]);
		if(strncmp(str, CRI_UTF_FLAG_STR[i], flag_len) == 0)
		{
			flag = i;
			return flag;
		}
	}

	return flag;
}

const CRI_UTF_TYPE cri_utf_str_to_type(const char* str)
{
	CRI_UTF_TYPE type = CRI_UTF_TYPE_STR_SIZE;

	for(uint8_t i = 0; i != CRI_UTF_TYPE_STR_SIZE; ++i)
	{
		const uint32_t type_len = strlen(CRI_UTF_TYPE_STR[i]);
		if(strncmp(str, CRI_UTF_TYPE_STR[i], type_len) == 0)
		{
			type = i;
			return type;
		}
	}

	return type;
}

void cri_utf_free(CRI_UTF_FILE* utf)
{
	utf->rows_size = 0;
	utf->string_table_size = 0;
	utf->data_size = 0;
	if(utf->string_table) free(utf->string_table);
	if(utf->data) free(utf->data);
	
	/* columns */
	for(uint32_t i = 0; i != utf->header.columns; ++i)
	{
		CRI_UTF_COLUMN* col = &utf->columns[i];
		
		if(col->desc.type == COLUMN_TYPE_VLDATA)
		{
			for(uint32_t j = 0; j != utf->header.rows; ++j)
			{
				if(col->records[j].utf)
				{
					cri_utf_free(col->records[j].utf);
				}
			}
		}
		
		free(col->records);
		col->records = NULL;
	}
	
	free(utf->columns);
	utf->columns = NULL;
}

CRI_UTF_FILE* cri_utf_alloc_file()
{
	return (CRI_UTF_FILE*)calloc(1, sizeof(CRI_UTF_FILE));
}

CRI_UTF_COLUMN* cri_utf_alloc_columns(const uint32_t column_count)
{
	return (CRI_UTF_COLUMN*)calloc(column_count, sizeof(CRI_UTF_COLUMN));
}

CRI_UTF_RECORD* cri_utf_alloc_records(const uint32_t record_count)
{
	return (CRI_UTF_RECORD*)calloc(record_count, sizeof(CRI_UTF_RECORD));
}

void cri_utf_print(CRI_UTF_FILE* utf)
{
	printf("table_size: %u\n", utf->header.table_size);
	printf("version: %u\n", utf->header.version);
	printf("rows_offset: %x\n", utf->header.rows_offset);
	printf("string_table_offset: %x\n", utf->header.string_table_offset);
	printf("data_offset: %x\n", utf->header.data_offset);
	printf("name_offset: %x\n", utf->header.name_offset);
	printf("columns: %u\n", utf->header.columns);
	printf("row_width: %u\n", utf->header.row_width);
	printf("rows: %u\n", utf->header.rows);
	
	printf("rows_size: %x\n", utf->rows_size);
	printf("string_table_size: %x\n", utf->string_table_size);
	printf("data_size: %x\n", utf->data_size);
	
	printf("header name: %s\n", utf->header.name);

	for(uint32_t i = 0; i != utf->header.columns; ++i)
	{
		CRI_UTF_COLUMN* cur_col = &utf->columns[i];
		printf("[%u] (%x %x) %s %s | %s\n", i, cur_col->desc.flag, cur_col->desc.type,
										  cri_utf_flag_to_str(cur_col->desc.flag),
										  cri_utf_type_to_str(cur_col->desc.type),
										  cur_col->name);
	}

	for(uint32_t i = 0; i != utf->header.columns; ++i)
	{
		printf("%s;", utf->columns[i].name);
	}
	printf("\n");
	
	for(uint32_t j = 0; j != utf->header.rows; ++j)
	{
		for(uint32_t i = 0; i != utf->header.columns; ++i)
		{
			switch(utf->columns[i].desc.type)
			{
				case COLUMN_TYPE_UINT8:
				case COLUMN_TYPE_SINT8:
				case COLUMN_TYPE_UINT16:
				case COLUMN_TYPE_SINT16:
				case COLUMN_TYPE_UINT32:
				case COLUMN_TYPE_SINT32:
				case COLUMN_TYPE_UINT64:
				case COLUMN_TYPE_SINT64:
					printf("%llu;", utf->columns[i].records[j].data.u64);
					break;
				case COLUMN_TYPE_FLOAT:
					printf("%f;", utf->columns[i].records[j].data.f32);
					break;
				case COLUMN_TYPE_DOUBLE:
					printf("%lf;", utf->columns[i].records[j].data.f64);
					break;
				case COLUMN_TYPE_STRING:
					printf("%s;", utf->columns[i].records[j].data.str);
					break;
				case COLUMN_TYPE_VLDATA:
					for(uint32_t k = 0; k != 8; ++k)
					{
						const char c = utf->columns[i].records[j].data.vl[k];
						if(c >= 32 && c <= 126) printf("%c", c);
						else printf(".");
					}
					printf(";");
					break;
				
				default: break;
			}
		}
		printf("\n");
	}
}