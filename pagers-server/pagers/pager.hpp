#pragma once

using ushort = unsigned short;

class Pager {
    ushort device_id;
    ushort flashes_left;

public:
    Pager(ushort device_id_)
        : device_id(device_id_)
        , flashes_left(0)
        { }

    ushort get_device_id() { return device_id; }

    bool any_flashes_left() { return flashes_left > 0; }
    void set_flashes_left(ushort flashes) { flashes_left = flashes; }
    void decrease_flashing_count() { flashes_left--; }
};