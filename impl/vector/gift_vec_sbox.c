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

static const int round_constant[] = {
        // rounds 0-15
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E,
        // rounds 16-31
        0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E, 0x1C, 0x38,
        // rounds 32-47
        0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
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
        // collect into 64-bit register (faster)
        uint64_t new_cipher_state = 0UL;

        // S-box 0-7
        uint64_t boxes = vgetq_lane_u64(cipher_state, 0);
        for (size_t box = 0; box < 8; box++) {
                for (size_t i = 0; i < 4; i++) {
                        int bit = (boxes >> (box * 8 + i)) & 0x1;
                        new_cipher_state |= (uint64_t)bit << perm_64[box * 4 + i];
                }
        }

        // S-box 8-15
        boxes = vgetq_lane_u64(cipher_state, 1);
        for (size_t box = 0; box < 8; box++) {
                for (size_t i = 0; i < 4; i++) {
                        int bit = (boxes >> (box * 8 + i)) & 0x1;
                        new_cipher_state |= (uint64_t)bit << perm_64[(box + 8) * 4 + i];
                }
        }

        // load into vector register (yes, this is slow. too bad!)
        uint8x16_t ret;
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 0) & 0xf, ret, 0);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 1) & 0xf, ret, 1);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 2) & 0xf, ret, 2);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 3) & 0xf, ret, 3);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 4) & 0xf, ret, 4);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 5) & 0xf, ret, 5);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 6) & 0xf, ret, 6);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 7) & 0xf, ret, 7);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 8) & 0xf, ret, 8);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 9) & 0xf, ret, 9);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 10) & 0xf, ret, 10);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 11) & 0xf, ret, 11);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 12) & 0xf, ret, 12);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 13) & 0xf, ret, 13);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 14) & 0xf, ret, 14);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 15) & 0xf, ret, 15);

        return ret;
}

uint8x16_t gift_64_vec_sbox_permute_inv(const uint8x16_t cipher_state)
{
        // collect into 64-bit register (faster)
        uint64_t new_cipher_state = 0;

        // S-box 0-7
        uint64_t boxes = vgetq_lane_u64(cipher_state, 0);
        for (size_t box = 0; box < 8; box++) {
                for (size_t i = 0; i < 4; i++) {
                        int bit = (boxes >> (box * 8 + i)) & 0x1;
                        new_cipher_state |= (uint64_t)bit << perm_64_inv[box * 4 + i];
                }
        }

        // S-box 8-15
        boxes = vgetq_lane_u64(cipher_state, 1);
        for (size_t box = 0; box < 8; box++) {
                for (size_t i = 0; i < 4; i++) {
                        int bit = (boxes >> (box * 8 + i)) & 0x1;
                        new_cipher_state |= (uint64_t)bit << perm_64_inv[(box + 8) * 4 + i];
                }
        }

        // load into vector register (yes, this is slow. too bad!)
        uint8x16_t ret;
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 0) & 0xf, ret, 0);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 1) & 0xf, ret, 1);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 2) & 0xf, ret, 2);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 3) & 0xf, ret, 3);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 4) & 0xf, ret, 4);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 5) & 0xf, ret, 5);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 6) & 0xf, ret, 6);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 7) & 0xf, ret, 7);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 8) & 0xf, ret, 8);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 9) & 0xf, ret, 9);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 10) & 0xf, ret, 10);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 11) & 0xf, ret, 11);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 12) & 0xf, ret, 12);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 13) & 0xf, ret, 13);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 14) & 0xf, ret, 14);
        ret = vsetq_lane_u8((new_cipher_state >> 4 * 15) & 0xf, ret, 15);

        return ret;
}

void gift_64_vec_sbox_generate_round_keys(uint8x16_t round_keys[ROUNDS_GIFT_64],
                                          const uint64_t key[2])
{
        uint64_t key_state[] = {key[0], key[1]};
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                int v = (key_state[0] >> 0 ) & 0xffff;
                int u = (key_state[0] >> 16) & 0xffff;

                // add round key (RK=U||V)
                uint64_t round_key = 0UL;
                for (size_t i = 0; i < 16; i++) {
                        int key_bit_v   = (v >> i)  & 0x1;
                        int key_bit_u   = (u >> i)  & 0x1;
                        round_key ^= (uint64_t)key_bit_v << (i * 4 + 0);
                        round_key ^= (uint64_t)key_bit_u << (i * 4 + 1);
                }

                // add single bit
                round_key ^= 1UL << 63;

                // add round constants
                round_key ^= ((round_constant[round] >> 0) & 0x1) << 3;
                round_key ^= ((round_constant[round] >> 1) & 0x1) << 7;
                round_key ^= ((round_constant[round] >> 2) & 0x1) << 11;
                round_key ^= ((round_constant[round] >> 3) & 0x1) << 15;
                round_key ^= ((round_constant[round] >> 4) & 0x1) << 19;
                round_key ^= ((round_constant[round] >> 5) & 0x1) << 23;

                // pack into vector register
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 0) & 0xf, round_keys[round], 0);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 1) & 0xf, round_keys[round], 1);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 2) & 0xf, round_keys[round], 2);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 3) & 0xf, round_keys[round], 3);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 4) & 0xf, round_keys[round], 4);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 5) & 0xf, round_keys[round], 5);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 6) & 0xf, round_keys[round], 6);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 7) & 0xf, round_keys[round], 7);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 8) & 0xf, round_keys[round], 8);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 9) & 0xf, round_keys[round], 9);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 10) & 0xf, round_keys[round], 10);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 11) & 0xf, round_keys[round], 11);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 12) & 0xf, round_keys[round], 12);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 13) & 0xf, round_keys[round], 13);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 14) & 0xf, round_keys[round], 14);
                round_keys[round] = vsetq_lane_u8((round_key >> 4 * 15) & 0xf, round_keys[round], 15);

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

uint64_t gift_64_vec_sbox_encrypt(const uint64_t m, const uint64_t key[2])
{
        // construct sbox_vec
        sbox_vec = vsetq_lane_u8(0x1, sbox_vec, 0);
        sbox_vec = vsetq_lane_u8(0xa, sbox_vec, 1);
        sbox_vec = vsetq_lane_u8(0x4, sbox_vec, 2);
        sbox_vec = vsetq_lane_u8(0xc, sbox_vec, 3);
        sbox_vec = vsetq_lane_u8(0x6, sbox_vec, 4);
        sbox_vec = vsetq_lane_u8(0xf, sbox_vec, 5);
        sbox_vec = vsetq_lane_u8(0x3, sbox_vec, 6);
        sbox_vec = vsetq_lane_u8(0x9, sbox_vec, 7);
        sbox_vec = vsetq_lane_u8(0x2, sbox_vec, 8);
        sbox_vec = vsetq_lane_u8(0xd, sbox_vec, 9);
        sbox_vec = vsetq_lane_u8(0xb, sbox_vec, 10);
        sbox_vec = vsetq_lane_u8(0x7, sbox_vec, 11);
        sbox_vec = vsetq_lane_u8(0x5, sbox_vec, 12);
        sbox_vec = vsetq_lane_u8(0x0, sbox_vec, 13);
        sbox_vec = vsetq_lane_u8(0x8, sbox_vec, 14);
        sbox_vec = vsetq_lane_u8(0xe, sbox_vec, 15);

        // pack into vector register
        uint8x16_t c;
        c = vsetq_lane_u8((m >> 4 * 0) & 0xf, c, 0);
        c = vsetq_lane_u8((m >> 4 * 1) & 0xf, c, 1);
        c = vsetq_lane_u8((m >> 4 * 2) & 0xf, c, 2);
        c = vsetq_lane_u8((m >> 4 * 3) & 0xf, c, 3);
        c = vsetq_lane_u8((m >> 4 * 4) & 0xf, c, 4);
        c = vsetq_lane_u8((m >> 4 * 5) & 0xf, c, 5);
        c = vsetq_lane_u8((m >> 4 * 6) & 0xf, c, 6);
        c = vsetq_lane_u8((m >> 4 * 7) & 0xf, c, 7);
        c = vsetq_lane_u8((m >> 4 * 8) & 0xf, c, 8);
        c = vsetq_lane_u8((m >> 4 * 9) & 0xf, c, 9);
        c = vsetq_lane_u8((m >> 4 * 10) & 0xf, c, 10);
        c = vsetq_lane_u8((m >> 4 * 11) & 0xf, c, 11);
        c = vsetq_lane_u8((m >> 4 * 12) & 0xf, c, 12);
        c = vsetq_lane_u8((m >> 4 * 13) & 0xf, c, 13);
        c = vsetq_lane_u8((m >> 4 * 14) & 0xf, c, 14);
        c = vsetq_lane_u8((m >> 4 * 15) & 0xf, c, 15);

        // generate round keys
        uint8x16_t round_keys[ROUNDS_GIFT_64];
        gift_64_vec_sbox_generate_round_keys(round_keys, key);

        // round loop
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                c = gift_64_vec_sbox_subcells(c);
                c = gift_64_vec_sbox_permute(c);
                c = veorq_u8(c, round_keys[round]);
        }

        // unpack
        uint64_t ret = 0UL;
        ret |= (uint64_t)vgetq_lane_u8(c, 0) << 4 * 0;
        ret |= (uint64_t)vgetq_lane_u8(c, 1) << 4 * 1;
        ret |= (uint64_t)vgetq_lane_u8(c, 2) << 4 * 2;
        ret |= (uint64_t)vgetq_lane_u8(c, 3) << 4 * 3;
        ret |= (uint64_t)vgetq_lane_u8(c, 4) << 4 * 4;
        ret |= (uint64_t)vgetq_lane_u8(c, 5) << 4 * 5;
        ret |= (uint64_t)vgetq_lane_u8(c, 6) << 4 * 6;
        ret |= (uint64_t)vgetq_lane_u8(c, 7) << 4 * 7;
        ret |= (uint64_t)vgetq_lane_u8(c, 8) << 4 * 8;
        ret |= (uint64_t)vgetq_lane_u8(c, 9) << 4 * 9;
        ret |= (uint64_t)vgetq_lane_u8(c, 10) << 4 * 10;
        ret |= (uint64_t)vgetq_lane_u8(c, 11) << 4 * 11;
        ret |= (uint64_t)vgetq_lane_u8(c, 12) << 4 * 12;
        ret |= (uint64_t)vgetq_lane_u8(c, 13) << 4 * 13;
        ret |= (uint64_t)vgetq_lane_u8(c, 14) << 4 * 14;
        ret |= (uint64_t)vgetq_lane_u8(c, 15) << 4 * 15;

        return ret;
}

uint64_t gift_64_vec_sbox_decrypt(const uint64_t c, const uint64_t key[2])
{
        // construct sbox_vec_inv
        sbox_vec_inv = vsetq_lane_u8(0xd, sbox_vec_inv, 0);
        sbox_vec_inv = vsetq_lane_u8(0x0, sbox_vec_inv, 1);
        sbox_vec_inv = vsetq_lane_u8(0x8, sbox_vec_inv, 2);
        sbox_vec_inv = vsetq_lane_u8(0x6, sbox_vec_inv, 3);
        sbox_vec_inv = vsetq_lane_u8(0x2, sbox_vec_inv, 4);
        sbox_vec_inv = vsetq_lane_u8(0xc, sbox_vec_inv, 5);
        sbox_vec_inv = vsetq_lane_u8(0x4, sbox_vec_inv, 6);
        sbox_vec_inv = vsetq_lane_u8(0xb, sbox_vec_inv, 7);
        sbox_vec_inv = vsetq_lane_u8(0xe, sbox_vec_inv, 8);
        sbox_vec_inv = vsetq_lane_u8(0x7, sbox_vec_inv, 9);
        sbox_vec_inv = vsetq_lane_u8(0x1, sbox_vec_inv, 10);
        sbox_vec_inv = vsetq_lane_u8(0xa, sbox_vec_inv, 11);
        sbox_vec_inv = vsetq_lane_u8(0x3, sbox_vec_inv, 12);
        sbox_vec_inv = vsetq_lane_u8(0x9, sbox_vec_inv, 13);
        sbox_vec_inv = vsetq_lane_u8(0xf, sbox_vec_inv, 14);
        sbox_vec_inv = vsetq_lane_u8(0x5, sbox_vec_inv, 15);

        // pack into vector register
        uint8x16_t m;
        m = vsetq_lane_u8((c >> 4 * 0) & 0xf, m, 0);
        m = vsetq_lane_u8((c >> 4 * 1) & 0xf, m, 1);
        m = vsetq_lane_u8((c >> 4 * 2) & 0xf, m, 2);
        m = vsetq_lane_u8((c >> 4 * 3) & 0xf, m, 3);
        m = vsetq_lane_u8((c >> 4 * 4) & 0xf, m, 4);
        m = vsetq_lane_u8((c >> 4 * 5) & 0xf, m, 5);
        m = vsetq_lane_u8((c >> 4 * 6) & 0xf, m, 6);
        m = vsetq_lane_u8((c >> 4 * 7) & 0xf, m, 7);
        m = vsetq_lane_u8((c >> 4 * 8) & 0xf, m, 8);
        m = vsetq_lane_u8((c >> 4 * 9) & 0xf, m, 9);
        m = vsetq_lane_u8((c >> 4 * 10) & 0xf, m, 10);
        m = vsetq_lane_u8((c >> 4 * 11) & 0xf, m, 11);
        m = vsetq_lane_u8((c >> 4 * 12) & 0xf, m, 12);
        m = vsetq_lane_u8((c >> 4 * 13) & 0xf, m, 13);
        m = vsetq_lane_u8((c >> 4 * 14) & 0xf, m, 14);
        m = vsetq_lane_u8((c >> 4 * 15) & 0xf, m, 15);

        // generate round keys
        uint8x16_t round_keys[ROUNDS_GIFT_64];
        gift_64_vec_sbox_generate_round_keys(round_keys, key);

        // round loop (in reverse)
        for (int round = ROUNDS_GIFT_64 - 1; round >= 0; round--) {
                m = veorq_u8(m, round_keys[round]);
                m = gift_64_vec_sbox_permute_inv(m);
                m = gift_64_vec_sbox_subcells_inv(m);
        }

        // unpack
        uint64_t ret = 0UL;
        ret |= (uint64_t)vgetq_lane_u8(m, 0) << 4 * 0;
        ret |= (uint64_t)vgetq_lane_u8(m, 1) << 4 * 1;
        ret |= (uint64_t)vgetq_lane_u8(m, 2) << 4 * 2;
        ret |= (uint64_t)vgetq_lane_u8(m, 3) << 4 * 3;
        ret |= (uint64_t)vgetq_lane_u8(m, 4) << 4 * 4;
        ret |= (uint64_t)vgetq_lane_u8(m, 5) << 4 * 5;
        ret |= (uint64_t)vgetq_lane_u8(m, 6) << 4 * 6;
        ret |= (uint64_t)vgetq_lane_u8(m, 7) << 4 * 7;
        ret |= (uint64_t)vgetq_lane_u8(m, 8) << 4 * 8;
        ret |= (uint64_t)vgetq_lane_u8(m, 9) << 4 * 9;
        ret |= (uint64_t)vgetq_lane_u8(m, 10) << 4 * 10;
        ret |= (uint64_t)vgetq_lane_u8(m, 11) << 4 * 11;
        ret |= (uint64_t)vgetq_lane_u8(m, 12) << 4 * 12;
        ret |= (uint64_t)vgetq_lane_u8(m, 13) << 4 * 13;
        ret |= (uint64_t)vgetq_lane_u8(m, 14) << 4 * 14;
        ret |= (uint64_t)vgetq_lane_u8(m, 15) << 4 * 15;

        return ret;
}
