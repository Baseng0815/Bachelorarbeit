#include "gift_vec_sbox.h"

#include <stdint.h>
#include <arm_neon.h>
#include <string.h>

static uint64_t sbox_vec_u64[2] = {
        0x09030f060c040a01UL, 0x0e080005070b0d02UL
};

static uint64_t sbox_vec_inv_u64[2] = {
0x0b040c020608000dUL, 0x050f09030a01070eUL
};

static uint8x16_t sbox_vec;
static uint8x16_t sbox_vec_inv;

#define U64_TO_V128(V,M)\
        V = vsetq_lane_u64(\
        (uint64_t)((M >> 4 * 0) & 0xf) << 8 * 0 |\
        (uint64_t)((M >> 4 * 1) & 0xf) << 8 * 1 |\
        (uint64_t)((M >> 4 * 2) & 0xf) << 8 * 2 |\
        (uint64_t)((M >> 4 * 3) & 0xf) << 8 * 3 |\
        (uint64_t)((M >> 4 * 4) & 0xf) << 8 * 4 |\
        (uint64_t)((M >> 4 * 5) & 0xf) << 8 * 5 |\
        (uint64_t)((M >> 4 * 6) & 0xf) << 8 * 6 |\
        (uint64_t)((M >> 4 * 7) & 0xf) << 8 * 7,\
        V, 0);\
        V = vsetq_lane_u64(\
        (uint64_t)((M >> 4 * 8)  & 0xf) << 8 * 0 |\
        (uint64_t)((M >> 4 * 9)  & 0xf) << 8 * 1 |\
        (uint64_t)((M >> 4 * 10) & 0xf) << 8 * 2 |\
        (uint64_t)((M >> 4 * 11) & 0xf) << 8 * 3 |\
        (uint64_t)((M >> 4 * 12) & 0xf) << 8 * 4 |\
        (uint64_t)((M >> 4 * 13) & 0xf) << 8 * 5 |\
        (uint64_t)((M >> 4 * 14) & 0xf) << 8 * 6 |\
        (uint64_t)((M >> 4 * 15) & 0xf) << 8 * 7,\
        V, 1);

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

static const int round_const[] = {
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
        uint64_t boxes[2];
        vst1q_u64(boxes, cipher_state);
        for (size_t box = 0; box < 8; box++) {
                for (size_t i = 0; i < 4; i++) {
                        int bit = (boxes[0] >> (box * 8 + i)) & 0x1;
                        new_cipher_state |= (uint64_t)bit << perm_64[box * 4 + i];
                }
        }

        // S-box 8-15
        for (size_t box = 0; box < 8; box++) {
                for (size_t i = 0; i < 4; i++) {
                        int bit = (boxes[1] >> (box * 8 + i)) & 0x1;
                        new_cipher_state |= (uint64_t)bit << perm_64[(box + 8) * 4 + i];
                }
        }

        uint8x16_t ret;
        U64_TO_V128(ret, new_cipher_state);

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

        uint8x16_t ret;
        U64_TO_V128(ret, new_cipher_state);

        return ret;
}

void gift_64_vec_sbox_generate_round_keys(uint8x16_t rks[ROUNDS_GIFT_64],
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
                round_key ^= ((round_const[round] >> 0) & 0x1) << 3;
                round_key ^= ((round_const[round] >> 1) & 0x1) << 7;
                round_key ^= ((round_const[round] >> 2) & 0x1) << 11;
                round_key ^= ((round_const[round] >> 3) & 0x1) << 15;
                round_key ^= ((round_const[round] >> 4) & 0x1) << 19;
                round_key ^= ((round_const[round] >> 5) & 0x1) << 23;

                // pack into vector register
                U64_TO_V128(rks[round], round_key)

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

void gift_64_vec_sbox_init(void)
{
        // construct sbox_vec
        sbox_vec = vld1q_u64(sbox_vec_u64);

        // construct sbox_vec_inv
        sbox_vec_inv = vld1q_u64(sbox_vec_inv_u64);
}

uint64_t gift_64_vec_sbox_encrypt(const uint64_t m, const uint64_t key[2])
{
        // pack into vector register
        uint8x16_t c;
        U64_TO_V128(c, m);

        // generate round keys
        uint8x16_t rks[ROUNDS_GIFT_64];
        gift_64_vec_sbox_generate_round_keys(rks, key);

        // round loop
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                c = gift_64_vec_sbox_subcells(c);
                c = gift_64_vec_sbox_permute(c);
                c = veorq_u8(c, rks[round]);
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
        // pack into vector register
        uint8x16_t m;
        U64_TO_V128(m, c);

        // generate round keys
        uint8x16_t rks[ROUNDS_GIFT_64];
        gift_64_vec_sbox_generate_round_keys(rks, key);

        // round loop (in reverse)
        for (int round = ROUNDS_GIFT_64 - 1; round >= 0; round--) {
                m = veorq_u8(m, rks[round]);
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
