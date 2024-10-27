#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <vector>
#include <set>
#include <algorithm>

#include <pugixml/pugixml.hpp>

#include <kwaslib/kwas_all.h>
#include <kwasutils_pugi_helper.hpp>

/*
	Common
*/
void awb_tool_print_usage(char* exe_path);

/*
	Unpacker
*/
void awb_tool_to_xml(AWB_FILE* afs2, PU_STRING* out, PU_STRING* dir_out);
void awb_tool_afs2_to_xml(AWB_FILE* afs2, pugi::xml_node* node, PU_STRING* dir_name);

/*
	Packer
*/
AWB_FILE* awb_tool_xml_to_afs2(pugi::xml_node* awb);

#ifndef AWB_TOOL_NO_MAIN
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
		PU_PATH input_file_path = {0};
		pu_split_path(argv[1], strlen(argv[1]), &input_file_path);

		/* It's an XML */
		if(strncmp(input_file_path.ext.p, "xml", 3) == 0)
		{
			pugi::xml_document doc;
			doc.load_file(argv[1]);
			pugi::xml_node root = doc.root();
			pugi::xml_node awb = root.child("AWB");
			AWB_FILE* afs2 = awb_tool_xml_to_afs2(&awb);
			FU_FILE* awb_fu = awb_write_file(afs2);

			/* Remove XML extension */
			pu_free_string(&input_file_path.ext);
			PU_STRING awb_out_str = {0};
			pu_path_to_string(&input_file_path, &awb_out_str);

			printf("AWB Path: %s\n", awb_out_str.p);
			fu_buffer_to_file(awb_out_str.p, awb_fu->buf, awb_fu->size, 1);
		}
		else /* Check if the file is a valid AFS2 file */
		{
			FU_FILE awb = {0};
			fu_open_file(argv[1], 1, &awb);
			AWB_FILE* afs2 = awb_read_file(&awb, fu_tell(&awb));
			awb_print(afs2);

			/* Append XML extension */
			pu_insert_char(".xml", 4, -1, &input_file_path.ext);
			PU_STRING xml_out_str = {0};
			pu_path_to_string(&input_file_path, &xml_out_str);

			/* Prepare directory for files */
			pu_free_string(&input_file_path.ext);
			input_file_path.type = PU_PATH_TYPE_DIR;
			PU_STRING dir_out_str = {0};
			pu_path_to_string(&input_file_path, &dir_out_str);

			char offset_str[32] = {0};
			sprintf(offset_str, "_0x%08x", afs2->file_offset);
			pu_insert_char(offset_str, strlen(offset_str), -1, &dir_out_str);

			/* Print the paths */
			printf("XML Path: %s\n", xml_out_str.p);
			printf("Files dir: %s\n", dir_out_str.p);

			/* Save the AWB to xml and extract the contents */
			awb_tool_to_xml(afs2, &xml_out_str, &dir_out_str);
			awb_extract_to_folder(afs2, &dir_out_str);

			/* Close the file */
			fu_close(&awb);
			awb_free(afs2);
			free(afs2);
		}
	}
	else if(pu_is_dir(argv[1])) /* It's a directory, let's create the AWB */
	{
		AWB_FILE* afs2 = awb_parse_directory(argv[1]);
		FU_FILE* awb = awb_write_file(afs2);

		/* Check if there's an appended offset in the name */
		const uint32_t dir_path_len = strlen(argv[1]);
		const uint32_t app_off = dir_path_len - 11; /* _0x00000000 */

		PU_STRING awb_path_str = {0};

		if(strncmp(&argv[1][app_off], "_0x", 3) == 0) /* Remove the suffix */
		{
			pu_create_string(argv[1], app_off, &awb_path_str);
		}
		else
		{
			pu_create_string(argv[1], dir_path_len, &awb_path_str);
		}

		pu_insert_char(".awb", 4, -1, &awb_path_str);

		printf("awb file path: %s\n", awb_path_str.p);

		fu_buffer_to_file(awb_path_str.p, awb->buf, awb->size, 1);

		pu_free_string(&awb_path_str);
		awb_print(afs2);
		awb_free(afs2);
		free(afs2);
		fu_close(awb);
		free(awb);
	}

	return 0;
}

/*
	Common
*/
void awb_tool_print_usage(char* exe_path)
{
	printf("Converts CRIWARE AWB to XML and vice versa.\n");
	printf("Usage:\n");
	printf("\tTo unpack: %s <file.awb>\n", exe_path);
	printf("\tTo pack: %s <file.xml>/<directory>\n", exe_path);
}

#endif

/*
	Unpacker
*/
void awb_tool_to_xml(AWB_FILE* afs2, PU_STRING* xml_out, PU_STRING* dir_name)
{
	pugi::xml_document doc;
	pugi::xml_node root = doc.root();
	awb_tool_afs2_to_xml(afs2, &root, dir_name);
	doc.save_file(xml_out->p);
}

void awb_tool_afs2_to_xml(AWB_FILE* afs2, pugi::xml_node* node, PU_STRING* dir_name)
{
	AWB_HEADER* header = &afs2->header;

	pugi::xml_node awb = node->append_child("AWB");
	awb.append_attribute("version").set_value(header->version);
	awb.append_attribute("alignment").set_value(header->alignment);
	awb.append_attribute("id_align").set_value(header->id_align);
	awb.append_attribute("subkey").set_value(header->subkey);
	
	PU_STRING file_str = {0};
	pu_create_string(dir_name->p, dir_name->s, &file_str);
	
	/* Only create the directory when there's data */
	if(afs2->no_data == 0)
		pu_create_dir_char(file_str.p);
	else
		awb.append_attribute("no_data").set_value(1);
	
	pu_insert_char("/00000.hca", 10, -1, &file_str);
	
	pugi::xml_node files = awb.append_child("files");
	for(uint32_t i = 0; i != header->file_count; ++i)
	{
		pugi::xml_node file = files.append_child("file");
		
		if(afs2->no_data == 0)
		{
			sprintf(&file_str.p[file_str.s-9], "%05u.hca", i);
			fu_buffer_to_file(file_str.p, (char*)afs2->entries[i].data, afs2->entries[i].size, 1);
			
			/* Changed "id" to "seq" (sequence number) because the value is not used at all,
			   it's just for easier look up for order */
			file.append_attribute("seq").set_value(afs2->entries[i].id);
			
			file.append_attribute("path").set_value(file_str.p, file_str.s);
			
			/* No need for size if we're going to read a file anyway */
			/* file.append_attribute("size").set_value(afs2->entries[i].size); */ 
		}
		else /* There is no data */
		{
			file.append_attribute("id").set_value(afs2->entries[i].id);
			file.append_attribute("offset").set_value(afs2->entries[i].offset);

			/* Last offset in AFS2 header is the file size of entire AWB container */
			if(i == (header->file_count-1))
			{
				pugi::xml_node last_file = files.append_child("file");
				last_file.append_attribute("id").set_value(-1);
				last_file.append_attribute("offset").set_value(afs2->entries[header->file_count].offset);
			}
		}
	}

	if(afs2->no_data == 0)
	{
		pugi::xml_node order = awb.append_child("order");
		for(uint32_t i = 0; i != header->file_count; ++i)
		{
			sprintf(&file_str.p[file_str.s-9], "%05u.hca", i);
			fu_buffer_to_file(file_str.p, (char*)afs2->entries[i].data, afs2->entries[i].size, 1);
			order.append_child("entry").append_attribute("id").set_value(afs2->entries[i].id);
		}
	}
}

/*
	Packer
*/
void awb_tool_xml_to_afs2_data(pugi::xml_node* awb, AWB_FILE* afs2)
{
	AWB_HEADER* h = &afs2->header;
	pugi::xml_node files = awb->child("files");
	pugi::xml_node order = awb->child("order");
	
	/*
		"order" node in XML here is the real amount of files in the resulting AWB.
		It also allows for making AWBs smaller, as we can reuse audio files by pointing
		to different offsets.
	*/
	h->file_count = kwasutils_get_xml_child_count(&order, "entry");
	const uint32_t files_size = kwasutils_get_xml_child_count(&files, "file");
	
	/* Read files order */
	std::vector<uint32_t> file_order;
	file_order.reserve(h->file_count);
	
	for(pugi::xml_node entry : order.children())
		file_order.push_back(entry.attribute("id").as_uint());
	
	/* Sort and remove duplicates from order */
	std::vector<uint32_t> file_order_no_dupes(file_order.begin(), file_order.end());
	std::set<uint32_t> file_order_no_dupes_set(file_order.begin(), file_order.end());
	file_order_no_dupes.assign(file_order_no_dupes_set.begin(), file_order_no_dupes_set.end());
	
	/* Load files requested by order */
	AWB_ENTRY* file_entries = awb_alloc_entries(file_order_no_dupes.size());
	
	uint32_t file_entry_it = 0;
	uint32_t order_it = 0;
	for(pugi::xml_node file : files.children())
	{
		if(order_it == file_order_no_dupes.size()) break;
		
		const uint32_t cur_order_id = file_order_no_dupes[order_it];
		
		/* File is in order - add it to the entries */
		if(file_entry_it == cur_order_id)
		{
			file_entries[order_it].id = order_it;
			
			FU_FILE f = {0};
			fu_open_file(file.attribute("path").as_string(), 1, &f);
			file_entries[order_it].size = f.size;
			file_entries[order_it].data = (uint8_t*)calloc(1, f.size);
			memcpy(file_entries[order_it].data, f.buf, f.size);
			fu_close(&f);
		
			order_it += 1;
		}

		file_entry_it += 1;
	}
	
	/* Calculate offsets for the entries used */
	uint32_t first_offset = AWB_ID_OFFSET + h->file_count * (h->offset_size + h->id_align);
	first_offset += h->offset_size; /* For the AWB size which is the last offset */
	//first_offset += bound_calc_leftover(h->alignment, first_offset);
	first_offset += bound_calc_leftover(1024, first_offset);
	
	uint32_t cur_offset = first_offset;

	for(uint32_t i = 0; i != file_order_no_dupes.size(); ++i)
	{	
		file_entries[i].offset = cur_offset;
		cur_offset += file_entries[i].size;
		cur_offset += bound_calc_leftover(h->alignment, cur_offset);
	}
	
	/* Copy the filled out file_entries to afs2 entries */
	afs2->entries = awb_alloc_entries(h->file_count);
	
	for(uint32_t i = 0; i != h->file_count; ++i)
	{
		AWB_ENTRY* cur_file = NULL;
		AWB_ENTRY* cur_entry = &afs2->entries[i];
		
		/* Find file to copy */
		const uint32_t file_order_id = file_order[i];
		
		for(uint32_t j = 0; j != file_order_no_dupes.size(); ++j)
			if(file_order_no_dupes[j] == file_order_id)
				cur_file = &file_entries[j];
		
		/* Copy data */
		cur_entry->id = i;
		cur_entry->size = cur_file->size;
		cur_entry->offset = cur_file->offset;
		
		cur_entry->data = (uint8_t*)calloc(1, cur_entry->size);
		memcpy(cur_entry->data, cur_file->data, cur_entry->size);
	}
}

void awb_tool_xml_to_afs2_no_data(pugi::xml_node* awb, AWB_FILE* afs2)
{
	AWB_HEADER* h = &afs2->header;
	pugi::xml_node files = awb->child("files");
	
	/* Last offset is an AWB file size */
	h->file_count = kwasutils_get_xml_child_count(&files, "file") - 1;
	afs2->entries = awb_alloc_entries(h->file_count + 1);
	
	/* Only loading ids and offsets */
	uint32_t it = 0;
	for(pugi::xml_node file : files.children())
	{
		afs2->entries[it].id = file.attribute("id").as_uint();
		afs2->entries[it].offset = file.attribute("offset").as_uint();
		it += 1;
	}
}

AWB_FILE* awb_tool_xml_to_afs2(pugi::xml_node* awb)
{
	AWB_FILE* afs2 = awb_alloc_file();
	afs2->no_data = awb->attribute("no_data").as_uint();
	
	/* Default header values */
	AWB_HEADER* h = &afs2->header;
	
	memcpy(&afs2->header.magic[0], AWB_MAGIC, 4);
	h->version = awb->attribute("version").as_uint();
	h->offset_size = 4;
	h->id_align = awb->attribute("id_align").as_uint();
	h->alignment = awb->attribute("alignment").as_uint();
	h->subkey = awb->attribute("subkey").as_uint();
	
	/* 
		Read the header depending if we have data or not.
		I hate how offsets to files are not matching with file data.
		Because of that, I need to do this when ACB file stores the AWB's header in `StreamAwbHeader`.
		Otherwise ACB gets "corrupted" and audio doesn't play.
	*/
	if(afs2->no_data)
	{
		awb_tool_xml_to_afs2_no_data(awb, afs2);
	}
	else
	{
		awb_tool_xml_to_afs2_data(awb, afs2);
	}
	
	awb_print(afs2);
		
	return afs2;
}