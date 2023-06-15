#include "pagerlist.hpp"

bool PagerList::pager_exists(unsigned short device_id) {
    return pagers.count(device_id) > 0;
}

bool PagerList::add_pager(Pager* pager) {
    if (pager_exists(pager->get_device_id()))
        return false;

    pagers[pager->get_device_id()] = pager;
    mark_update_fs();
    return true;
}

bool PagerList::remove_pager(unsigned short device_id) {
    if (!pager_exists(device_id))
        return false;

    pagers.erase(device_id);
    mark_update_fs();
    return true;
}

Pager* PagerList::get_pager(unsigned short device_id) {
    if (!pager_exists(device_id))
        return nullptr;

    return pagers[device_id];
}

void PagerList::load_fs() {
    lfs_file_t file;
    int r;

    r = lfs_file_open(lfs, &file, path, LFS_O_RDONLY);
    if (r < 0) {
        printf("failed to load pagers err=%d\n", r);
        puts("if first boot, ignore");
        return;
    }

    // line by line
    // hex encoded short
    // 4 hex chars + newline

    char buf[5];
    while (true) {
        r = lfs_file_read(lfs, &file, buf, 5);
        if (r < 0) {
            printf("failed to load pagers err=%d\n", r);
            break;
        }
        if (r < 5) {
            // finished
            break;
        }

        buf[4] = '\0';

        int device_id;
        sscanf(buf, "%x", &device_id);
        pagers[device_id] = new Pager(device_id);
    }

    lfs_file_close(lfs, &file);
}

void PagerList::update_fs() {
    lfs_file_t file;
    int r;

    // write only, create, truncate
    r = lfs_file_open(lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (r < 0) {
        printf("failed to save pagers err=%d\n", r);
        return;
    }

    // line by line
    // hex encoded short
    // 4 hex chars + newline

    char buf[6];
    for (auto p : pagers) {
        sprintf(buf, "%04x\n", p.second->get_device_id());

        r = lfs_file_write(lfs, &file, buf, 5);
        if (r < 5) {
            printf("failed to save pagers err=%d\n", r);
            break;
        }
    }

    lfs_file_close(lfs, &file);
}

void PagerList::loop() {
    if (update_fs_required) {
        update_fs();
        update_fs_required = false;
    }
}
