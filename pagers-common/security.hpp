#pragma once

#include <cstdint>
#include <cstddef>

const size_t KEY_LENGTH_BITS = 128;
const size_t KEY_LENGTH_BYTES = KEY_LENGTH_BITS / 8;

// each function should return 0 on success
// values other than zero indicate error (add consts here for values)
// example: const int ERR_BAD_SIGNATURE = 1;

// should operate on raw bytes
// (no protocol struct access)

int crypto_encrypt(const uint8_t* private_key, const uint8_t* data, uint8_t* encrypted_data);
int crypto_decrypt(const uint8_t* public_key, const uint8_t* encrypted_data, uint8_t* data);