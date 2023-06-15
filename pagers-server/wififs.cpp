#include "wififs.hpp"

#include <fsutil.hpp>

const char* PATH = "/wifi";

void wifi_save(lfs_t* lfs, const char* ssid, const char* pwd, uint32_t auth) {
    lfs_file_t file;
    lfs_file_open(lfs, &file, PATH, LFS_O_WRONLY | LFS_O_CREAT);

    write_line(lfs, &file, ssid);
    write_line(lfs, &file, pwd);

    char buf[9];
    sprintf(buf, "%08lx", auth);
    write_line(lfs, &file, buf);

    lfs_file_close(lfs, &file);
}

// static int split(const char* start, const char** second) {
//     const char* ptr = strchr(start, '=');
//     if (!ptr)
//         return -1;
//
//     ptr++;
//     *second = ptr;
//     return 0;
// }

int wifi_read(lfs_t* lfs, char* ssid, char* pwd, uint32_t* auth) {
    lfs_file_t file;
    int r = lfs_file_open(lfs, &file, PATH, LFS_O_RDONLY);
    if (r < 0) {
        return -1;
    }

    read_line(lfs, &file, ssid);
    read_line(lfs, &file, pwd);

    char buf[9];
    read_line(lfs, &file, buf);
    sscanf(buf, "%lx", auth);

    lfs_file_close(lfs, &file);
    return 0;
}
