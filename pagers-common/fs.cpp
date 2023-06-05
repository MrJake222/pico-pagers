#include "fs.hpp"

#include <hardware/flash.h>

#define FS_BASE_IN_FLASH (PICO_FLASH_SIZE_BYTES - FS_SIZE)
#define FS_BASE_ABS      (XIP_NOCACHE_NOALLOC_BASE + FS_BASE_IN_FLASH)

// Read a region in a block. Negative error codes are propagated
// to the user.
int pico_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    memcpy(buffer,
           (const void*)(FS_BASE_ABS + block * FLASH_SECTOR_SIZE + off),
           size);

    return 0;
}

// Program a region in a block. The block must have previously
// been erased. Negative error codes are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int pico_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    flash_range_program(FS_BASE_IN_FLASH + block * FLASH_SECTOR_SIZE + off,
                        (const uint8_t*)buffer,
                        size);

    return 0;
}

// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int pico_erase(const struct lfs_config *c, lfs_block_t block) {
    flash_range_erase(FS_BASE_IN_FLASH + block * FLASH_SECTOR_SIZE,
                      1);

    return 0;
}

// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
int pico_sync(const struct lfs_config *c) { return 0; }

const struct lfs_config pico_lfs_config = {
    .context = nullptr,

    // block device operations
    .read  = pico_read,
    .prog  = pico_prog,
    .erase = pico_erase,
    .sync  = pico_sync,

    // block device configuration
    .read_size = FLASH_PAGE_SIZE,
    .prog_size = FLASH_PAGE_SIZE,
    .block_size = FLASH_SECTOR_SIZE,

    .block_count = FS_SIZE / FLASH_SECTOR_SIZE,

    .block_cycles = 500,
    .cache_size = 1024,
    .lookahead_size = 1024,
};