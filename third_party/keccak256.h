/*
 * Minimal Keccak-256 (SHA-3 candidate) implementation.
 * Based on the Keccak reference specification.
 * Only provides keccak256() for Ethereum/Tron address derivation.
 */
#ifndef KECCAK256_H
#define KECCAK256_H

#include <cstdint>
#include <cstring>

static inline uint64_t keccak_rotl64(uint64_t x, int y) {
    return (x << y) | (x >> (64 - y));
}

static inline void keccak_f1600(uint64_t state[25]) {
    static const uint64_t RC[24] = {
        0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
        0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
        0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
        0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
        0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
        0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
        0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
        0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
    };
    static const int ROTC[24] = {
        1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14,
        27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44
    };
    static const int PILN[24] = {
        10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4,
        15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1
    };

    for (int round = 0; round < 24; ++round) {
        // Theta
        uint64_t C[5];
        for (int i = 0; i < 5; ++i)
            C[i] = state[i] ^ state[i + 5] ^ state[i + 10] ^ state[i + 15] ^ state[i + 20];
        for (int i = 0; i < 5; ++i) {
            uint64_t D = C[(i + 4) % 5] ^ keccak_rotl64(C[(i + 1) % 5], 1);
            for (int j = 0; j < 25; j += 5)
                state[j + i] ^= D;
        }
        // Rho and Pi
        uint64_t t = state[1];
        for (int i = 0; i < 24; ++i) {
            int j = PILN[i];
            uint64_t tmp = state[j];
            state[j] = keccak_rotl64(t, ROTC[i]);
            t = tmp;
        }
        // Chi
        for (int j = 0; j < 25; j += 5) {
            uint64_t tmp[5];
            for (int i = 0; i < 5; ++i)
                tmp[i] = state[j + i];
            for (int i = 0; i < 5; ++i)
                state[j + i] = tmp[i] ^ ((~tmp[(i + 1) % 5]) & tmp[(i + 2) % 5]);
        }
        // Iota
        state[0] ^= RC[round];
    }
}

/*
 * Compute Keccak-256 hash.
 * input: data to hash
 * inputLen: length of input in bytes
 * output: 32-byte hash result
 */
static inline void keccak256(const uint8_t *input, size_t inputLen, uint8_t *output) {
    const size_t rate = 136; // (1600 - 256*2) / 8
    uint64_t state[25] = {0};

    // Absorb
    size_t offset = 0;
    while (offset + rate <= inputLen) {
        for (size_t i = 0; i < rate / 8; ++i) {
            uint64_t lane;
            memcpy(&lane, input + offset + i * 8, 8);
            state[i] ^= lane;
        }
        keccak_f1600(state);
        offset += rate;
    }

    // Final block with padding
    uint8_t padded[136] = {0};
    size_t remaining = inputLen - offset;
    memcpy(padded, input + offset, remaining);
    padded[remaining] = 0x01;     // Keccak padding (NOT SHA-3 0x06)
    padded[rate - 1] |= 0x80;

    for (size_t i = 0; i < rate / 8; ++i) {
        uint64_t lane;
        memcpy(&lane, padded + i * 8, 8);
        state[i] ^= lane;
    }
    keccak_f1600(state);

    // Squeeze (only need 32 bytes)
    memcpy(output, state, 32);
}

#endif /* KECCAK256_H */
