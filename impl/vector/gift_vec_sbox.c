#include "gift_vec_sbox.h"

#include <stdint.h>
#include <arm_neon.h>
#include <string.h>

static uint8x16_t sbox_vec;
static uint8x16_t sbox_vec_inv;

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

uint8x16_t gift_64_vec_sbox_subcells(const uint8x16_t cipher_state)
{
        return vqtbl1q_u8(sbox_vec, cipher_state);
}

uint8x16_t gift_64_vec_sbox_subcells_inv(const uint8x16_t cipher_state)
{
        return vqtbl1q_u8(sbox_vec_inv, cipher_state);
}

uint8x16_t gift_64_vec_sbox_permute(const uint8x16_t cipher_state)
{
        uint8x16_t new_cipher_state = vdupq_n_u8(0);

        // extract from cipher_state
        uint8_t nibbles[16];
        for (size_t i = 0; i < 16; i++) {
                nibbles[i] = vgetq_lane_u8(cipher_state, i);
        }

        for (size_t i = 0; i < 64; i++) {
                int bit = vgetq_lane_u8(cipher_state, i % 8);
                /* int bit = (cipher_state >> i) & 0x1; */
                /* new_cipher_state |= (uint8x16_t)bit << perm_64[i]; */
        }

        return new_cipher_state;
}

uint8x16_t gift_64_vec_sbox_permute_inv(const uint8x16_t cipher_state)
{
        uint8x16_t new_cipher_state = vdupq_n_u8(0);

        /* for (size_t i = 0; i < 64; i++) { */
        /*         int bit = (cipher_state >> i) & 0x1; */
        /*         new_cipher_state |= (uint8x16_t)bit << perm_64[i]; */
        /* } */

        return new_cipher_state;
}

/* void gift_64_vec_sbox_generate_round_keys(uint8x16_t round_keys[ROUNDS_GIFT_64], */
/*                                           const uint8x16_t key[2]) */
/* { */

/* } */

uint64_t gift_64_vec_sbox_encrypt(const uint64_t m, const uint64_t key[2])
{
        // construct sbox_vec
        vsetq_lane_u8(0x1, sbox_vec, 0);
        vsetq_lane_u8(0xa, sbox_vec, 1);
        vsetq_lane_u8(0x4, sbox_vec, 2);
        vsetq_lane_u8(0xc, sbox_vec, 3);
        vsetq_lane_u8(0x6, sbox_vec, 4);
        vsetq_lane_u8(0xf, sbox_vec, 5);
        vsetq_lane_u8(0x3, sbox_vec, 6);
        vsetq_lane_u8(0x9, sbox_vec, 7);
        vsetq_lane_u8(0x2, sbox_vec, 8);
        vsetq_lane_u8(0xd, sbox_vec, 9);
        vsetq_lane_u8(0xb, sbox_vec, 10);
        vsetq_lane_u8(0x7, sbox_vec, 11);
        vsetq_lane_u8(0x5, sbox_vec, 12);
        vsetq_lane_u8(0x0, sbox_vec, 13);
        vsetq_lane_u8(0x8, sbox_vec, 14);
        vsetq_lane_u8(0xe, sbox_vec, 15);
}

uint64_t gift_64_vec_sbox_decrypt(const uint64_t c, const uint64_t key[2])
{
        // construct sbox_vec_inv
        vsetq_lane_u8(0xd, sbox_vec_inv, 0);
        vsetq_lane_u8(0x0, sbox_vec_inv, 1);
        vsetq_lane_u8(0x8, sbox_vec_inv, 2);
        vsetq_lane_u8(0x6, sbox_vec_inv, 3);
        vsetq_lane_u8(0x2, sbox_vec_inv, 4);
        vsetq_lane_u8(0xc, sbox_vec_inv, 5);
        vsetq_lane_u8(0x4, sbox_vec_inv, 6);
        vsetq_lane_u8(0xb, sbox_vec_inv, 7);
        vsetq_lane_u8(0xe, sbox_vec_inv, 8);
        vsetq_lane_u8(0x7, sbox_vec_inv, 9);
        vsetq_lane_u8(0x1, sbox_vec_inv, 10);
        vsetq_lane_u8(0xa, sbox_vec_inv, 11);
        vsetq_lane_u8(0x3, sbox_vec_inv, 12);
        vsetq_lane_u8(0x9, sbox_vec_inv, 13);
        vsetq_lane_u8(0xf, sbox_vec_inv, 14);
        vsetq_lane_u8(0x5, sbox_vec_inv, 15);
}
