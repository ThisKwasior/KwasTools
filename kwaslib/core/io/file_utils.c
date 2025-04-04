#include <stdlib.h>
#include <string.h>

#include "file_utils.h"

#include "type_readers.h"
#include "type_writers.h"

static const char* FU_STATUS_STR[] = 
{
	"FU_SUCCESS", "FU_ERROR", "FU_EOF", "FU_ENDDATA",
	"FU_FEXISTS", "FU_NOTMEMF", "FU_REQ0", "FU_REQBEL0"
};

uint8_t fu_open_file(const char* path, const uint8_t to_memory, FU_FILE* f)
{
	fu_close(f);
	
	if(pu_is_file(path))
	{
		f->size = fu_get_file_size(path);
		f->rem = f->size;
		f->f = fopen(path, "rb");
		
		if(f->f)
		{
			if(to_memory)
			{				
				f->buf = (char*)calloc(1, f->size);
				
				if(f->buf)
				{					
					if(fread(f->buf, f->size, 1, f->f) == 0)
					{
						fclose(f->f);
						return FU_ERROR;
					}
					
					f->is_buf = 1;
					f->do_free = 1;
					f->writeable = 0;
					
					fclose(f->f);
				}
				else
				{
					fclose(f->f);
					return FU_ERROR;
				}
			}
			
			return FU_SUCCESS;
		}
	}
	
	return FU_ERROR;
}

uint8_t fu_open_file_pu(PU_PATH* path, const uint8_t to_memory, FU_FILE* f)
{
	SU_STRING* str = pu_path_to_string(path);
	
	const uint8_t retval = fu_open_file((const char*)str->ptr, to_memory, f);
	
	su_free(str);
	
	return retval;
}

FU_FILE* fu_alloc_file()
{
	return (FU_FILE*)calloc(1, sizeof(FU_FILE));
}

uint64_t fu_get_file_size(const char* path)
{
	FILE* f = fopen(path, "rb");
	
	if(f)
	{
		fseek(f, 0, SEEK_END);

#if defined(__WIN32__) || defined(__MINGW32__)
		const uint64_t len = ftello64(f);
#else
		const uint64_t len = ftello(f);
#endif
		fclose(f);
		return len;
	}
	
	/* Couldn't get the size */
	return 0;
}

uint64_t fu_get_file_size_pu(PU_PATH* path)
{
	SU_STRING* str = pu_path_to_string(path);

	const uint64_t size = fu_get_file_size((const char*)str->ptr);

	su_free(str);
	
	return size;
}

uint8_t fu_seek(FU_FILE* f, int64_t offset, uint8_t whence)
{
	if(f->is_buf == 0)
	{
		const int fseekret = fseek(f->f, offset, whence);
		
		if(fseekret != 0) 
			return FU_ERROR;
	}
	
	int64_t buf_pos = 0;
	
	switch(whence)
	{
		case FU_SEEK_SET:
			buf_pos += offset;
			break;
		case FU_SEEK_CUR:
			buf_pos = offset + f->pos;
			break;
		case FU_SEEK_END:
			buf_pos = offset + f->size;
			break;
	}

	if(buf_pos < 0) buf_pos = 0;
	if(buf_pos >= f->size) buf_pos = f->size;

	f->pos = buf_pos;
	f->rem = f->size - f->pos;
	
	return FU_SUCCESS;
}

uint64_t fu_tell(FU_FILE* f)
{
	return f->pos;
}

void fu_close(FU_FILE* f)
{
	if(f->is_buf)
	{
		if(f->do_free)
		{
			free(f->buf);
		}
	}
	else
	{
		if(f->f)
		{
			fclose(f->f);
		}
	}

	f->buf = NULL;
	f->f = NULL;

	f->is_buf = 0;
	f->do_free = 0;
	f->writeable = 0;

	f->size = 0;
	f->rem = 0;
	f->pos = 0;
}

const char* fu_status_str(const uint8_t status)
{
	return FU_STATUS_STR[status];
}

/* 
	Memory file
*/
uint8_t fu_create_mem_file(FU_FILE* f)
{
	char* p = (char*)calloc(1, 1);
	
	if(p)
	{
		f->buf = p;
		
		f->is_buf = 1;
		f->do_free = 1;
		f->writeable = 1;
		
		f->size = 1;
		f->pos = 0;
		f->rem = 1;
	}
	else
	{
		return FU_ERROR;
	}
	
	return FU_SUCCESS;
}

uint8_t fu_change_buf_size(FU_FILE* f, const int64_t desired_size)
{
	if(f->writeable == 0) return FU_NOTMEMF;
	if(desired_size == 0) return FU_REQ0;
	if(desired_size < 0) return FU_REQBEL0;
	
	char* p = (char*)calloc(desired_size, 1);
	
	if(p == NULL)
	{
		return FU_ERROR;
	}
	
	if(desired_size < f->size)
	{
		memcpy(p, f->buf, desired_size);
	}
	else
	{
		memcpy(p, f->buf, f->size);
	}
	
	f->size = desired_size;
	
	if(f->pos > f->size)
	{
		f->pos = f->size;
	}
	
	f->rem = f->size - f->pos;
	
	if(f->do_free)
		free(f->buf);
	
	f->buf = p;
	
	return FU_SUCCESS;
}

uint8_t fu_add_to_buf_size(FU_FILE* f, const int64_t bytes_req)
{
	return fu_change_buf_size(f, f->size + bytes_req);
}

uint8_t fu_check_buf_rem(FU_FILE* f, const uint64_t bytes_req)
{
	if(f->writeable == 0)
	{
		return FU_NOTMEMF;
	}
	
	if(f->rem >= bytes_req)
	{
		return FU_SUCCESS;
	}
	
	const int64_t to_add = bytes_req - f->rem;
	
	return fu_add_to_buf_size(f, to_add);
}

FU_FILE* fu_create_mem_file_data(const uint8_t* data, const uint64_t size)
{
	FU_FILE* mem_fu = fu_alloc_file();
	fu_create_mem_file(mem_fu);
	fu_write_data(mem_fu, data, (uint64_t)size);
	fu_seek(mem_fu, 0, FU_SEEK_SET);
	return mem_fu;
}

/*
	Saving
*/
uint8_t fu_to_file(const char* path, FU_FILE* f, const uint8_t overwrite)
{
	return fu_buffer_to_file(path, f->buf, f->size, overwrite);
}

uint8_t fu_to_file_pu(PU_PATH* path, FU_FILE* f, const uint8_t overwrite)
{
	return fu_buffer_to_file_pu(path, f->buf, f->size, overwrite);
}

uint8_t fu_buffer_to_file(const char* path, const char* buffer, const uint64_t size, const uint8_t overwrite)
{
	if(pu_is_dir(path))
		return FU_ERROR;
	
	if(pu_is_file(path))
	{
		if(overwrite == 0)
		{
			return FU_FEXISTS;
		}
	}
	
	FILE* f = fopen(path, "wb");
	
	if(f)
	{
		fwrite(buffer, size, 1, f);
		fclose(f);
	}
	else
	{
		return FU_ERROR;
	}
	
	return FU_SUCCESS;
}

uint8_t fu_buffer_to_file_pu(PU_PATH* path, const char* buffer, const uint64_t size, const uint8_t overwrite)
{
	SU_STRING* str = pu_path_to_string(path);
	
	const uint8_t retval = fu_buffer_to_file((const char*)str->ptr, buffer, size, overwrite);
	
	su_free(str);
	
	return retval;
}

/*
	Readers
*/

uint8_t fu_check_read_req(FU_FILE* f, const uint32_t req)
{
	if(f->rem < req)
	{
		return FU_ERROR;
	}

	return FU_SUCCESS;
}

uint8_t fu_read_data(FU_FILE* f, uint8_t* buf, const uint64_t size, uint64_t* read)
{
	if(f->rem == 0)
	{
		if(read != NULL) *read = 0;
		return FU_EOF;
	}
	
	uint8_t status = FU_SUCCESS;
	uint64_t to_read = size;
	
	/* EOF, read everything we can */
	if(size >= f->rem)
	{
		to_read = f->rem;
		status = FU_ENDDATA;
	}
	
	if(f->is_buf)
	{
		memcpy(buf, &f->buf[f->pos], to_read);
	}
	else
	{
		fread(buf, to_read, 1, f->f);
	}
	
	if(read != NULL) *read = to_read;
	
	f->pos += to_read;
	f->rem = f->size - f->pos;
	
	return status;
}

uint8_t fu_read_u8(FU_FILE* f, uint8_t* status)
{
	uint8_t buf = 0;

	if(fu_check_read_req(f, 1) == FU_SUCCESS)
	{
		const uint8_t stat = fu_read_data(f, (uint8_t*)&buf, 1, NULL);
		if(status != NULL) *status = stat;
	}
	else
	{
		if(status != NULL) *status = FU_ERROR;
	}
	
	return buf;
}

uint16_t fu_read_u16(FU_FILE* f, uint8_t* status, const uint8_t endian)
{
	uint16_t buf = 0;

	if(fu_check_read_req(f, 2) == FU_SUCCESS)
	{
		const uint8_t stat = fu_read_data(f, (uint8_t*)&buf, 2, NULL);
		if(status != NULL) *status = stat;

		switch(endian)
		{
			case FU_LITTLE_ENDIAN:
				buf = tr_read_u16le((const uint8_t*)&buf);
				break;
			case FU_BIG_ENDIAN:
				buf = tr_read_u16be((const uint8_t*)&buf);
				break;
		}
	}
	else
	{
		if(status != NULL) *status = FU_ERROR;
	}
	
	return buf;
}

uint32_t fu_read_u32(FU_FILE* f, uint8_t* status, const uint8_t endian)
{
	uint32_t buf = 0;

	if(fu_check_read_req(f, 4) == FU_SUCCESS)
	{
		const uint8_t stat = fu_read_data(f, (uint8_t*)&buf, 4, NULL);
		if(status != NULL) *status = stat;

		switch(endian)
		{
			case FU_LITTLE_ENDIAN:
				buf = tr_read_u32le((const uint8_t*)&buf);
				break;
			case FU_BIG_ENDIAN:
				buf = tr_read_u32be((const uint8_t*)&buf);
				break;
		}
	}
	else
	{
		if(status != NULL) *status = FU_ERROR;
	}
	
	return buf;
}

uint64_t fu_read_u64(FU_FILE* f, uint8_t* status, const uint8_t endian)
{
	uint64_t buf = 0;

	if(fu_check_read_req(f, 8) == FU_SUCCESS)
	{
		const uint8_t stat = fu_read_data(f, (uint8_t*)&buf, 8, NULL);
		if(status != NULL) *status = stat;
	
		switch(endian)
		{
			case FU_LITTLE_ENDIAN:
				buf = tr_read_u64le((const uint8_t*)&buf);
				break;
			case FU_BIG_ENDIAN:
				buf = tr_read_u64be((const uint8_t*)&buf);
				break;
		}
	}
	else
	{
		if(status != NULL) *status = FU_ERROR;
	}
	
	return buf;
}

float fu_read_f32(FU_FILE* f, uint8_t* status, const uint8_t endian)
{
	float buf = 0;

	if(fu_check_read_req(f, 4) == FU_SUCCESS)
	{
		const uint8_t stat = fu_read_data(f, (uint8_t*)&buf, 4, NULL);
		if(status != NULL) *status = stat;

		switch(endian)
		{
			case FU_LITTLE_ENDIAN:
				buf = tr_read_f32le((const uint8_t*)&buf);
				break;
			case FU_BIG_ENDIAN:
				buf = tr_read_f32be((const uint8_t*)&buf);
				break;
		}
	}
	else
	{
		if(status != NULL) *status = FU_ERROR;
	}
	
	return buf;
}

double fu_read_f64(FU_FILE* f, uint8_t* status, const uint8_t endian)
{
	double buf = 0;

	if(fu_check_read_req(f, 8) == FU_SUCCESS)
	{
		const uint8_t stat = fu_read_data(f, (uint8_t*)&buf, 8, NULL);
		if(status != NULL) *status = stat;

		switch(endian)
		{
			case FU_LITTLE_ENDIAN:
				buf = tr_read_f64le((const uint8_t*)&buf);
				break;
			case FU_BIG_ENDIAN:
				buf = tr_read_f64be((const uint8_t*)&buf);
				break;
		}
	}
	else
	{
		if(status != NULL) *status = FU_ERROR;
	}
	
	return buf;
}

/*
	Writers
*/
uint8_t fu_write_data(FU_FILE* f, const uint8_t* data, const uint64_t size)
{
	if(size == 0) return FU_REQ0;
	
	const uint8_t check_stat = fu_check_buf_rem(f, size);
	
	if(check_stat != FU_SUCCESS)
	{
		return check_stat;
	}
	
	memcpy(&f->buf[f->pos], data, size);
	return fu_seek(f, size, FU_SEEK_CUR);
}

uint8_t fu_write_u8(FU_FILE* f, const uint8_t data)
{
	return fu_write_data(f, (const uint8_t*)&data, 1);
}

uint8_t fu_write_u16(FU_FILE* f, const uint16_t data, const uint8_t endian)
{
	uint16_t buf = data;
	
	switch(endian)
	{
		case FU_LITTLE_ENDIAN:
			tw_write_u16le(data, (uint8_t*)&buf);
			break;
		case FU_BIG_ENDIAN:
			tw_write_u16be(data, (uint8_t*)&buf);
			break;
	}
	
	return fu_write_data(f, (const uint8_t*)&buf, 2);
}

uint8_t fu_write_u32(FU_FILE* f, const uint32_t data, const uint8_t endian)
{
	uint32_t buf = data;
	
	switch(endian)
	{
		case FU_LITTLE_ENDIAN:
			tw_write_u32le(data, (uint8_t*)&buf);
			break;
		case FU_BIG_ENDIAN:
			tw_write_u32be(data, (uint8_t*)&buf);
			break;
	}
	
	return fu_write_data(f, (const uint8_t*)&buf, 4);
}

uint8_t fu_write_u64(FU_FILE* f, const uint64_t data, const uint8_t endian)
{
	uint64_t buf = data;
	
	switch(endian)
	{
		case FU_LITTLE_ENDIAN:
			tw_write_u64le(data, (uint8_t*)&buf);
			break;
		case FU_BIG_ENDIAN:
			tw_write_u64be(data, (uint8_t*)&buf);
			break;
	}
	
	return fu_write_data(f, (const uint8_t*)&buf, 8);
}

uint8_t fu_write_f32(FU_FILE* f, const float data, const uint8_t endian)
{
	float buf = data;
	
	switch(endian)
	{
		case FU_LITTLE_ENDIAN:
			tw_write_f32le(data, (uint8_t*)&buf);
			break;
		case FU_BIG_ENDIAN:
			tw_write_f32be(data, (uint8_t*)&buf);
			break;
	}
	
	return fu_write_data(f, (const uint8_t*)&buf, 4);
}

uint8_t fu_write_f64(FU_FILE* f, const double data, const uint8_t endian)
{
	double buf = data;
	
	switch(endian)
	{
		case FU_LITTLE_ENDIAN:
			tw_write_f64le(data, (uint8_t*)&buf);
			break;
		case FU_BIG_ENDIAN:
			tw_write_f64be(data, (uint8_t*)&buf);
			break;
	}
	
	return fu_write_data(f, (const uint8_t*)&buf, 8);
}
