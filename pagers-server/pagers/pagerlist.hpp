#pragma once

#include <map>
#include <lfs.h>
#include "pager.hpp"

class PagerList {
    using PagersMap = std::map<unsigned short, Pager*>;

    PagersMap pagers;

    // filesystem
    lfs_t* lfs;
    const char* const path;

    volatile bool update_fs_required = false;
    void mark_update_fs() { update_fs_required = true; }
    void update_fs();

public:
    PagerList(lfs_t* lfs_, const char* path_)
        : lfs(lfs_)
        , path(path_)
        { }

    void load_fs();
    void loop();

    bool pager_exists(unsigned short device_id);
    bool add_pager(Pager* pager);
    bool remove_pager(unsigned short device_id);
    Pager* get_pager(unsigned short device_id);
    int size() { return pagers.size(); }

    PagersMap::iterator begin() { return pagers.begin(); }
    PagersMap::iterator end()   { return pagers.end(); }
};
