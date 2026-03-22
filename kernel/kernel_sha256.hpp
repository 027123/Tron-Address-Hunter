#ifndef HPP_KERNEL_SHA256
#define HPP_KERNEL_SHA256

#include <string>

const std::string kernel_sha256 = R"(

/* SHA-256 round constants in __constant memory (shared by all work items).
 * Moving these out of per-thread private memory saves ~256 bytes of register
 * pressure per SHA-256 invocation.  Since the scoring kernel calls SHA-256
 * twice, this frees ~512 bytes of registers per thread -- a significant
 * occupancy improvement on most GPUs. */
__constant const uint sha256_K[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
  0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
  0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
  0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
  0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
  0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
  0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
  0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define SHA256_H0 0x6a09e667
#define SHA256_H1 0xbb67ae85
#define SHA256_H2 0x3c6ef372
#define SHA256_H3 0xa54ff53a
#define SHA256_H4 0x510e527f
#define SHA256_H5 0x9b05688c
#define SHA256_H6 0x1f83d9ab
#define SHA256_H7 0x5be0cd19

inline uint sha_rotr(uint x, int n) {
  return (x >> n) | (x << (32 - n));
}

inline uint sha_ch(uint x, uint y, uint z) {
  return (x & y) ^ (~x & z);
}

inline uint sha_maj(uint x, uint y, uint z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

inline uint sha_sig0(uint x) { return sha_rotr(x, 7) ^ sha_rotr(x, 18) ^ (x >> 3); }
inline uint sha_sig1(uint x) { return sha_rotr(x, 17) ^ sha_rotr(x, 19) ^ (x >> 10); }
inline uint sha_csig0(uint x) { return sha_rotr(x, 2) ^ sha_rotr(x, 13) ^ sha_rotr(x, 22); }
inline uint sha_csig1(uint x) { return sha_rotr(x, 6) ^ sha_rotr(x, 11) ^ sha_rotr(x, 25); }

/* ------------------------------------------------------------------ */
/* Specialized SHA-256 for exactly 21-byte input.                      */
/* Used for: SHA256(0x41 || 20-byte-hash) in TRON address generation. */
/* Eliminates all branching and uses rolling W[16] schedule.           */
/* ------------------------------------------------------------------ */
void sha256_21(const uchar *key, uchar *output) {
  uint W[16];

  /* Load 21 bytes into message schedule -- fully unrolled, no branches */
  W[0]  = ((uint)key[0]  << 24) | ((uint)key[1]  << 16) | ((uint)key[2]  << 8) | (uint)key[3];
  W[1]  = ((uint)key[4]  << 24) | ((uint)key[5]  << 16) | ((uint)key[6]  << 8) | (uint)key[7];
  W[2]  = ((uint)key[8]  << 24) | ((uint)key[9]  << 16) | ((uint)key[10] << 8) | (uint)key[11];
  W[3]  = ((uint)key[12] << 24) | ((uint)key[13] << 16) | ((uint)key[14] << 8) | (uint)key[15];
  W[4]  = ((uint)key[16] << 24) | ((uint)key[17] << 16) | ((uint)key[18] << 8) | (uint)key[19];
  W[5]  = ((uint)key[20] << 24) | 0x00800000u;
  W[6]  = 0; W[7]  = 0; W[8]  = 0; W[9]  = 0;
  W[10] = 0; W[11] = 0; W[12] = 0; W[13] = 0;
  W[14] = 0;
  W[15] = 168u;  /* 21 * 8 */

  uint A = SHA256_H0, B = SHA256_H1, C = SHA256_H2, D = SHA256_H3;
  uint E = SHA256_H4, F = SHA256_H5, G = SHA256_H6, H = SHA256_H7;
  uint T1, T2;

  /* Rounds 0-15: consume initial message words */
#pragma unroll
  for (int i = 0; i < 16; i++) {
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

  /* Rounds 16-63: rolling W[16] schedule -- saves 192 bytes of registers */
#pragma unroll
  for (int i = 16; i < 64; i++) {
    W[i & 15] += sha_sig1(W[(i - 2) & 15]) + W[(i - 7) & 15] + sha_sig0(W[(i - 15) & 15]);
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i & 15];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

  W[0] = A + SHA256_H0; W[1] = B + SHA256_H1;
  W[2] = C + SHA256_H2; W[3] = D + SHA256_H3;
  W[4] = E + SHA256_H4; W[5] = F + SHA256_H5;
  W[6] = G + SHA256_H6; W[7] = H + SHA256_H7;

  for (int i = 0; i < 8; i++) {
    output[i * 4 + 0] = (W[i] >> 24) & 0xFF;
    output[i * 4 + 1] = (W[i] >> 16) & 0xFF;
    output[i * 4 + 2] = (W[i] >> 8)  & 0xFF;
    output[i * 4 + 3] =  W[i]        & 0xFF;
  }
}

/* ------------------------------------------------------------------ */
/* Specialized SHA-256 for exactly 32-byte input.                      */
/* Used for: SHA256(first_hash) in double-SHA256 TRON checksum.        */
/* ------------------------------------------------------------------ */
void sha256_32(const uchar *key, uchar *output) {
  uint W[16];

  /* Load 32 bytes -- exactly 8 words, no branches */
  W[0] = ((uint)key[0]  << 24) | ((uint)key[1]  << 16) | ((uint)key[2]  << 8) | (uint)key[3];
  W[1] = ((uint)key[4]  << 24) | ((uint)key[5]  << 16) | ((uint)key[6]  << 8) | (uint)key[7];
  W[2] = ((uint)key[8]  << 24) | ((uint)key[9]  << 16) | ((uint)key[10] << 8) | (uint)key[11];
  W[3] = ((uint)key[12] << 24) | ((uint)key[13] << 16) | ((uint)key[14] << 8) | (uint)key[15];
  W[4] = ((uint)key[16] << 24) | ((uint)key[17] << 16) | ((uint)key[18] << 8) | (uint)key[19];
  W[5] = ((uint)key[20] << 24) | ((uint)key[21] << 16) | ((uint)key[22] << 8) | (uint)key[23];
  W[6] = ((uint)key[24] << 24) | ((uint)key[25] << 16) | ((uint)key[26] << 8) | (uint)key[27];
  W[7] = ((uint)key[28] << 24) | ((uint)key[29] << 16) | ((uint)key[30] << 8) | (uint)key[31];
  W[8] = 0x80000000u;
  W[9]  = 0; W[10] = 0; W[11] = 0; W[12] = 0;
  W[13] = 0; W[14] = 0;
  W[15] = 256u;  /* 32 * 8 */

  uint A = SHA256_H0, B = SHA256_H1, C = SHA256_H2, D = SHA256_H3;
  uint E = SHA256_H4, F = SHA256_H5, G = SHA256_H6, H = SHA256_H7;
  uint T1, T2;

  /* Rounds 0-15: consume initial message words */
#pragma unroll
  for (int i = 0; i < 16; i++) {
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

  /* Rounds 16-63: rolling W[16] schedule -- saves 192 bytes of registers */
#pragma unroll
  for (int i = 16; i < 64; i++) {
    W[i & 15] += sha_sig1(W[(i - 2) & 15]) + W[(i - 7) & 15] + sha_sig0(W[(i - 15) & 15]);
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i & 15];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

  W[0] = A + SHA256_H0; W[1] = B + SHA256_H1;
  W[2] = C + SHA256_H2; W[3] = D + SHA256_H3;
  W[4] = E + SHA256_H4; W[5] = F + SHA256_H5;
  W[6] = G + SHA256_H6; W[7] = H + SHA256_H7;

  for (int i = 0; i < 8; i++) {
    output[i * 4 + 0] = (W[i] >> 24) & 0xFF;
    output[i * 4 + 1] = (W[i] >> 16) & 0xFF;
    output[i * 4 + 2] = (W[i] >> 8)  & 0xFF;
    output[i * 4 + 3] =  W[i]        & 0xFF;
  }
}

/* ------------------------------------------------------------------ */
/* Fused double-SHA256 for TRON checksum.                              */
/* Computes SHA256(SHA256(input_21_bytes)) keeping intermediate hash   */
/* in registers (W[0..7]) instead of serializing to a byte array.     */
/* Eliminates 64 bytes of intermediate private memory (hash1 + hash2) */
/* and ~64 byte-level load/store operations between the two rounds.   */
/* Only the first 4 bytes of the final hash are output (checksum).    */
/* ------------------------------------------------------------------ */
void sha256_21_to_checksum(const uchar *input, uchar *checksum) {
  uint W[16];

  /* === Round 1: SHA-256 of 21-byte input === */
  W[0]  = ((uint)input[0]  << 24) | ((uint)input[1]  << 16) | ((uint)input[2]  << 8) | (uint)input[3];
  W[1]  = ((uint)input[4]  << 24) | ((uint)input[5]  << 16) | ((uint)input[6]  << 8) | (uint)input[7];
  W[2]  = ((uint)input[8]  << 24) | ((uint)input[9]  << 16) | ((uint)input[10] << 8) | (uint)input[11];
  W[3]  = ((uint)input[12] << 24) | ((uint)input[13] << 16) | ((uint)input[14] << 8) | (uint)input[15];
  W[4]  = ((uint)input[16] << 24) | ((uint)input[17] << 16) | ((uint)input[18] << 8) | (uint)input[19];
  W[5]  = ((uint)input[20] << 24) | 0x00800000u;
  W[6]  = 0; W[7]  = 0; W[8]  = 0; W[9]  = 0;
  W[10] = 0; W[11] = 0; W[12] = 0; W[13] = 0;
  W[14] = 0;
  W[15] = 168u;  /* 21 * 8 */

  uint A = SHA256_H0, B = SHA256_H1, C = SHA256_H2, D = SHA256_H3;
  uint E = SHA256_H4, F = SHA256_H5, G = SHA256_H6, H = SHA256_H7;
  uint T1, T2;

#pragma unroll
  for (int i = 0; i < 16; i++) {
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

#pragma unroll
  for (int i = 16; i < 64; i++) {
    W[i & 15] += sha_sig1(W[(i - 2) & 15]) + W[(i - 7) & 15] + sha_sig0(W[(i - 15) & 15]);
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i & 15];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

  /* First hash stays in W[0..7] -- no byte serialization needed */
  W[0] = A + SHA256_H0; W[1] = B + SHA256_H1;
  W[2] = C + SHA256_H2; W[3] = D + SHA256_H3;
  W[4] = E + SHA256_H4; W[5] = F + SHA256_H5;
  W[6] = G + SHA256_H6; W[7] = H + SHA256_H7;

  /* === Round 2: SHA-256 of 32-byte first hash (already in W[0..7]) === */
  W[8] = 0x80000000u;
  W[9]  = 0; W[10] = 0; W[11] = 0; W[12] = 0;
  W[13] = 0; W[14] = 0;
  W[15] = 256u;  /* 32 * 8 */

  A = SHA256_H0; B = SHA256_H1; C = SHA256_H2; D = SHA256_H3;
  E = SHA256_H4; F = SHA256_H5; G = SHA256_H6; H = SHA256_H7;

#pragma unroll
  for (int i = 0; i < 16; i++) {
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

#pragma unroll
  for (int i = 16; i < 64; i++) {
    W[i & 15] += sha_sig1(W[(i - 2) & 15]) + W[(i - 7) & 15] + sha_sig0(W[(i - 15) & 15]);
    T1 = H + sha_csig1(E) + sha_ch(E, F, G) + sha256_K[i] + W[i & 15];
    T2 = sha_csig0(A) + sha_maj(A, B, C);
    H = G; G = F; F = E; E = D + T1;
    D = C; C = B; B = A; A = T1 + T2;
  }

  /* Only the first 4 bytes (checksum) are needed */
  uint h0 = A + SHA256_H0;
  checksum[0] = (h0 >> 24) & 0xFF;
  checksum[1] = (h0 >> 16) & 0xFF;
  checksum[2] = (h0 >> 8)  & 0xFF;
  checksum[3] =  h0        & 0xFF;
}

/* ------------------------------------------------------------------ */
/* Optimized TRON address hash conversion.                             */
/* Writes payload directly to tronhash, then appends 4-byte checksum  */
/* via fused double-SHA256.  Saves 85 bytes of private memory vs.     */
/* the unfused version (hash0[21] + hash1[32] + hash2[32]).           */
/* ------------------------------------------------------------------ */
void ethhash_to_tronhash(const uchar *ethhash, uchar *tronhash) {
  tronhash[0] = 65;  /* 0x41 -- TRON network prefix */
  for (uint i = 0; i < 20; i++) {
    tronhash[i + 1] = ethhash[i];
  }
  sha256_21_to_checksum(tronhash, tronhash + 21);
}

__constant char alphabet[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

void base58_encode(const uchar *input, char *output, const int input_len) {
  __private uint digits[34] = {0};
  int digit_count = 1;

  for (int i = 0; i < input_len; i++) {
    uint carry = input[i];
    for (int j = 0; j < digit_count; j++) {
      carry += digits[j] << 8;
      digits[j] = carry % 58;
      carry /= 58;
    }
    while (carry) {
      digits[digit_count++] = carry % 58;
      carry /= 58;
    }
  }

  /* For TRON addresses the first byte is 0x41 (non-zero),
   * so zero_count is always 0.  We skip that check entirely. */
  int output_idx = 0;
  for (int i = digit_count - 1; i >= 0; i--) {
    output[output_idx++] = alphabet[digits[i]];
  }
  output[output_idx] = '\0';
}
)";

#endif /* HPP_KERNEL_SHA256 */
