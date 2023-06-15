#include "fsutil.hpp"

void write_line(lfs_t* lfs, lfs_file_t* file, const char* line) {
    while (*line) {
        lfs_file_write(lfs, file, line, 1);
        line++;
    }

    lfs_file_write(lfs, file, "\n", 1);
}

void read_line(lfs_t* lfs, lfs_file_t* file, char* line) {
    char chr;

    while (true) {
        lfs_file_read(lfs, file, &chr, 1);
        if (chr == '\n')
            break;

        *line = chr;
        line++;
    }

    *line = '\0';
}
