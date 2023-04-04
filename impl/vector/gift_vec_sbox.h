#ifndef GIFT_VEC_SBOX_H
#define GIFT_VEC_SBOX_H

#include <stdint.h>
#include <arm_neon.h>

#define ROUNDS_GIFT_64 28

// expose for benchmarking
uint8x16_t gift_64_vec_sbox_bits_pack(const uint64_t a);
uint64_t   gift_64_vec_sbox_bits_unpack(const uint8x16_t a);
uint8x16_t gift_64_vec_sbox_subcells(const uint8x16_t cipher_state);
uint8x16_t gift_64_vec_sbox_subcells_inv(const uint8x16_t cipher_state);
uint8x16_t gift_64_vec_sbox_permute(const uint8x16_t cipher_state);
uint8x16_t gift_64_vec_sbox_permute_inv(const uint8x16_t cipher_state);
void       gift_64_vec_sbox_generate_round_keys(uint8x16_t rks[ROUNDS_GIFT_64],
                                                const uint64_t key[restrict 2]);

// construct tables
void gift_64_vec_sbox_init(void);

uint64_t gift_64_vec_sbox_encrypt(const uint64_t m,
                                  const uint8x16_t rks[restrict ROUNDS_GIFT_64]);
uint64_t gift_64_vec_sbox_decrypt(const uint64_t c,
                                  const uint8x16_t rks[restrict ROUNDS_GIFT_64]);

#endif
