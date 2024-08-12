#pragma once

#include <stdint.h>

/* Structures */

typedef struct DBL_LINK_LIST_NODE
{
	void* data;
	struct DBL_LINK_LIST_NODE* prev;
	struct DBL_LINK_LIST_NODE* next;
} DBL_LIST_NODE;

/* Functions */

DBL_LIST_NODE* dbl_list_create_node(const void* const data, const uint64_t size, DBL_LIST_NODE* prev, DBL_LIST_NODE* next);

const uint64_t dbl_list_count(DBL_LIST_NODE* head);

DBL_LIST_NODE* dbl_list_append(DBL_LIST_NODE* head, const void* const data, const uint64_t size);

DBL_LIST_NODE* dbl_list_insert(DBL_LIST_NODE* head, const void* const data, const uint64_t size, uint64_t pos);

DBL_LIST_NODE* dbl_list_get_node(DBL_LIST_NODE* head, uint64_t pos);

void dbl_list_replace(const void* const data, const uint64_t size, DBL_LIST_NODE* node);

DBL_LIST_NODE* dbl_list_remove_node(DBL_LIST_NODE* head, DBL_LIST_NODE* node);

DBL_LIST_NODE* dbl_list_free_list(DBL_LIST_NODE* head);