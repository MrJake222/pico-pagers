#pragma once

#include <cstdint>
#include <cstddef>

#include "security.hpp"

// this will get signed
struct proto_data {
    int receiver_id;
    int sequence_number;
};

// this will be sent as is
struct proto_frame {
    uint8_t encrypted_hash[HASH_LENGTH_BYTES];
    struct proto_data data;
};

// functions

// each function should return 0 on success
// values other than zero indicate error (add consts here for values)
// example: const int ERR_BAD_SIGNATURE = 1;

// takes frame->data, hashes, signs, and saves encrypted hash to
// frame->encrypted_hash
int proto_sign(const uint8_t* private_key, const struct proto_frame* frame);

// verifies frame->data with frame->encrypted_hash
int proto_verify(const uint8_t* public_key, const struct proto_frame* frame);