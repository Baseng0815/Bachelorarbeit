#include <stdint.h>

uint64_t gift_64_encrypt(uint64_t m, const uint64_t key[2]);
uint64_t gift_64_decrypt(uint64_t c, const uint64_t key[2]);
void gift_128_encrypt(uint64_t c[2], const uint64_t m[2], const uint64_t key[2]);
void gift_128_decrypt(uint64_t m[2], const uint64_t c[2], const uint64_t key[2]);
