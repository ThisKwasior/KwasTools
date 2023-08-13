#include "arg_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char* AP_TYPES_STR[] = 
{
	"AP_TYPE_NOV", 
	"AP_TYPE_U8", "AP_TYPE_S8", 
	"AP_TYPE_U16", "AP_TYPE_S16",
	"AP_TYPE_U32", "AP_TYPE_S32", 
	"AP_TYPE_U64", "AP_TYPE_S64",
	"AP_TYPE_FLT", "AP_TYPE_DBL", 
	"AP_TYPE_STR"
};

AP_VALUE_NODE* ap_parse_argv(char** argv, int argc, const AP_ARG_DESC* descs, const uint32_t descs_size)
{
	AP_VALUE_NODE* node = NULL;
	AP_VALUE_NODE* cur_node = NULL;

	/* First argument is the executable run path */
	for(uint32_t it = 1; it != argc; ++it)
	{
		uint8_t arg_found = AP_ERROR;
		
		for(uint32_t it_desc = 0; it_desc < descs_size; ++it_desc)
		{
			const int arg_len = strlen(argv[it]);
			const int desc_len = strlen(descs[it_desc].arg);
			
			if(strncmp(descs[it_desc].arg, argv[it], arg_len) == 0)
			{
				if(arg_len != desc_len)
				{
					/* Different sizes of the strings! Abort!!! */
					it_desc = descs_size;
					continue;
				}
				else
				{
					uint8_t selected_type = AP_TYPE_ILL;
					
					switch(descs[it_desc].type)
					{
						case AP_TYPE_NOV:
							arg_found = AP_SUCCESS;
							selected_type = AP_TYPE_NOV;
							break;
						case AP_TYPE_U8:
						case AP_TYPE_U16:
						case AP_TYPE_U32:
						case AP_TYPE_U64:
						case AP_TYPE_S8:
						case AP_TYPE_S16:
						case AP_TYPE_S32:
						case AP_TYPE_S64:
						case AP_TYPE_FLT:
						case AP_TYPE_DBL:
						case AP_TYPE_STR:
							if(it < (argc-1))
							{
								arg_found = AP_SUCCESS;
								selected_type = descs[it_desc].type;
								it += 1;
							}
							break;
					}
					
					if(arg_found == AP_SUCCESS)
					{
						if(node == NULL)
						{
							node = ap_create_empty_node();
							cur_node = node;
						}
						else
						{
							ap_append_new_elem(node);
							cur_node = cur_node->next;
						}
						
						cur_node->data.desc.type = selected_type;
						memcpy(cur_node->data.desc.arg, descs[it_desc].arg, arg_len);
						
						switch(cur_node->data.desc.type)
						{
							case AP_TYPE_U8:
								cur_node->data.value.u8 = strtoul(argv[it], NULL, 0);
								break;
							case AP_TYPE_U16:
								cur_node->data.value.u16 = strtoul(argv[it], NULL, 0);
								break;
							case AP_TYPE_U32:
								cur_node->data.value.u32 = strtoul(argv[it], NULL, 0);
								break;
							case AP_TYPE_U64:
								cur_node->data.value.u64 = strtoul(argv[it], NULL, 0);
								break;
							case AP_TYPE_S8:
								cur_node->data.value.s8 = strtol(argv[it], NULL, 0);
								break;
							case AP_TYPE_S16:
								cur_node->data.value.s16 = strtol(argv[it], NULL, 0);
								break;
							case AP_TYPE_S32:
								cur_node->data.value.s32 = strtol(argv[it], NULL, 0);
								break;
							case AP_TYPE_S64:
								cur_node->data.value.s64 = strtol(argv[it], NULL, 0);
								break;
							case AP_TYPE_FLT:
								cur_node->data.value.f = strtod(argv[it], NULL);
								break;
							case AP_TYPE_DBL:
								cur_node->data.value.d = strtod(argv[it], NULL);
								break;
							case AP_TYPE_STR:
								cur_node->data.value.str = (char*)calloc(strlen(argv[it]), 1);
								memcpy(cur_node->data.value.str, argv[it], strlen(argv[it]));
								break;
						}
					}
				}
			}
		}
	}
	
	return node;
}

AP_VALUE_NODE* ap_create_empty_node()
{
	return (AP_VALUE_NODE*)calloc(1, sizeof(AP_VALUE_NODE));
}

AP_VALUE_NODE* ap_get_node_by_arg(AP_VALUE_NODE* node, const char* arg)
{
	if(node == NULL) return NULL;
	
	const uint32_t arg_size = strlen(arg);
	AP_VALUE_NODE* cur_elem = node;
	
	while(1)
	{
		if(cur_elem != NULL)
		{
			const uint32_t cur_arg_size = strlen(cur_elem->data.desc.arg);
			
			if(cur_arg_size == arg_size)
			{
				if(strncmp(arg, cur_elem->data.desc.arg, arg_size) == 0)
				{
					return cur_elem;
				}
			}
			
			cur_elem = cur_elem->next;
		}
		else
		{
			return NULL;
		}
	}
	
	return NULL;
}

void ap_append_new_elem(AP_VALUE_NODE* node)
{
	AP_VALUE_NODE* new_elem = ap_create_empty_node();
	AP_VALUE_NODE* cur_elem = node;

	while(1)
	{
		if(cur_elem->next == NULL)
		{
			cur_elem->next = new_elem;
			return;
		}
		else
		{
			cur_elem = cur_elem->next;
		}
	}
}


void ap_free_node(AP_VALUE_NODE* node)
{
	AP_VALUE_NODE* cur_node = node;
	
	while(cur_node != NULL)
	{
		AP_VALUE_NODE* next_node = cur_node->next;
		
		if(cur_node->data.desc.type == AP_TYPE_STR)
		{
			free(cur_node->data.value.str);
		}
		
		free(cur_node);
		
		cur_node = next_node;
	}
}

void ap_print_node(AP_VALUE_NODE* node)
{
	AP_VALUE_NODE* elem = node;
	uint32_t it = 0;
	
	while(elem != NULL)
	{
		switch(elem->data.desc.type)
		{
			case AP_TYPE_NOV:
				printf("Node [%04u][%s]\n", it, ap_type_str(elem->data.desc.type));
				break;
			case AP_TYPE_U8: 
				printf("Node [%04u][%s]: %u\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s8);
				break;
			case AP_TYPE_U16: 
				printf("Node [%04u][%s]: %u\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s16);
				break;
			case AP_TYPE_U32: 
				printf("Node [%04u][%s]: %u\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s32);
				break;
			case AP_TYPE_U64: 
				printf("Node [%04u][%s]: %llu\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s64);
				break;
			case AP_TYPE_S8: 
				printf("Node [%04u][%s]: %d\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s8);
				break;
			case AP_TYPE_S16: 
				printf("Node [%04u][%s]: %d\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s16);
				break;
			case AP_TYPE_S32: 
				printf("Node [%04u][%s]: %d\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s32);
				break;
			case AP_TYPE_S64: 
				printf("Node [%04u][%s]: %lld\n", it, ap_type_str(elem->data.desc.type), elem->data.value.s64);
				break;
			case AP_TYPE_FLT: 
				printf("Node [%04u][%s]: %f\n", it, ap_type_str(elem->data.desc.type), elem->data.value.f);
				break;
			case AP_TYPE_DBL: 
				printf("Node [%04u][%s]: %lf\n", it, ap_type_str(elem->data.desc.type), elem->data.value.d);
				break;
			case AP_TYPE_STR: 
				printf("Node [%04u][%s]: %s\n", it, ap_type_str(elem->data.desc.type), elem->data.value.str);
				break;
		}
		
		elem = elem->next;
		it += 1;
	}
}

const char* ap_type_str(const uint8_t type)
{
	return AP_TYPES_STR[type];
}