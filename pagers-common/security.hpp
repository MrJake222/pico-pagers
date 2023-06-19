#pragma once

#include <cstddef>
#include <cstdlib>

typedef unsigned short ushort;
typedef unsigned int uint;

/*
 * This function encode 16-bit numbers into 32-bit ones
 * (modulo n should fit into 32bit)
 */

void crypto_encrypt(uint len, const ushort* data, uint* enc);
void crypto_decrypt(uint len, ushort* data, const uint* enc);