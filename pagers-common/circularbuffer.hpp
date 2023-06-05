#pragma once

#include <cstdint>

class CircularBuffer {

    uint8_t* const buffer_hidden;
    uint8_t* const buffer;

    // can't do read pos + avail
    // available data would be modified from 2 cores
    // read/write keep that separate

    // where to read/write data from/to
    long read_at;
    long write_at;

    using read_callback_fn = void(*)(void*, unsigned int);
    void* read_ack_arg;
    read_callback_fn read_ack_callback;

public:
    const int size_hidden;
    const int size;

    /**
     *
     * @param size size of the visible buffer
     * @param size_hidden_ area before visible buffer (for wrapping)
     */
    CircularBuffer(int size, int size_hidden_)
        : size(size)
        , size_hidden(size_hidden_)
        , buffer_hidden(new uint8_t[size + size_hidden_])
        , buffer(buffer_hidden + size_hidden_)
        {
            reset();
        }

    ~CircularBuffer() {
        delete[] buffer_hidden;
    }

    void reset() volatile {
        read_at = 0;
        write_at = 0;
        read_ack_callback = nullptr;
    }

    long get_read_offset()  volatile const { return read_at; }
    long get_write_offset() volatile const { return write_at; }

    long data_left()                volatile const;
    long data_left_continuous()     volatile const;
    long space_left()               volatile const;
    long space_left_continuous()    volatile const;

    int health() volatile const { return data_left() * 100 / size; }


    uint8_t* read_ptr()  volatile const; // TODO make this return const, requires helixmp3 adjustments
    uint8_t* write_ptr() volatile const;

    void read_ack(unsigned int bytes)  volatile;
    void write_ack(unsigned int bytes) volatile;

    void read_reverse(unsigned int bytes) volatile { read_at -= (long)bytes; }


    // wrapping
    bool can_wrap_buffer()  volatile const;
    void wrap_buffer()      volatile;


    // set read pointer almost at the end of continuous buffer chunk.
    // This may involve write pointer (marks the end of available data)
    void set_read_ptr_end(unsigned int from_end) volatile;


    // helper functions
    // move all data from here to other buffer
    void move_to(volatile CircularBuffer& other) volatile;

    // write at write_ptr and ack
    void write(const uint8_t* data, long data_len) volatile;


    // register a callback when data was just consumed
    // (more free space available)
    // only one callback supported
    void set_read_ack_callback(void* arg, read_callback_fn callback) volatile;


    // print read debug info
    void debug_read(int bytes, int prepend) volatile;
};
