#pragma once

#include "afs.h"

/*
    Returns index of the first entry in the AFS file.
    Otherwise AFS_ERROR.
*/
const uint32_t afs_get_first_entry_id(AFS_FILE* afs);

/*
    Returns index of the last entry in the AFS file.
    Otherwise AFS_ERROR.
*/
const uint32_t afs_get_last_entry_id(AFS_FILE* afs);

/*
    Calculates the size of the data section aligned to block size.
*/
const uint32_t afs_get_data_section_size(AFS_FILE* afs, const uint32_t block_size);