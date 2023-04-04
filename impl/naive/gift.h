#ifndef GIFT_H
#define GIFT_H

#include <stdint.h>

#define ROUNDS_GIFT_64 28
#define ROUNDS_GIFT_128 40

// expose for benchmarking
uint64_t gift_64_subcells(const uint64_t cipher_state);
uint64_t gift_64_subcells_inv(const uint64_t cipher_state);
uint64_t gift_64_permute(const uint64_t cipher_state);
uint64_t gift_64_permute_inv(const uint64_t cipher_state);
void gift_64_generate_round_keys(uint64_t round_keys[ROUNDS_GIFT_64],
                                 const uint64_t key[2]);

void gift_128_subcells(uint8_t cipher_state[restrict 32]);
void gift_128_subcells_inv(uint8_t cipher_state[restrict 32]);
void gift_128_permute(uint8_t cipher_state[restrict 32]);
void gift_128_permute_inv(uint8_t cipher_state[restrict 32]);
void gift_128_generate_round_keys(uint8_t round_keys[restrict ROUNDS_GIFT_128][32],
                                         const uint64_t key[restrict 2]);

uint64_t gift_64_encrypt(const uint64_t m,
                         const uint64_t round_keys[restrict ROUNDS_GIFT_64]);
uint64_t gift_64_decrypt(const uint64_t c,
                         const uint64_t round_keys[restrict ROUNDS_GIFT_64]);
void gift_128_encrypt(uint8_t c[restrict 16],
                      const uint8_t m[restrict 16],
                      const uint8_t round_keys[restrict ROUNDS_GIFT_128][32]);
void gift_128_decrypt(uint8_t m[restrict 16],
                      const uint8_t c[restrict 16],
                      const uint8_t round_keys[restrict ROUNDS_GIFT_128][32]);

#endif
