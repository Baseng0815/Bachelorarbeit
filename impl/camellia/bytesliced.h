#ifndef BYTESLICED_H
#define BYTESLICED_H

// 128-bit bitsliced camellia

#include <stdint.h>
#include <arm_neon.h>

struct camellia_rks_sliced_128 {
        uint8x16x4_t kw[2][4]; // whitening
        uint8x16x4_t ku[2][18]; // used in F function
        uint8x16x4_t kl[2][4]; // used in FL layer
};

void camellia_sliced_F(uint8x16x4_t X[restrict 2],
                       const uint8x16x4_t k[restrict 2]);
void camellia_sliced_FL(uint8x16x4_t X[restrict 2],
                        const uint8x16x4_t kl[restrict 2]);
void camellia_sliced_FL_inv(uint8x16x4_t X[restrict 2],
                            const uint8x16x4_t kl[restrict 2]);
void camellia_sliced_feistel_round(uint8x16x4_t state[restrict 2],
                                   const uint8x16x4_t kr[restrict 2]);
void camellia_sliced_feistel_round_inv(uint8x16x4_t state[restrict 2],
                                       const uint8x16x4_t kr[restrict 2]);
void camellia_sliced_generate_round_keys_128(const uint64_t key[restrict 2],
                                             struct camellia_rks_sliced_128 *restrict rks);

void camellia_sliced_pack(uint8x16x4_t packed[restrict 2],
                          const uint64_t x[restrict 2][8]);

void camellia_sliced_unpack(uint64_t x[restrict 2][8],
                            const uint8x16x4_t packed[restrict 2]);

void camellia_sliced_init(void);

void camellia_sliced_encrypt_128(uint64_t c[restrict 2][8],
                                 const uint64_t m[restrict 2][8],
                                 struct camellia_rks_sliced_128 *restrict rks);

void camellia_sliced_decrypt_128(uint64_t m[restrict 2][8],
                                 const uint64_t c[restrict 2][8],
                                 struct camellia_rks_sliced_128 *restrict rks);

#endif
