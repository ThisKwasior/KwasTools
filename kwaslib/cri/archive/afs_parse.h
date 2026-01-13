#pragma once

#include "afs.h"

/*
    Checks if the file seems like AFS.
    
    AFS_GOOD on success, otherwise AFS_ERROR.
*/
const uint8_t afs_check_if_valid(const uint8_t* data, const uint32_t size);

/*
    First file does not need to have index 0.
*/
const uint32_t afs_find_first_file_index(const uint8_t* data, const uint32_t size);

/*
    Will count files that aren't null from the first ID
    to its file offset or until AFS_MAX_FILES is reached.
*/
const uint32_t afs_count_possible_files(const uint32_t first_file_id, const uint8_t* data);

/*
    `file_count` here is the REAL amount of files, without null entries inbetween.
    Returns the metadata info index in the header.
*/
const uint32_t afs_find_metadata_index(const uint32_t first_file_id, const uint32_t file_count, const uint8_t* data);
