#ifndef NAIVE_H
#define NAIVE_H

#include <stdint.h>

// 128-bit Camellia

struct camellia_keytable {
        uint64_t kw[4]; // whitening
        uint64_t ku[18]; // used in F function
        uint64_t kl[4]; // used in FL layer
};

uint64_t camellia_naive_S(uint64_t X, uint64_t k);
uint64_t camellia_naive_P(uint64_t X, uint64_t k);
uint64_t camellia_naive_F(uint64_t X, uint64_t k);
uint64_t camellia_naive_FL(uint64_t X, uint64_t kl);
uint64_t camellia_naive_FL_inv(uint64_t X, uint64_t kl);
void camellia_naive_feistel_round(uint64_t state[2], uint64_t kr);
void camellia_naive_generate_round_keys(const uint64_t key[restrict 2],
                                        struct camellia_keytable *restrict rks);

void camellia_naive_encrypt(uint64_t m[restrict 2],
                            uint64_t key[restrict 2]);

#endif
