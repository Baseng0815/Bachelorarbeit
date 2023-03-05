#ifndef GIFT_VEC_SLICED_H
#define GIFT_VEC_SLICED_H

#include <stdint.h>
#include <arm_neon.h>

#define ROUNDS_GIFT_64 28

// expose for benchmarking
void swapmove(uint8x16_t *a, uint8x16_t *b, uint8x16_t m, int n);
void bits_pack(uint8x16x4_t m);

void gift_64_vec_sliced_subcells(uint8x16x4_t cipher_state);
void gift_64_vec_sliced_subcells_inv(uint8x16x4_t cipher_state);
void gift_64_vec_sliced_permute(uint8x16x4_t cipher_state);
void gift_64_vec_sliced_permute_inv(uint8x16x4_t cipher_state);
void gift_64_vec_sliced_generate_round_keys(uint8x16x4_t round_keys[ROUNDS_GIFT_64],
                                            const uint64_t key[2]);

void gift_64_vec_sliced_init(void);

void gift_64_vec_sbox_encrypt(uint64_t c[8], const uint64_t m[8], const uint64_t key[2]);
void gift_64_vec_sbox_decrypt(uint64_t m[8], const uint64_t c[8], const uint64_t key[2]);

#endif
