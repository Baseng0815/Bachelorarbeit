#pragma once

// 128-bit bitsliced camellia with 16 blocks encrypted in parallel


#include <stdint.h>
#include <arm_neon.h>

#include "camellia_keys.h"

void rol32_1(uint8x16x4_t *a);

void camellia_sliced_F(uint8x16x4_t X[restrict 2],
                       const uint8x16x4_t k[restrict 2]);
void camellia_sliced_FL(uint8x16x4_t X[restrict 2],
                        const uint8x16x4_t kl[restrict 2]);
void camellia_sliced_FL_inv(uint8x16x4_t Y[restrict 2],
                            const uint8x16x4_t kl[restrict 2]);
void camellia_sliced_feistel_round(uint8x16x4_t state[restrict 4],
                                   const uint8x16x4_t kr[restrict 2]);
void camellia_sliced_feistel_round_inv(uint8x16x4_t state[restrict 4],
                                       const uint8x16x4_t kr[restrict 2]);
void camellia_sliced_generate_round_keys_128(struct camellia_rks_sliced_128 *restrict rks,
                                             const uint64_t key[2]);

void camellia_sliced_pack(uint8x16x4_t packed[restrict 4],
                          const uint64_t x[restrict 16][2]);

void camellia_sliced_unpack(uint64_t x[restrict 16][2],
                            const uint8x16x4_t packed[restrict 4]);

void camellia_sliced_init(void);

void camellia_sliced_encrypt_128(uint64_t c[restrict 16][2],
                                 const uint64_t m[restrict 16][2],
                                 struct camellia_rks_sliced_128 *restrict rks);

void camellia_sliced_decrypt_128(uint64_t m[restrict 16][2],
                                 const uint64_t c[restrict 16][2],
                                 struct camellia_rks_sliced_128 *restrict rks);
