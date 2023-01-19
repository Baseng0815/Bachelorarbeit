#include <stdint.h>
#include <stdio.h>

#define ROUNDS_GIFT_64 28

// implementation of GIFT-64 in pure C, without any intrinsics

static const int sbox[] = {
        0x1, 0xa, 0x4, 0xc, 0x6, 0xf, 0x3, 0x9,
        0x2, 0xd, 0xb, 0x7, 0x5, 0x0, 0x8, 0xe
};

static const int sbox_inv[] = {
        0xd, 0x0, 0x8, 0x6, 0x2, 0xc, 0x4, 0xb,
        0xe, 0x7, 0x1, 0xa, 0x3, 0x9, 0xf, 0x5
};

static const size_t perm[] = {
        0, 17, 34, 51, 48, 1, 18, 35, 32, 49, 2, 19, 16, 33, 50, 3,
        4, 21, 38, 55, 52, 5, 22, 39, 36, 53, 6, 23, 20, 37, 54, 7,
        8, 25, 42, 59, 56, 9, 26, 43, 40, 57, 10, 27, 24, 41, 58, 11,
        12, 29, 46, 63, 60, 13, 30, 47, 44, 61, 14, 31, 28, 45, 62, 15
};

static const size_t perm_inv[] = {
        0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63,
        12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59,
        8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55,
        4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51
};

static const int round_constant[] = {
        // rounds 0-15
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E,
        // rounds 16-31
        0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E, 0x1C, 0x38,
        // rounds 32-47
        0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
};

static uint64_t gift_64_subcells(uint64_t cipher_state)
{
        uint64_t new_cipher_state = 0UL;
        for (size_t i = 0; i < 16; i++) {
                int nibble = (cipher_state >> (i * 4)) & 0xf;
                nibble = sbox[nibble];
                new_cipher_state |= (uint64_t)nibble << i * 4;
        }

        return new_cipher_state;
}

static uint64_t gift_64_subcells_inv(uint64_t cipher_state)
{
        uint64_t new_cipher_state = 0UL;
        for (size_t i = 0; i < 16; i++) {
                int nibble = (cipher_state >> (i * 4)) & 0xf;
                nibble = sbox_inv[nibble];
                new_cipher_state |= (uint64_t)nibble << i * 4;
        }

        return new_cipher_state;
}

static uint64_t gift_64_permbits(uint64_t cipher_state)
{
        uint64_t new_cipher_state = 0UL;
        for (size_t i = 0; i < 64; i++) {
                int bit = (cipher_state >> i) & 0x1;
                new_cipher_state |= (uint64_t)bit << perm[i];
        }

        return new_cipher_state;
}

static uint64_t gift_64_permbits_inv(uint64_t cipher_state)
{
        uint64_t new_cipher_state = 0UL;
        for (size_t i = 0; i < 64; i++) {
                int bit = (cipher_state >> i) & 0x1;
                new_cipher_state |= (uint64_t)bit << perm_inv[i];
        }

        return new_cipher_state;
}

static void gift_64_generate_round_keys(uint64_t *round_keys,
                                        const uint64_t key[2],
                                        int rounds)
{
        uint64_t key_state[] = {key[0], key[1]};
        for (int round = 0; round < rounds; round++) {
                uint32_t v = (key_state[0] >> 0 ) & 0xffff;
                uint32_t u = (key_state[0] >> 16) & 0xffff;

                // add round key (RK=U||V)
                round_keys[round] = 0;
                for (size_t i = 0; i < 16; i++) {
                        int key_bit_v   = (v >> i)  & 0x1;
                        int key_bit_u   = (u >> i)  & 0x1;
                        round_keys[round] ^= (uint64_t)key_bit_v << (i * 4);
                        round_keys[round] ^= (uint64_t)key_bit_u << (i * 4 + 1);
                }

                // add single bit
                round_keys[round] ^= (1UL << 63);

                // add round constants
                round_keys[round] ^= (uint64_t)((round_constant[round] >> 0) & 0x1) << 3;
                round_keys[round] ^= (uint64_t)((round_constant[round] >> 1) & 0x1) << 7;
                round_keys[round] ^= (uint64_t)((round_constant[round] >> 2) & 0x1) << 11;
                round_keys[round] ^= (uint64_t)((round_constant[round] >> 3) & 0x1) << 15;
                round_keys[round] ^= (uint64_t)((round_constant[round] >> 4) & 0x1) << 19;
                round_keys[round] ^= (uint64_t)((round_constant[round] >> 5) & 0x1) << 23;

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

uint64_t gift_64_encrypt(uint64_t m, const uint64_t key[2])
{
        // generate round keys
        uint64_t round_keys[ROUNDS_GIFT_64];
        gift_64_generate_round_keys(round_keys, key, ROUNDS_GIFT_64);

        // round loop
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                m = gift_64_subcells(m);
#ifdef DEBUG
                printf("GIFT_64_ENCRYPT round %d, subcells: %lx\n",
                       round, m);
#endif
                m = gift_64_permbits(m);
#ifdef DEBUG
                printf("GIFT_64_ENCRYPT round %d, permbits: %lx\n",
                       round, m);
#endif
                m ^= round_keys[round];
#ifdef DEBUG
                printf("GIFT_64_ENCRYPT round %d, add round key: %lx\n",
                       round, m);
#endif
        }

        return m;
}

uint64_t gift_64_decrypt(uint64_t m, const uint64_t key[2])
{
        // generate round keys
        uint64_t round_keys[ROUNDS_GIFT_64];
        gift_64_generate_round_keys(round_keys, key, ROUNDS_GIFT_64);

        // round loop (in reverse)
        for (int round = ROUNDS_GIFT_64 - 1; round >= 0; round--) {
                m ^= round_keys[round];
#ifdef DEBUG
                printf("GIFT_64_DECRYPT round %d, add round key: %lx\n",
                       round, m);
#endif
                m = gift_64_permbits_inv(m);
#ifdef DEBUG
                printf("GIFT_64_DECRYPT round %d, permbits inv: %lx\n",
                       round, m);
#endif
                m = gift_64_subcells_inv(m);
#ifdef DEBUG
                printf("GIFT_64_DECRYPT round %d, subcells inv: %lx\n",
                       round, m);
#endif
        }

        return m;
}
