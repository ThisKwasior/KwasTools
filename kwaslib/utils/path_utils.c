#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path_utils.h"

int pu_is_file(const char* path)
{
    struct stat st;
    stat(path, &st);
    return S_ISREG(st.st_mode);
}

int pu_is_dir(const char* path)
{
    struct stat st;
    stat(path, &st);
    return S_ISDIR(st.st_mode);
}

void pu_path_to_nix(char* str, uint32_t size)
{
	while(size)
	{
		if(str[size-1] == '\\')
			str[size-1] = '/';
		
		size-=1;
	}
}

void pu_path_to_dos(char* str, uint32_t size)
{
	while(size)
	{
		if(str[size-1] == '/')
			str[size-1] = '\\';
		
		size-=1;
	}
}

void pu_create_string(const char* str, const uint32_t size, PU_STRING* pustr)
{
    pu_free_string(pustr);
    
    if(size != PU_PATH_NO_VALUE)
    {
        pustr->s = size;
        pustr->p = (char*)calloc(size+1, 1);
        memcpy(pustr->p, str, size);
    }
    else
    {
        pustr->s = 0;
        pustr->p = (char*)calloc(1, 1);
    }
}

void pu_insert_char(const char* str, const uint32_t size,
                      const uint32_t pos, PU_STRING* pustr)
{
    const uint32_t new_size = size+pustr->s;
    char* buf = (char*)calloc(new_size+1, 1);
    
    if(pos == 0)
    {
        memcpy(&buf[0], str, size);
        memcpy(&buf[size], pustr->p, pustr->s);
    }
    else if(pos > pustr->s)
    {
        memcpy(&buf[0], pustr->p, pustr->s);
        memcpy(&buf[pustr->s], str, size);
    }
    else
    {
        memcpy(&buf[0], pustr->p, pos);
        memcpy(&buf[pos], str, size);
        memcpy(&buf[pos+size], &pustr->p[pos], pustr->s-pos);
    }
    
    pu_free_string(pustr);
    
    pustr->p = buf;
    pustr->s = new_size;
}

void pu_insert_string(const PU_STRING* insert, const uint32_t pos, PU_STRING* target)
{
    if(target->s == PU_PATH_NO_VALUE)
    {
        pu_create_string(insert->p, insert->s, target);
    }
    else
    {
        pu_insert_char(insert->p, insert->s, pos, target);
    }
}

void pu_split_path(const char* file_path, uint32_t file_path_size, PU_PATH* desc)
{
	desc->dir.s = PU_PATH_NO_VALUE;
	desc->name.s = PU_PATH_NO_VALUE;
	desc->ext.s = PU_PATH_NO_VALUE;
    
    /* Convert path to *nix */
    /* TODO: Make windows not suck balls with text encoding */
    char* path = (char*)calloc(file_path_size, 1);
    memcpy(path, file_path, file_path_size);

    pu_path_to_nix(path, file_path_size);

    /* Check if it's either a file or directory */
    if(pu_is_file(file_path)) desc->type = PU_PATH_TYPE_FILE;
    else if(pu_is_dir(file_path)) desc->type = PU_PATH_TYPE_DIR;
    else
    {
        printf("The path is not either a file or directory.\n");
        desc->type = PU_PATH_TYPE_UNK;
        return;
    }
    
    /* Find base directory, file name and extension */
    uint32_t period = PU_PATH_NO_VALUE;
	uint32_t slash = PU_PATH_NO_VALUE;
    
    for(uint32_t i = 0; i != file_path_size; ++i)
    {	
        if(path[i] == '.') period = i;
        if(path[i] == '/') slash = i;
    }
    
    // The path has a dir
    if(slash != PU_PATH_NO_VALUE)
    {
        desc->dir.s = slash;
        desc->dir.p = (char*)calloc(desc->dir.s, 1);
        memcpy(desc->dir.p, path, desc->dir.s);
        file_path_size -= desc->dir.s + 1;
        memcpy(&path[0], &path[desc->dir.s+1], file_path_size);
        if(period != PU_PATH_NO_VALUE) period -= desc->dir.s + 1;
    }

    if(desc->type == PU_PATH_TYPE_FILE)
    {
        // File with an extension
        if(period != PU_PATH_NO_VALUE)
        {
            desc->name.s = period;
            desc->ext.s = file_path_size-period-1;
            desc->name.p = (char*)calloc(desc->name.s+1, 1);
            desc->ext.p = (char*)calloc(desc->ext.s+1, 1);
            memcpy(desc->name.p, path, desc->name.s);
            memcpy(desc->ext.p, &path[period+1], desc->ext.s);
        }
        else // File without an extension
        {
            desc->name.s = file_path_size;
            desc->name.p = (char*)calloc(desc->name.s+1, 1);
            memcpy(desc->name.p, path, desc->name.s);
        }
    }
    else if(desc->type == PU_PATH_TYPE_DIR)
    {
        desc->name.s = file_path_size;
        desc->name.p = (char*)calloc(desc->name.s+1, 1);
        memcpy(desc->name.p, path, desc->name.s);
    }

    /*printf("%d %d %d\n", slash, period, file_path_size);
    printf("%d %d %d\n", desc->dir.s, desc->name.s, desc->ext.s);
    printf("%s %s %s\n", desc->dir.p, desc->name.p, desc->ext.p);*/

    free(path);
}

int pu_create_dir_char(const char* path)
{
#if defined(__WIN32__) || defined(__MINGW32__)
    return mkdir(path);
#else
    return mkdir(path, S_IFDIR);
#endif
}

int pu_create_dir(const PU_PATH* path)
{
    PU_STRING str = {0};

    if(path->dir.s != PU_PATH_NO_VALUE)
        pu_create_string(path->dir.p, path->dir.s, &str);
    else
        pu_create_string("", 0, &str);
    
    if(path->type == PU_PATH_TYPE_DIR)
    {
        if(path->dir.s != PU_PATH_NO_VALUE)
            pu_insert_char("/", 1, -1, &str);
        pu_insert_string(&path->name, -1, &str);
    }

    return pu_create_dir_char(str.p);
}

void pu_free_string(PU_STRING* str)
{
	free(str->p);
	str->s = PU_PATH_NO_VALUE;
}

void pu_free_path(PU_PATH* desc)
{
	pu_free_string(&desc->dir);
	pu_free_string(&desc->name);
	pu_free_string(&desc->ext);
    
    desc->type = PU_PATH_TYPE_UNK;
}

void pu_path_to_string(const PU_PATH* desc, PU_STRING* str)
{
    // PATH doesn't exist
    if(desc->type == PU_PATH_TYPE_UNK)
    {
        str->s = PU_PATH_NO_VALUE;
        return;
    }
    
    str->s = 0;
    
    if(desc->dir.s != PU_PATH_NO_VALUE) str->s += desc->dir.s + 1;
    if(desc->name.s != PU_PATH_NO_VALUE) str->s += desc->name.s;
    if(desc->ext.s != PU_PATH_NO_VALUE) str->s += desc->ext.s + 1;

    str->p = (char*)calloc(str->s+1, 1);
    
    uint32_t it = 0;
    
    if(desc->dir.s != PU_PATH_NO_VALUE)
    {
        memcpy(&str->p[it], desc->dir.p, desc->dir.s);
        it += desc->dir.s;
        str->p[it++] = '/';
    }
    
    if(desc->name.s != PU_PATH_NO_VALUE)
    {
        memcpy(&str->p[it], desc->name.p, desc->name.s);
        it += desc->name.s;
    }
    
    if(desc->ext.s != PU_PATH_NO_VALUE)
    {
        str->p[it++] = '.';
        memcpy(&str->p[it], desc->ext.p, desc->ext.s);
    }
}