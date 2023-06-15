#pragma once

using ushort = unsigned short;

class Pager {
    ushort device_id;

    // flashing
    // how many more flash messages to send
    int flash_msgs_left = 0;

    // pairing
    // how many more pair messages to send
    int pair_msgs_left = 0;


public:
    Pager(ushort device_id_)
        : device_id(device_id_)
        { }

    ushort get_device_id() { return device_id; }

    // flashing data
    // how long the pager should flash
    ushort flash_time;
    // flashing messages
    void set_flash_msgs_left(ushort flash_msgs) { flash_msgs_left = flash_msgs; }
    bool any_flash_msgs_left() { return flash_msgs_left > 0; }
    void flash_msg_sent() { flash_msgs_left--; }

    // pairing messages
    void set_pair_msgs_left(ushort pair_msgs) { pair_msgs_left = pair_msgs; }
    bool any_pair_msgs_left() { return pair_msgs_left > 0; }
    void pair_msg_sent() { pair_msgs_left--; }
};