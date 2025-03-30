#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

PU_PATH* pu_alloc_path()
{
    PU_PATH* path = calloc(1, sizeof(PU_PATH));
    
    path->type = PU_PATH_TYPE_UNK;
    
    path->dir = su_create_string("", 0);
    path->name = su_create_string("", 0);
    path->ext = su_create_string("", 0);
    
	path->dir->size = PU_PATH_NO_VALUE;
	path->name->size = PU_PATH_NO_VALUE;
	path->ext->size = PU_PATH_NO_VALUE;
    
    return path;
}

PU_PATH* pu_split_path(const char* file_path, uint32_t file_path_size)
{   
    /* Convert path to *nix */
    SU_STRING* path_nix = su_create_string(file_path, file_path_size);
    pu_path_to_nix(path_nix->ptr, file_path_size);
    
    PU_PATH* desc = pu_alloc_path();
    
    /* Check if it's either a file or a directory */
    if(pu_is_file(path_nix->ptr)) desc->type = PU_PATH_TYPE_FILE;
    else if(pu_is_dir(path_nix->ptr)) desc->type = PU_PATH_TYPE_DIR;
    else desc->type = PU_PATH_TYPE_FILE; /* Doesn't exist, assume it's a file */

    /* Split path */
    desc->dir = pu_get_parent_dir_char(path_nix->ptr);
    
    /* If it's a file, separate the file name from the extension */
    if(desc->type == PU_PATH_TYPE_FILE)
    {
        desc->name = pu_get_name_char(path_nix->ptr);
        desc->ext = pu_get_ext_char(path_nix->ptr);
    }
    else if(desc->type == PU_PATH_TYPE_DIR)
    {
        desc->name = pu_get_basename_char(path_nix->ptr);
        desc->ext = su_create_string("", 0);
    }

    su_free(path_nix);

    return desc;
}

int pu_create_dir_char(const char* path)
{
#if defined(__WIN32__) || defined(__MINGW32__)
    return mkdir(path);
#else
    return mkdir(path, 0766);
#endif
}

int pu_create_dir(const PU_PATH* path)
{
    SU_STRING* str = su_create_string("", 0);

    if(path->dir->size != PU_PATH_NO_VALUE)
    {
        su_insert_string(str, 0, path->dir);
    }
    
    if(path->type == PU_PATH_TYPE_DIR)
    {
        if(path->dir->size != PU_PATH_NO_VALUE)
        {
            su_insert_char(str, -1, "/", 1);
        }
        su_insert_string(str, -1, path->name);
    }

    const int ret = pu_create_dir_char(str->ptr);
    
    su_free(str);
    
    return ret;
}

int pu_remove_dir_char(const char* path)
{
    return rmdir(path);
}

int pu_remove_dir(const PU_PATH* path)
{
    SU_STRING* str = su_create_string("", 0);

    if(path->dir->size != PU_PATH_NO_VALUE)
    {
        su_insert_string(str, 0, path->dir);
    }
    
    if(path->type == PU_PATH_TYPE_DIR)
    {
        if(path->dir->size != PU_PATH_NO_VALUE)
        {
            su_insert_char(str, -1, "/", 1);
        }
        su_insert_string(str, -1, path->name);
    }

    const int ret = pu_remove_dir_char(str->ptr);
    
    su_free(str);
    
    return ret;
}

PU_PATH* pu_free_path(PU_PATH* path)
{
	path->dir = su_free(path->dir);
	path->name = su_free(path->name);
	path->ext = su_free(path->ext);
    
    path->type = 0;
    
    free(path);
    
    return NULL;
}

SU_STRING* pu_path_to_string(const PU_PATH* path)
{
    SU_STRING* str = su_create_string("", 0);
    
    if(path->type != PU_PATH_TYPE_UNK)
    {
        if(path->dir->size)
        {
            su_insert_string(str, -1, path->dir);
        }
        
        if(path->name->size)
        {
            if(path->dir->size)
            {
                su_insert_char(str, -1, "/", 1);
            }

            su_insert_string(str, -1, path->name);
        }
        
        if(path->ext->size)
        {
            if(path->dir->size && (path->name->size == PU_PATH_NO_VALUE))
            {
                su_insert_char(str, -1, "/", 1);
            }

            su_insert_char(str, -1, ".", 1);
            su_insert_string(str, -1, path->ext);
        }
    }
    
    return str;
}

SU_STRING* pu_get_basename_char(const char* path)
{
    const char* slash_nix = strrchr(path, '/');
    const char* slash_dos = strrchr(path, '\\');
    
    uint32_t slash = 0;
    
    if(slash_nix != slash_dos)
    {
        slash = (slash_nix > slash_dos)
                ? (slash_nix - &path[0] + 1)
                : (slash_dos - &path[0] + 1);
    }

    const uint32_t str_len = strlen(&path[slash]);

    SU_STRING* str = su_create_string(&path[slash], str_len);
    
    return str;
}

SU_STRING* pu_get_parent_dir_char(const char* path)
{
    const char* slash_nix = strrchr(path, '/');
    const char* slash_dos = strrchr(path, '\\');
    
    char* slash = (char*)path;
    
    if(slash_nix != slash_dos)
    {
        slash = (char*)((slash_nix > slash_dos) ? (slash_nix) : (slash_dos));
    }
    
    const uint32_t str_len = slash - &path[0];

    SU_STRING* dir = su_create_string(path, str_len);
    
    return dir;
}

SU_STRING* pu_get_name_char(const char* path)
{
    SU_STRING* str = pu_get_basename_char(path);
    
    char* dot_ptr = strrchr(str->ptr, '.');
    
    if(dot_ptr)
    {
        dot_ptr[0] = '\0';
    }
    
    const uint32_t str_len = strlen(str->ptr);

    SU_STRING* name = su_cut(str, 0, str_len);
    
    su_free(str);
    
    return name;
}

SU_STRING* pu_get_ext_char(const char* path)
{
    SU_STRING* str = pu_get_basename_char(path);
    
    const char* dot_ptr = strrchr(str->ptr, '.');
    const uint32_t dot = dot_ptr ? (dot_ptr - &str->ptr[0] + 1) : str->size;
    const uint32_t str_len = strlen(&path[dot]);

    SU_STRING* ext = su_cut(str, dot, str_len);
    
    su_free(str);
    
    return ext;
}