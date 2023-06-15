#pragma once

#include <lfs.h>

void wifi_save(lfs_t* lfs, const char* ssid, const char* pwd, uint32_t auth);

// returns -1 on failure
int wifi_read(lfs_t* lfs, char* ssid, char* pwd, uint32_t* auth);