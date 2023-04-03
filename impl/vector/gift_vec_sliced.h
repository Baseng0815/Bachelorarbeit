#ifndef GIFT_VEC_SLICED_H
#define GIFT_VEC_SLICED_H

#include <stdint.h>
#include <arm_neon.h>

#define ROUNDS_GIFT_64 28

// expose for benchmarking
uint8x16_t shl(uint8x16_t v, int n);
uint8x16_t shr(uint8x16_t v, int n);
void gift_64_vec_sliced_swapmove(uint8x16_t *restrict a, uint8x16_t *restrict b, uint8x16_t m, int n);
void gift_64_vec_sliced_bits_pack(uint8x16x4_t m[restrict 2]);
void gift_64_vec_sliced_bits_unpack(uint8x16x4_t m[restrict 2]);

void gift_64_vec_sliced_subcells(uint8x16x4_t cipher_state[restrict 2]);
void gift_64_vec_sliced_subcells_inv(uint8x16x4_t cipher_state[restrict 2]);
void gift_64_vec_sliced_permute(uint8x16x4_t cipher_state[restrict 2]);
void gift_64_vec_sliced_permute_inv(uint8x16x4_t cipher_state[2]);
void gift_64_vec_sliced_generate_round_keys(uint8x16x4_t rks[restrict ROUNDS_GIFT_64][2],
                                            const uint64_t key[restrict 2]);

void gift_64_vec_sliced_init(void);

void gift_64_vec_sliced_encrypt(uint64_t c[restrict 16],
                                const uint64_t m[restrict 16],
                                const uint64_t key[restrict 2]);
void gift_64_vec_sliced_decrypt(uint64_t m[restrict 16],
                                const uint64_t c[restrict 16],
                                const uint64_t key[restrict 2]);

#endif
