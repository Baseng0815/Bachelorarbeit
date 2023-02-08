#include <stdint.h>

uint64_t gift_64_encrypt(uint8_t c[8], const uint8_t m[8], const uint64_t key[2]);
uint64_t gift_64_decrypt(uint8_t m[8], const uint8_t c[8], const uint64_t key[2]);
void gift_128_encrypt(uint8_t c[16], const uint8_t m[16], const uint64_t key[2]);
void gift_128_decrypt(uint8_t m[16], const uint8_t c[16], const uint64_t key[2]);
