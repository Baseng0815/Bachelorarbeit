#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gift.h""

// implementation of GIFT-64 in pure C, without any intrinsics

static const int sbox[] = {
        0x1, 0xa, 0x4, 0xc, 0x6, 0xf, 0x3, 0x9,
        0x2, 0xd, 0xb, 0x7, 0x5, 0x0, 0x8, 0xe
};

static const int sbox_inv[] = {
        0xd, 0x0, 0x8, 0x6, 0x2, 0xc, 0x4, 0xb,
        0xe, 0x7, 0x1, 0xa, 0x3, 0x9, 0xf, 0x5
};

static const size_t perm_64[] = {
        0, 17, 34, 51, 48, 1, 18, 35, 32, 49, 2, 19, 16, 33, 50, 3,
        4, 21, 38, 55, 52, 5, 22, 39, 36, 53, 6, 23, 20, 37, 54, 7,
        8, 25, 42, 59, 56, 9, 26, 43, 40, 57, 10, 27, 24, 41, 58, 11,
        12, 29, 46, 63, 60, 13, 30, 47, 44, 61, 14, 31, 28, 45, 62, 15
};

static const size_t perm_64_inv[] = {
        0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63,
        12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59,
        8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55,
        4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51
};

static const size_t perm_128[] = {
        0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3,
        4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7,
        8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11,
        12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15,
        16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19,
        20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23,
        24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27,
        28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31,
};

static const size_t perm_128_inv[] = {
        0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63,
        64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127,
        12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59,
        76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123,
        8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55,
        72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119,
        4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51,
        68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115
};

static const int round_constant[] = {
        // rounds 0-15
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E,
        // rounds 16-31
        0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E, 0x1C, 0x38,
        // rounds 32-47
        0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
};

static void print_bytes(uint8_t b[], size_t n);

void print_bytes(uint8_t b[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                printf("%01x", b[i] & 0xf);
                if (i % 4 == 3)
                        printf(" ");
        }

        printf("\n");
}

void gift_64_subcells(uint8_t cipher_state[16])
{
        for (size_t i = 0; i < 16; i++) {
                cipher_state[i] = sbox[cipher_state[i]];
        }
}

void gift_64_subcells_inv(uint8_t cipher_state[16])
{
        for (size_t i = 0; i < 16; i++) {
                cipher_state[i] = sbox_inv[cipher_state[i]];
        }
}

void gift_64_permute(uint8_t cipher_state[16])
{
        uint8_t old_cipher_state[16];
        memcpy(old_cipher_state, cipher_state, sizeof(old_cipher_state));
        memset(cipher_state, 0, 16);

        for (size_t i = 0; i < 64; i++) {
                size_t pi = perm_64[i];
                int bit = (old_cipher_state[i / 4] >> (i % 4)) & 0x1;
                cipher_state[pi / 4] |= bit << (pi % 4);
        }
}

void gift_64_permute_inv(uint8_t cipher_state[16])
{
        uint8_t old_cipher_state[16];
        memcpy(old_cipher_state, cipher_state, sizeof(old_cipher_state));
        memset(cipher_state, 0, 16);

        for (size_t i = 0; i < 64; i++) {
                size_t pi = perm_64_inv[i];
                int bit = (old_cipher_state[i / 4] >> (i % 4)) & 0x1;
                cipher_state[pi / 4] |= (uint64_t)bit << (pi % 4);
        }
}

void gift_64_generate_round_keys(uint8_t round_keys[ROUNDS_GIFT_64][16], const uint64_t key[2])
{
        uint64_t key_state[] = {key[0], key[1]};
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                int v = (key_state[0] >> 0 ) & 0xffff;
                int u = (key_state[0] >> 16) & 0xffff;

                // add round key (RK=U||V)
                memset(&round_keys[round][0], 0, 16);
                for (size_t i = 0; i < 16; i++) {
                        int key_bit_v   = (v >> i)  & 0x1;
                        int key_bit_u   = (u >> i)  & 0x1;
                        round_keys[round][i] ^= key_bit_v << 0;
                        round_keys[round][i] ^= key_bit_u << 1;
                }

                // add single bit
                round_keys[round][15] ^= 1 << 3;

                // add round constants
                round_keys[round][0] ^= ((round_constant[round] >> 0) & 0x1) << 3;
                round_keys[round][1] ^= ((round_constant[round] >> 1) & 0x1) << 3;
                round_keys[round][2] ^= ((round_constant[round] >> 2) & 0x1) << 3;
                round_keys[round][3] ^= ((round_constant[round] >> 3) & 0x1) << 3;
                round_keys[round][4] ^= ((round_constant[round] >> 4) & 0x1) << 3;
                round_keys[round][5] ^= ((round_constant[round] >> 5) & 0x1) << 3;

                // update key state
                int k0 = (key_state[0] >> 0 ) & 0xffffUL;
                int k1 = (key_state[0] >> 16) & 0xffffUL;
                k0 = (k0 >> 12) | ((k0 & 0xfff) << 4);
                k1 = (k1 >> 2 ) | ((k1 & 0x3  ) << 14);
                key_state[0] >>= 32;
                key_state[0] |= (key_state[1] & 0xffffffffUL) << 32;
                key_state[1] >>= 32;
                key_state[1] |= ((uint64_t)k0 << 32) | ((uint64_t)k1 << 48);
        }
}

void gift_128_subcells(uint8_t cipher_state[32])
{
        gift_64_subcells(&cipher_state[0]);
        gift_64_subcells(&cipher_state[16]);
}

void gift_128_subcells_inv(uint8_t cipher_state[32])
{
        gift_64_subcells_inv(&cipher_state[0]);
        gift_64_subcells_inv(&cipher_state[16]);
}

void gift_128_permute(uint8_t cipher_state[32])
{
        uint8_t old_cipher_state[32];
        memcpy(old_cipher_state, cipher_state, sizeof(old_cipher_state));
        memset(cipher_state, 0, 32);

        for (size_t i = 0; i < 128; i++) {
                size_t pi = perm_128[i];
                int bit = (old_cipher_state[i / 4] >> (i % 4)) & 0x1;
                cipher_state[pi / 4] |= bit << (pi % 4);
        }
}

void gift_128_permute_inv(uint8_t cipher_state[32])
{
        uint8_t old_cipher_state[32];
        memcpy(old_cipher_state, cipher_state, sizeof(old_cipher_state));
        memset(cipher_state, 0, 32);

        for (size_t i = 0; i < 128; i++) {
                size_t pi = perm_128_inv[i];
                int bit = (old_cipher_state[i / 4] >> (i % 4)) & 0x1;
                cipher_state[pi / 4] |= (uint64_t)bit << (pi % 4);
        }
}

void gift_128_generate_round_keys(uint8_t round_keys[ROUNDS_GIFT_128][32], const uint64_t key[2])
{
        uint64_t key_state[] = {key[0], key[1]};
        for (int round = 0; round < ROUNDS_GIFT_128; round++) {
                uint32_t v = key_state[0] & 0xffffffff;
                uint32_t u = key_state[1] & 0xffffffff;

                // add round key (RK=U||V)
                memset(&round_keys[round][0], 0, 32);
                for (size_t i = 0; i < 32; i++) {
                        int key_bit_v   = (v >> i)  & 0x1;
                        int key_bit_u   = (u >> i)  & 0x1;
                        round_keys[round][i] ^= key_bit_v << 1;
                        round_keys[round][i] ^= key_bit_u << 2;
                }

                // add single bit
                round_keys[round][31] ^= 1 << 3;

                // add round constants
                round_keys[round][0] ^= ((round_constant[round] >> 0) & 0x1) << 3;
                round_keys[round][1] ^= ((round_constant[round] >> 1) & 0x1) << 3;
                round_keys[round][2] ^= ((round_constant[round] >> 2) & 0x1) << 3;
                round_keys[round][3] ^= ((round_constant[round] >> 3) & 0x1) << 3;
                round_keys[round][4] ^= ((round_constant[round] >> 4) & 0x1) << 3;
                round_keys[round][5] ^= ((round_constant[round] >> 5) & 0x1) << 3;

                // update key state
                int k0 = (key_state[0] >> 0 ) & 0xffffUL;
                int k1 = (key_state[0] >> 16) & 0xffffUL;
                k0 = (k0 >> 12) | ((k0 & 0xfff) << 4);
                k1 = (k1 >> 2 ) | ((k1 & 0x3  ) << 14);
                key_state[0] >>= 32;
                key_state[0] |= (key_state[1] & 0xffffffffUL) << 32;
                key_state[1] >>= 32;
                key_state[1] |= ((uint64_t)k0 << 32) | ((uint64_t)k1 << 48);
        }
}

void gift_64_encrypt(uint8_t c_[8], const uint8_t m[8], const uint64_t key[2])
{
        uint8_t c[16]; // one byte per s-box
        for (size_t i = 0; i < 16; i++) {
                c[i] = (m[i / 2] >> (i % 2) * 4) & 0xf;
        }

        // generate round keys
        uint8_t round_keys[ROUNDS_GIFT_64][16];
        gift_64_generate_round_keys(round_keys, key);

        // round loop
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                gift_64_subcells(c);
#ifdef DEBUG
                printf("GIFT_64_ENCRYPT round %2d, subcells:      ", round);
                print_bytes(c, 16);
#endif
                gift_64_permute(c);
#ifdef DEBUG
                printf("GIFT_64_ENCRYPT round %2d, permute:       ", round);
                print_bytes(c, 16);
#endif
                for (size_t i = 0; i < 16; i++) {
                        c[i] ^= round_keys[round][i];
                }
#ifdef DEBUG
                printf("GIFT_64_ENCRYPT round %2d, add round key: ", round);
                print_bytes(c, 16);
#endif
        }

        for (size_t i = 0; i < 8; i++) {
                c_[i] = c[2 * i + 0] | (c[2 * i + 1] << 4);
        }
}

void gift_64_decrypt(uint8_t m_[8], const uint8_t c[8], const uint64_t key[2])
{
        // generate round keys
        uint8_t round_keys[ROUNDS_GIFT_64][16];
        gift_64_generate_round_keys(round_keys, key);

        uint8_t m[16]; // one byte per s-box
        for (size_t i = 0; i < 16; i++) {
                m[i] = (c[i / 2] >> (i % 2) * 4) & 0xf;
        }

        // round loop (in reverse)
        for (int round = ROUNDS_GIFT_64 - 1; round >= 0; round--) {
                for (size_t i = 0; i < 16; i++) {
                        m[i] ^= round_keys[round][i];
                }
#ifdef DEBUG
                printf("GIFT_64_DECRYPT round %2d, add round key: ", round);
                print_bytes(m, 16);
#endif
                gift_64_permute_inv(m);
#ifdef DEBUG
                printf("GIFT_64_DECRYPT round %2d, permute inv:   ", round);
                print_bytes(m, 16);
#endif
                gift_64_subcells_inv(m);
#ifdef DEBUG
                printf("GIFT_64_DECRYPT round %2d, subcells inv:  ", round);
                print_bytes(m, 16);
#endif
        }

        for (size_t i = 0; i < 8; i++) {
                m_[i] = m[2 * i + 0] | (m[2 * i + 1] << 4);
        }
}

void gift_128_encrypt(uint8_t c_[16], const uint8_t m[16], const uint64_t key[2])
{
        uint8_t c[32]; // one byte per s-box
        for (size_t i = 0; i < 32; i++) {
                c[i] = (m[i / 2] >> (i % 2) * 4) & 0xf;
        }

        // generate round keys
        uint8_t round_keys[ROUNDS_GIFT_128][32];
        gift_128_generate_round_keys(round_keys, key);

        // round loop
        for (int round = 0; round < ROUNDS_GIFT_128; round++) {
                gift_128_subcells(c);
#ifdef DEBUG
                printf("GIFT_128_ENCRYPT round %2d, subcells:      ", round);
                print_bytes(c, 32);
#endif
                gift_128_permute(c);
#ifdef DEBUG
                printf("GIFT_128_ENCRYPT round %2d, permute:       ", round);
                print_bytes(c, 32);
#endif
                for (size_t i = 0; i < 32; i++) {
                        c[i] ^= round_keys[round][i];
                }
#ifdef DEBUG
                printf("GIFT_128_ENCRYPT round %2d, add round key: ", round);
                print_bytes(c, 32);
#endif
        }

        for (size_t i = 0; i < 16; i++) {
                c_[i] = c[2 * i + 0] | (c[2 * i + 1] << 4);
        }
}

void gift_128_decrypt(uint8_t m_[16], const uint8_t c[16], const uint64_t key[2])
{
        uint8_t m[32]; // one byte per s-box
        for (size_t i = 0; i < 32; i++) {
                m[i] = (c[i / 2] >> (i % 2) * 4) & 0xf;
        }

        // generate round keys
        uint8_t round_keys[ROUNDS_GIFT_128][32];
        gift_128_generate_round_keys(round_keys, key);

        // round loop (in reverse)
        for (int round = ROUNDS_GIFT_128 - 1; round >= 0; round--) {
                for (size_t i = 0; i < 32; i++) {
                        m[i] ^= round_keys[round][i];
                }
#ifdef DEBUG
                printf("GIFT_128_DECRYPT round %2d, add round key: ", round);
                print_bytes(m, 32);
#endif
                gift_128_permute_inv(m);
#ifdef DEBUG
                printf("GIFT_128_DECRYPT round %2d, permute inv:   ", round);
                print_bytes(m, 32);
#endif
                gift_128_subcells_inv(m);
#ifdef DEBUG
                printf("GIFT_128_DECRYPT round %2d, subcells inv:  ", round);
                print_bytes(m, 32);
#endif
        }

        for (size_t i = 0; i < 16; i++) {
                m_[i] = m[2 * i + 0] | (m[2 * i + 1] << 4);
        }
}
