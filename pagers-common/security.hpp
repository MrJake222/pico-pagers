#pragma once

#include <cstdint>
#include <cstddef>

// mostly to be used externally
const size_t HASH_LENGTH_BITS = 128;
const size_t HASH_LENGTH_BYTES = HASH_LENGTH_BITS / 8;

const size_t KEY_LENGTH_BITS = 64;
const size_t KEY_LENGTH_BYTES = KEY_LENGTH_BITS / 8;

// each function should return 0 on success
// values other than zero indicate error (add consts here for values)
// example: const int ERR_BAD_SIGNATURE = 1;

// should operate on raw bytes
// (no protocol struct access)

int crypto_hash(const uint8_t* data, size_t data_len, uint8_t* hash);
int crypto_sign(const uint8_t* private_key, const uint8_t* hash, uint8_t* encrypted_hash);
int crypto_verify(const uint8_t* public_key, const uint8_t* hash, const uint8_t* encrypted_hash);