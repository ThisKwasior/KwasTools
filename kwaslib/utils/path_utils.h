#pragma once

#include <stdint.h>

#define PU_PATH_NO_VALUE (uint32_t)(-1)

#define PU_PATH_TYPE_UNK 0
#define PU_PATH_TYPE_FILE 1
#define PU_PATH_TYPE_DIR 2

typedef struct
{
	char* p;
	uint32_t s;
} PU_STRING;

/*
	Description of the given path.
	
	If the size == PU_PATH_NO_VALUE, then the given char* does not exist.
*/
typedef struct
{
	PU_STRING dir;  // ex. C:/projects
	PU_STRING name; // ex. pathsplit	
	PU_STRING ext;  // ex. `exe`, `h`, `ass`
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
	Creates a PU_STRING.
    str/size are the string and its size, 
    pustr is a pointer to allocated PU_STRING structure
*/
void pu_create_string(const char* str, const uint32_t size, PU_STRING* pustr);

/*
	Inserts the char* string into the existing PU_STRING
*/
void pu_insert_char(const char* str, const uint32_t size,
                    const uint32_t pos, PU_STRING* pustr);
                    
/*
	Inserts the PU_STRING into the PU_STRING
*/
void pu_insert_string(const PU_STRING* insert, const uint32_t pos, PU_STRING* target);

/*
	Splits the path and allocates all arrays in `desc` with proper data.
*/
void pu_split_path(const char* file_path, uint32_t file_path_size, PU_PATH* desc);

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
	Frees char array inside of the `str`
    and sets size value to PU_PATH_NO_VALUE.
	It won't free the `str` itself.
*/
void pu_free_string(PU_STRING* str);

/*
	Frees all char arrays inside of the `desc`
    and sets size values to PU_PATH_NO_VALUE.
	It won't free the `desc` itself.
*/
void pu_free_path(PU_PATH* desc);

/*
    Converts `desc` and all its components to one string and
    fills `str` with the info.
*/
void pu_path_to_string(const PU_PATH* desc, PU_STRING* str);