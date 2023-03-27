#pragma once

#include <cstdint>
#include <cstddef>

#include "security.hpp"

// this will get signed
struct proto_data {
    unsigned short receiver_id;         // 2 bytes
    unsigned long sequence_number;      // 8 bytes

    unsigned short message_type;        // 2 bytes
    unsigned short message_param;       // 2 bytes

    unsigned int checksum;              // 4 bytes
                                        // 18 bytes
};

#define PROTO_DATA_SIZE sizeof(struct proto_data)

// this will be sent as is
struct proto_frame {
    unsigned char encrypted_data[PROTO_DATA_SIZE];
};

// functions

// each function should return 0 on success
// values other than zero indicate error (add consts here for values)
// example: const int ERR_BAD_SIGNATURE = 1;
const int SUCCESS = 0;

int proto_checksum_calc(struct proto_data* data);
int proto_checksum_verify(struct proto_data* data);

// takes data and saves encrypted data to frame->encrypted_data
int proto_encrypt(const uint8_t* private_key, const struct proto_data* data, struct proto_frame* frame);

// verifies frame->encrypted_data and saves as frame
int proto_decrypt(const uint8_t* public_key, const struct proto_frame* frame, struct proto_data* data);