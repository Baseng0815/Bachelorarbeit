#ifndef GIFT_TABLE_H
#define GIFT_TABLE_H

#include <stdint.h>

#define ROUNDS_GIFT_64 28

void gift_64_table_generate_round_keys(uint64_t round_keys[restrict ROUNDS_GIFT_64],
                                 const uint64_t key[restrict 2]);

uint64_t gift_64_table_subperm(const uint64_t cipher_state);

// can only encrypt using table technique!
uint64_t gift_64_table_encrypt(const uint64_t m, const uint64_t key[restrict 2]);

#endif
