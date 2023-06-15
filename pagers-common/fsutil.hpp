#pragma once

#include <lfs.h>

void write_line(lfs_t* lfs, lfs_file_t * file, const char* line);

// returns line length
void read_line(lfs_t* lfs, lfs_file_t * file, char* line);