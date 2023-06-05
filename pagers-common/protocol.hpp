#pragma once

#include <cstdint>
#include <cstddef>

#include "security.hpp"

// this will get signed
// compiler promotes every sub 2-byte type to int (4 bytes)
struct proto_data {
    // defining this lower displaces memory and causes larger
    // overall struct size
    unsigned long long sequence_number;

    unsigned short receiver_id;

    unsigned short message_type;
    unsigned short message_param;

    unsigned short checksum;
};

#define PROTO_DATA_SIZE sizeof(struct proto_data)
#define PROTO_CHECKSUM_SIZE sizeof(short)

// this will be sent as is
struct proto_frame {
    unsigned char encrypted_data[PROTO_DATA_SIZE];
};

#define PROTO_FRAME_SIZE sizeof(struct proto_frame)

// functions

// each function should return 0 on success
// values other than zero indicate error (add consts here for values)
// example: const int ERR_BAD_SIGNATURE = 1;
const int SUCCESS = 0;
const int ERR_WRONG_CHECKSUM = -1;

int proto_checksum_calc(struct proto_data* data);
int proto_checksum_verify(struct proto_data* data);

// takes data and saves encrypted data to frame->encrypted_data
int proto_encrypt(const uint8_t* private_key, const struct proto_data* data, struct proto_frame* frame);

// verifies frame->encrypted_data and saves as frame
int proto_decrypt(const uint8_t* public_key, const struct proto_frame* frame, struct proto_data* data);