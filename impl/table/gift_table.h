#ifndef GIFT_TABLE_H
#define GIFT_TABLE_H

#include <stdint.h>

#define ROUNDS_GIFT_64 28

void gift_64_table_generate_round_keys(uint8_t round_keys[ROUNDS_GIFT_64][16],
                                 const uint64_t key[2]);

void gift_64_table_subperm(uint8_t cipher_state[8]);

void gift_64_table_encrypt(uint8_t c_[8], const uint8_t m[8], const uint64_t key[2]);
void gift_64_table_decrypt(uint8_t m_[8], const uint8_t c[8], const uint64_t key[2]);

#endif
