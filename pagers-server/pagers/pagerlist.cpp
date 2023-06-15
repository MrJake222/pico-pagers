#include "pagerlist.hpp"

bool PagerList::pager_exists(unsigned short device_id) {
    return pagers.count(device_id) > 0;
}

bool PagerList::add_pager(Pager* pager) {
    if (!pager_exists(pager->get_device_id()))
        return false;

    pagers[pager->get_device_id()] = pager; // copy
    return true;
}

bool PagerList::remove_pager(unsigned short device_id) {
    if (!pager_exists(device_id))
        return false;

    pagers.erase(device_id);
    return true;
}

Pager* PagerList::get_pager(unsigned short device_id) {
    if (!pager_exists(device_id))
        return nullptr;

    return pagers[device_id];
}