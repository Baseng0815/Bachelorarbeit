#ifndef NAIVE_H
#define NAIVE_H

// 128-bit and 256-bit Camellia

#include "camellia_keys.h"
#include <stdint.h>

uint64_t camellia_naive_S(uint64_t X);
uint64_t camellia_naive_P(uint64_t X);
uint64_t camellia_naive_F(uint64_t X, const uint64_t k);
uint64_t camellia_naive_FL(uint64_t X, const uint64_t kl);
uint64_t camellia_naive_FL_inv(uint64_t X, const uint64_t kl);
void camellia_naive_feistel_round(uint64_t state[2], const uint64_t kr);
void camellia_naive_feistel_round_inv(uint64_t state[2], const uint64_t kr);
void camellia_naive_generate_round_keys_128(const uint64_t key[restrict 2],
                                            struct camellia_rks_128 *restrict rks);
void camellia_naive_generate_round_keys_256(const uint64_t key[restrict 4],
                                            struct camellia_rks_256 *restrict rks);

void camellia_naive_encrypt_128(uint64_t c[restrict 2],
                            const uint64_t m[restrict 2],
                            struct camellia_rks_128 *restrict rks);

void camellia_naive_decrypt_128(uint64_t m[restrict 2],
                            const uint64_t c[restrict 2],
                            struct camellia_rks_128 *restrict rks);

void camellia_naive_encrypt_256(uint64_t c[restrict 2],
                            const uint64_t m[restrict 2],
                            struct camellia_rks_256 *restrict rks);

void camellia_naive_decrypt_256(uint64_t m[restrict 2],
                            const uint64_t c[restrict 2],
                            struct camellia_rks_256 *restrict rks);

#endif
