#include <stdint.h>

#define ROUNDS_GIFT_64 28
#define ROUNDS_GIFT_128 40

// expose for benchmarking
void gift_64_subcells(uint8_t cipher_state[16]);
void gift_64_subcells_inv(uint8_t cipher_state[16]);
void gift_64_permute(uint8_t cipher_state[16]);
void gift_64_permute_inv(uint8_t cipher_state[16]);
void gift_64_generate_round_keys(uint8_t round_keys[ROUNDS_GIFT_64][16],
                                 const uint64_t key[2]);

void gift_128_subcells(uint8_t cipher_state[32]);
void gift_128_subcells_inv(uint8_t cipher_state[32]);
void gift_128_permute(uint8_t cipher_state[32]);
void gift_128_permute_inv(uint8_t cipher_state[32]);
void gift_128_generate_round_keys(uint8_t round_keys[ROUNDS_GIFT_128][32],
                                         const uint64_t key[2]);

void gift_64_encrypt(uint8_t c[8], const uint8_t m[8], const uint64_t key[2]);
void gift_64_decrypt(uint8_t m[8], const uint8_t c[8], const uint64_t key[2]);
void gift_128_encrypt(uint8_t c[16], const uint8_t m[16], const uint64_t key[2]);
void gift_128_decrypt(uint8_t m[16], const uint8_t c[16], const uint64_t key[2]);
