#include "security.hpp"

// source:
// https://www.geeksforgeeks.org/modular-exponentiation-power-in-modular-arithmetic/
// calculate (x^y) % p
static uint power(long long x, uint y, uint p) {
    uint res = 1;

    x = x % p;

    if (x == 0)
        return 0;

    while (y > 0) {
        // If y is odd, multiply x with result
        if (y & 1)
            res = (res*x) % p;

        y = y>>1;
        x = (x*x) % p;
    }

    return res;
}

// keys
static uint n = 0x9050f681;
static uint e = 0x10001;
static uint d = 0xafe41a1d;

void crypto_encrypt(uint len, const ushort* data, uint* enc) {
    for (int i=0; i<len; i++) {
        enc[i] = power(data[i], d, n);
    }
}

void crypto_decrypt(uint len, ushort* data, const uint* enc) {
    for (int i=0; i<len; i++) {
        data[i] = power(enc[i], e, n);
    }
}
