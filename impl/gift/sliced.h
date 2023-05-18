#pragma once

#include <stdint.h>

#define ROUNDS_GIFT_SLICED_64 28

void swapmove(uint64_t *a, uint64_t *b, uint64_t m, int n);
void bits_pack(uint64_t m[8]);

void gift_64_sliced_generate_round_keys(uint64_t round_keys[ROUNDS_GIFT_SLICED_64][8],
                                        const uint64_t key[2]);
void gift_64_sliced_subcells(uint64_t s[8]);
void gift_64_sliced_subcells_inv(uint64_t s[8]);
void gift_64_sliced_permute(uint64_t s[8]);
void gift_64_sliced_permute_inv(uint64_t s[8]);

void gift_64_sliced_encrypt(uint64_t c[8], const uint64_t m[8], const uint64_t key[2]);
void gift_64_sliced_decrypt(uint64_t m[8], const uint64_t c[8], const uint64_t key[2]);
