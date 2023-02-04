#include "gift_neon.h"

void gift_64_neon_encrypt(uint64_t m[8], uint64_t c[8], const uint64_t key[2]);
void gift_64_neon_decrypt(uint64_t m[8], uint64_t c[8], const uint64_t key[2]);
