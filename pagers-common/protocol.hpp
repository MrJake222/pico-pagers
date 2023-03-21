#pragma once

#include <cstdint>
#include <cstddef>

const size_t KEY_LENGTH_BITS = 64;
const size_t KEY_LENGTH_BYTES = KEY_LENGTH_BITS / 8;

const size_t HASH_LENGTH_BITS = 128;
const size_t HASH_LENGTH_BYTES = HASH_LENGTH_BITS / 8;

// structs
struct proto_frame {
    int receiver_id;
    int sequence_number;
    uint8_t encrypted_hash[HASH_LENGTH_BYTES];
};

// functions
// each function should return 0 on success
// values other than zero indicate error

int hash(const uint8_t* data, size_t data_len, uint8_t* hash);
int crypto_sign(const uint8_t* private_key, const uint8_t* hash, uint8_t* encrypted_hash);
int crypto_verify(const uint8_t* public_key, const uint8_t* hash, const uint8_t* encrypted_hash);
