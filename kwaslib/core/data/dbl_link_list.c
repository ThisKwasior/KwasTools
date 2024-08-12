#include "dbl_link_list.h"

#include <string.h>
#include <stdlib.h>

DBL_LIST_NODE* dbl_list_create_node(const void* const data, const uint64_t size, DBL_LIST_NODE* prev, DBL_LIST_NODE* next)
{
	DBL_LIST_NODE* node = (DBL_LIST_NODE*)calloc(1, sizeof(DBL_LIST_NODE));
	
	node->data = calloc(1, size);
	memcpy(node->data, data, size);
	
	node->prev = prev;
	node->next = next;
	
	return node;
}

const uint64_t dbl_list_count(DBL_LIST_NODE* head)
{
	uint64_t count = 0;
	DBL_LIST_NODE* node = head;
	
	while(node != NULL)
	{
		node = node->next;
		count += 1;
	}
	
	return count;
}

DBL_LIST_NODE* dbl_list_append(DBL_LIST_NODE* head, const void* const data, const uint64_t size)
{
	/* Head doesn't exist. Create it. */
	if(head == NULL)
	{
		DBL_LIST_NODE* node = dbl_list_create_node(data, size, NULL, NULL);
		return node;
	}
	
	DBL_LIST_NODE* node = head;
	
	while(node->next != NULL)
	{
		node = node->next;
	}
	
	node->next = dbl_list_create_node(data, size, node, NULL);
	
	return head;
}

DBL_LIST_NODE* dbl_list_get_node(DBL_LIST_NODE* head, uint64_t pos)
{
	if(head == NULL) return NULL;
	
	DBL_LIST_NODE* node = head;
	
	while(pos--)
	{	
		if(node->next == NULL) 
		{
			return node;
		}
		
		node = node->next;
	}
	
	return node;
}

DBL_LIST_NODE* dbl_list_insert(DBL_LIST_NODE* head, const void* const data, const uint64_t size, uint64_t pos)
{
	if(pos == 0)
	{
		DBL_LIST_NODE* node = dbl_list_create_node(data, size, NULL, head);
		head->prev = node;
		return node;
	}
	
	const uint64_t count = dbl_list_count(head);
	
	if(count <= pos)
	{
		dbl_list_append(head, data, size);
		return head;
	}
	
	DBL_LIST_NODE* node = dbl_list_get_node(head, pos);
	DBL_LIST_NODE* previous = node->prev;
	DBL_LIST_NODE* inserted = dbl_list_create_node(data, size, node->prev, node);
	previous->next = inserted;
	node->prev = inserted;
	
	return head;
}

void dbl_list_replace(const void* const data, const uint64_t size, DBL_LIST_NODE* node)
{
	free(node->data);
	node->data = calloc(1, size);
	memcpy(node->data, data, size);
}

DBL_LIST_NODE* dbl_list_remove_node(DBL_LIST_NODE* head, DBL_LIST_NODE* node)
{
	if(head == NULL) return NULL;
	if(node == NULL) return head;
	
	DBL_LIST_NODE* prev = node->prev;
	DBL_LIST_NODE* next = node->next;
	node->prev = NULL;
	node->next = NULL;
	
	free(node->data);
	node->data = NULL;
	
	free(node);
	node = NULL;
	
	/* First element */
	if(prev == NULL && next)
	{
		next->prev = NULL;
		return next;
	}

	/* Element inbetween */
	if(prev && next)
	{
		prev->next = next;
		next->prev = prev;
	}

	/* Last element */
	if(prev && next == NULL)
	{
		prev->next = NULL;
	}
	
	/* One element left */
	if(prev == NULL && next == NULL)
	{
		return NULL;
	}
	
	return head;
}

DBL_LIST_NODE* dbl_list_free_list(DBL_LIST_NODE* head)
{
	while(head != NULL)
	{
		head = dbl_list_remove_node(head, dbl_list_get_node(head, 0));
	}
}
