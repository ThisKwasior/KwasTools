#pragma once

#include <stdint.h>

#include "string_utils.h"

#define PU_PATH_NO_VALUE (uint32_t)(0)

#define PU_PATH_TYPE_UNK 0
#define PU_PATH_TYPE_FILE 1
#define PU_PATH_TYPE_DIR 2

/*
	Description of the given path.
	
	If the size == PU_PATH_NO_VALUE, then the given char* does not exist.
*/
typedef struct
{
	SU_STRING* dir;  // ex. C:/projects
	SU_STRING* name; // ex. pathsplit	
	SU_STRING* ext;  // ex. `exe`, `h`, `ass`
    uint8_t type;
} PU_PATH;

/*

*/
int pu_is_file(const char* path);
int pu_is_dir(const char* path);

/*
	Converts any MS-DOS '\' to *nix '/'
*/
void pu_path_to_nix(char* str, uint32_t size);

/*
	Converts any *nix '/' to MS-DOS '\'
*/
void pu_path_to_dos(char* str, uint32_t size);

/*
    Allocates new PU_PATH and prepares internal strings to be used with string_utils
*/
PU_PATH* pu_alloc_path();

/*
	Splits the path and return allocated PU_PATH
*/
PU_PATH* pu_split_path(const char* file_path, uint32_t file_path_size);

/*
    Creates a directory at specified path.
    0 on success, -1 on failure.
*/
int pu_create_dir_char(const char* path);

/*
    Creates a directory at specified PU_PATH.
    If PU_PATH.type == PU_PATH_TYPE_FILE it will create the parent directory of the file.
    0 on success, -1 on failure.
*/
int pu_create_dir(const PU_PATH* path);

/*
    Removes a directory at specified path.
    Only empty directories can be deleted.
    0 on success, -1 on failure.
*/
int pu_remove_dir_char(const char* path);

/*
    Removes a directory at specified PU_PATH.
    If PU_PATH.type == PU_PATH_TYPE_FILE it will remove the parent directory of the file.
    Only empty directories can be deleted.
    0 on success, -1 on failure.
*/
int pu_remove_dir(const PU_PATH* path);

/*
	Frees all strings and path itself

	Returns NULL
*/
PU_PATH* pu_free_path(PU_PATH* path);

/*
    Converts PU_PATH to SU_STRING
*/
SU_STRING* pu_path_to_string(const PU_PATH* path);

/*
    Returns a file/directory name with extension
*/
SU_STRING* pu_get_basename_char(const char* path);

/*
    Returns a parent directory of a file/directory
*/
SU_STRING* pu_get_parent_dir_char(const char* path);

/*
    Returns a file/directory name without extension
*/
SU_STRING* pu_get_name_char(const char* path);

/*
    Returns an extension without a dot
*/
SU_STRING* pu_get_ext_char(const char* path);