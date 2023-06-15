#pragma once

#include <map>
#include "pager.hpp"

class PagerList {
    using PagersMap = std::map<unsigned short, Pager*>;

    PagersMap pagers;

public:
    bool pager_exists(unsigned short device_id);
    bool add_pager(Pager* pager);
    bool remove_pager(unsigned short device_id);
    Pager* get_pager(unsigned short device_id);
    int size() { return pagers.size(); }

    PagersMap::iterator begin() { return pagers.begin(); }
    PagersMap::iterator end()   { return pagers.end(); }
};
