#include "cvector.h"

#include <stdlib.h>
#include <string.h>

#define CVECTOR_METADATA_SIZE sizeof(CVECTOR_METADATA)

CVECTOR_METADATA* cvec_create(const uint32_t elem_size)
{
	CVECTOR_METADATA* cvec = (CVECTOR_METADATA*)calloc(1, CVECTOR_METADATA_SIZE);
	
	if(cvec)
	{
		cvec->elem_size = elem_size;
	}
	
    return cvec;
}

CVECTOR_METADATA* cvec_destroy(CVECTOR_METADATA* cvec)
{
	free(cvec->data);
	memset(cvec, 0, CVECTOR_METADATA_SIZE);
	return NULL;
}

void cvec_grow(CVECTOR_METADATA* cvec)
{
	uint64_t new_cap = cvec_capacity(cvec);
	
#ifdef CVECTOR_LINEAR_GROWTH
	new_cap += 1;
#else
	if(cvec_empty(cvec))
		new_cap = 1;
	else
		new_cap = (new_cap << 1);
#endif

    const uint64_t to_alloc = new_cap*cvec->elem_size;
    const uint64_t to_copy = cvec->size*cvec->elem_size;
    uint8_t* new_data = (uint8_t*)calloc(to_alloc, 1);
    
    memcpy(new_data, cvec->data, to_copy);
    free(cvec->data);
    
    cvec->data = new_data;
    cvec->capacity = new_cap;
}

/*
 *	Element access
 */

void* cvec_at(CVECTOR_METADATA* cvec, const uint64_t pos)
{
    if(cvec == NULL)
    {
        return NULL;
    }
    
	if(cvec_size(cvec) <= pos)
    {
		return NULL;
    }
	
	return (void*)&cvec->data[cvec->elem_size*pos];
}

void* cvec_front(CVECTOR_METADATA* cvec)
{
	return cvec_at(cvec, 0);
}

void* cvec_back(CVECTOR_METADATA* cvec)
{
	if(cvec_empty(cvec))
		return NULL;
	
	return cvec_at(cvec, cvec_size(cvec)-1);
}

void* cvec_data(CVECTOR_METADATA* cvec)
{
	return (void*)&cvec->data[0];
}

/*
 *	Iterators
 */

void* cvec_begin(CVECTOR_METADATA* cvec)
{
	return cvec_data(cvec);
}

void* cvec_end(CVECTOR_METADATA* cvec)
{
	if(cvec_empty(cvec))
		return cvec_begin(cvec);

	uint8_t* last = (uint8_t*)cvec_back(cvec);
	last += cvec->elem_size;
	return (void*)last;
}

void* cvec_rbegin(CVECTOR_METADATA* cvec)
{
	return cvec_back(cvec);
}

void* cvec_rend(CVECTOR_METADATA* cvec)
{
	if(cvec_empty(cvec))
		return cvec_rbegin(cvec);
	
	uint8_t* last = (uint8_t*)cvec_front(cvec);
	last -= cvec->elem_size;
	return (void*)last;
}

/*
 *	Capacity
 */

const uint8_t cvec_empty(CVECTOR_METADATA* cvec)
{
	if(cvec_size(cvec))
		return 0;
	
	return 1;
}

const uint64_t cvec_size(CVECTOR_METADATA* cvec)
{
	return cvec->size;
}

const uint64_t cvec_max_size(CVECTOR_METADATA* cvec)
{
	return cvec->capacity;
}

void cvec_reserve(CVECTOR_METADATA* cvec, const uint64_t amount)
{
	if(amount > cvec_capacity(cvec))
		cvec_resize(cvec, amount);
}

const uint64_t cvec_capacity(CVECTOR_METADATA* cvec)
{
	return cvec->capacity;
}

void cvec_shrink_to_fit(CVECTOR_METADATA* cvec)
{
	cvec_resize(cvec, cvec_size(cvec));
}

/*
 *	Modifiers
 */
 
void cvec_clear(CVECTOR_METADATA* cvec)
{
	memset(&cvec->data[0], 0, cvec_capacity(cvec)*cvec->elem_size);
	cvec->size = 0;
}

void* cvec_insert(CVECTOR_METADATA* cvec, const uint64_t pos, void* value)
{
	if(cvec_empty(cvec))
		cvec_grow(cvec);
	
	void* data_pos = cvec_at(cvec, pos);	
	
	if(data_pos)
	{
		cvec->size += 1;

		if(cvec_size(cvec) > cvec_capacity(cvec))
		{
			cvec_grow(cvec);
			data_pos = cvec_at(cvec, pos);
		}
		
		for(uint64_t i = cvec_size(cvec)-1; i != pos; --i)
		{
			void* data_1 = cvec_at(cvec, i);
			void* data_2 = cvec_at(cvec, i-1);
			memcpy(data_1, data_2, cvec->elem_size);
		}
		
		memcpy(data_pos, value, cvec->elem_size);
	}
	
	return data_pos;
}

void* cvec_erase(CVECTOR_METADATA* cvec, const uint64_t pos)
{
	void* data_pos = cvec_at(cvec, pos);
	
	if(data_pos == NULL)
		return NULL;
	
	if(data_pos == cvec_back(cvec))
	{
		memset(data_pos, 0, cvec->elem_size);
		cvec->size -= 1;
		return data_pos;
	}

	for(uint64_t i = pos; i != (cvec_size(cvec)-1); ++i)
	{
		void* data_1 = cvec_at(cvec, i);
		void* data_2 = cvec_at(cvec, i+1);
		memcpy(data_1, data_2, cvec->elem_size);
	}
	
	cvec->size -= 1;
	
	return data_pos;
}

void cvec_push_back(CVECTOR_METADATA* cvec, void* value)
{
	if(cvec_empty(cvec))
		cvec_grow(cvec);
    
    cvec->size += 1;
	
	if(cvec_size(cvec) > cvec_capacity(cvec))
		cvec_grow(cvec);

	cvec_assign(cvec, cvec_back(cvec), value);
}

void cvec_pop_back(CVECTOR_METADATA* cvec)
{
	if(cvec_empty(cvec) == 0)
		cvec_erase(cvec, cvec_size(cvec)-1);
}

void cvec_resize(CVECTOR_METADATA* cvec, const uint64_t new_size)
{
    uint8_t* new_data = (uint8_t*)calloc(new_size, cvec->elem_size);
	
	if(new_data)
	{
        uint64_t to_copy = cvec->size;
		cvec->size = new_size;
		cvec->capacity = new_size;
        
        if(cvec->size > new_size) to_copy = new_size;

		memcpy(new_data, cvec->data, to_copy*cvec->elem_size);
		free(cvec->data);
		cvec->data = new_data;
	}
}

/*
 *	Misc
 */

void cvec_swap_elements(CVECTOR_METADATA* cvec, void* f, void* s)
{
	if(!f) return;
	if(!s) return;
	
	uint8_t* buff = (uint8_t*)calloc(1, cvec->elem_size);
	
	if(buff)
	{
		memcpy(buff, f, cvec->elem_size);
		memcpy(f, s, cvec->elem_size);
		memcpy(s, buff, cvec->elem_size);
		
		memset(buff, 0, cvec->elem_size);
		free(buff);
	}
}

void cvec_swap_elements_pos(CVECTOR_METADATA* cvec, const uint64_t f, const uint64_t s)
{
	void* first = cvec_at(cvec, f);
	void* second = cvec_at(cvec, s);
	
	if(first && second)
		cvec_swap_elements(cvec, first, second);
}

void cvec_assign(CVECTOR_METADATA* cvec, void* target, void* value)
{
	if(target && value)
		memcpy(target, value, cvec->elem_size);
}

void cvec_assign_pos(CVECTOR_METADATA* cvec, const uint64_t pos, void* value)
{
	void* target = cvec_at(cvec, pos);
	cvec_assign(cvec, target, value);
}