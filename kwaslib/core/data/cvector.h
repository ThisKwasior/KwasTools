#pragma once

#include <stdint.h>

//#define CVECTOR_LINEAR_GROWTH

/*
	Loosely based on and inspired by c-vector by eteran.
	https://github.com/eteran/c-vector
*/

typedef struct
{
	uint64_t size; /* Amount of stored elements */
	uint64_t capacity; /* Buffer size */
	uint8_t* data; /* Buffer */
	uint32_t elem_size; /* Size of single element */
} CVECTOR_METADATA;

/*
	Allocates the CVECTOR_METADATA structure.
	elem_size is here to make the overall api less
	dependent on casting and such; data in, data out.
	
	If something fails, returns NULL
*/
CVECTOR_METADATA* cvec_create(const uint32_t elem_size);

/*
	Frees and zeroes everything.
	Elements need to be freed manually if that's needed
	before calling this function.
	
	Returns NULL
*/
CVECTOR_METADATA* cvec_destroy(CVECTOR_METADATA* cvec);

/*
	Grows the data buffer.
	Defaults to logarithmic growth.
	If CVECTOR_LINEAR_GROWTH is defined, it will do size+=1
*/
void cvec_grow(CVECTOR_METADATA* cvec);

/*
 *	Element access
 */
 
void* cvec_at(CVECTOR_METADATA* cvec, const uint64_t pos);
void* cvec_front(CVECTOR_METADATA* cvec);
void* cvec_back(CVECTOR_METADATA* cvec);
void* cvec_data(CVECTOR_METADATA* cvec);

/*
 *	Iterators
 */
 
void* cvec_begin(CVECTOR_METADATA* cvec);
void* cvec_end(CVECTOR_METADATA* cvec);
void* cvec_rbegin(CVECTOR_METADATA* cvec);
void* cvec_rend(CVECTOR_METADATA* cvec);

/*
 *	Capacity
 */
 
const uint8_t cvec_empty(CVECTOR_METADATA* cvec);
const uint64_t cvec_size(CVECTOR_METADATA* cvec);
const uint64_t cvec_max_size(CVECTOR_METADATA* cvec);
void cvec_reserve(CVECTOR_METADATA* cvec, const uint64_t amount);
const uint64_t cvec_capacity(CVECTOR_METADATA* cvec);

/*
	Shrinks the data buffer to the amount of elements.
*/
void cvec_shrink_to_fit(CVECTOR_METADATA* cvec);

/*
 *	Modifiers
 */
 
void cvec_clear(CVECTOR_METADATA* cvec);
void* cvec_insert(CVECTOR_METADATA* cvec, const uint64_t pos, void* value);
void* cvec_erase(CVECTOR_METADATA* cvec, const uint64_t pos);
void cvec_push_back(CVECTOR_METADATA* cvec, void* value);
void cvec_pop_back(CVECTOR_METADATA* cvec);

/*
	Resizes the buffer and copies contents.
	If it can't alloc new buffer, it does nothing.
	
	Can lead to memory leaks as it will copy only the amount
	of elements requested, even if the size is higher.
*/
void cvec_resize(CVECTOR_METADATA* cvec, const uint64_t new_capacity);

/*
 *	Misc
 */

void cvec_swap_elements(CVECTOR_METADATA* cvec, void* f, void* s);
void cvec_swap_elements_pos(CVECTOR_METADATA* cvec, const uint64_t f, const uint64_t s);

void cvec_assign(CVECTOR_METADATA* cvec, void* target, void* value);
void cvec_assign_pos(CVECTOR_METADATA* cvec, const uint64_t pos, void* value);