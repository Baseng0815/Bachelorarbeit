#include "gift_vec_sliced.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static uint64_t pack_shf_u64[4] = {
        0x1303120211011000UL, 0x1707160615051404UL, // S0/S1/S2/S3
        0x1b0b1a0a19091808UL, 0x1f0f1e0e1d0d1c0cUL, // S4/S5/S6/S7
};

static uint64_t pack_shf_inv_u64[4] = {
        0x0e0c0a0806040200UL, 0x1e1c1a1816141210UL, // S0/S1/S2/S3
        0x0f0d0b0907050301UL, 0x1f1d1b1917151311UL, // S4/S5/S6/S7
};

static uint64_t perm_u64[8] = {
        0x0f0b07030c080400UL, 0x0d0905010e0a0602UL, // S0/S4
        0x0c0804000d090501UL, 0x0e0a06020f0b0703UL, // S1/S5
        0x0d0905010e0a0602UL, 0x0f0b07030c080400UL, // S2/S6
        0x0e0a06020f0b0703UL, 0x0c0804000d090501UL  // S3/S7
};


static uint64_t perm_inv_u64[8] = {
        0x05090d0104080c00UL, 0x070b0f03060a0e02UL, // S0/S4
        0x090d0105080c0004UL, 0x0b0f03070a0e0206UL, // S1/S5
        0x0d0105090c000408UL, 0x0f03070b0e02060aUL, // S2/S6
        0x0105090d0004080cUL, 0x03070b0f02060a0eUL  // S3/S7
};

static uint8x16x2_t pack_shf;
static uint8x16x2_t pack_shf_inv;
static uint8x16x4_t perm;
static uint8x16x4_t perm_inv;

static uint8x16_t pack_mask_0;
static uint8x16_t pack_mask_1;
static uint8x16_t pack_mask_2;

static const int round_const[] = {
        // rounds 0-15
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E,
        // rounds 16-31
        0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E, 0x1C, 0x38,
        // rounds 32-47
        0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
};

uint8x16_t shl(const uint8x16_t v, const int n)
{
        uint64_t l[2];
        vst1q_u64(l, v);
        l[1] = (l[1] << n) | (l[0] >> (64 - n));
        l[0] <<= n;
        return vld1q_u64(l);
}

uint8x16_t shr(const uint8x16_t v, const int n)
{
        uint64_t l[2];
        vst1q_u64(l, v);
        l[0] = l[0] >> n | (((l[1] << (64 - n)) >> (64 - n)) << (64 - n));
        l[1] >>= n;
        return vld1q_u64(l);
}

void gift_64_vec_sliced_swapmove(uint8x16_t *restrict a, uint8x16_t *restrict b,
                                 const uint8x16_t m, const int n)
{

        const uint8x16_t t = vandq_u8(veorq_u8(shr(*a, n), *b), m);
        *b = veorq_u8(*b, t);
        *a = veorq_u8(*a, shl(t, n));
}

void gift_64_vec_sliced_bits_pack(uint8x16x4_t m[restrict 2])
{
        // take care not to shift mask bits out of the register
        gift_64_vec_sliced_swapmove(&m[0].val[0], &m[0].val[1], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m[0].val[2], &m[0].val[3], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m[1].val[0], &m[1].val[1], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m[1].val[2], &m[1].val[3], pack_mask_0, 1);

        gift_64_vec_sliced_swapmove(&m[0].val[0], &m[0].val[2], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m[0].val[1], &m[0].val[3], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m[1].val[0], &m[1].val[2], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m[1].val[1], &m[1].val[3], pack_mask_1, 2);

        // make bytes (a0 b0 c0 d0 a4 b4 c4 d4 -> a0 b0 c0 d0 e0 f0 g0 h0)
        gift_64_vec_sliced_swapmove(&m[0].val[0], &m[1].val[0], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m[0].val[2], &m[1].val[2], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m[0].val[1], &m[1].val[1], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m[0].val[3], &m[1].val[3], pack_mask_2, 4);

        // same plaintext slice bits into same register (so we only have to do
        // what we are doing here once instead of every round)
        const uint8x16x2_t pairs[4] = {
                { .val = { m[0].val[0], m[1].val[0] }},
                { .val = { m[0].val[1], m[1].val[1] }},
                { .val = { m[0].val[2], m[1].val[2] }},
                { .val = { m[0].val[3], m[1].val[3] }},
        };

        m[0].val[0] = vqtbl2q_u8(pairs[0], pack_shf.val[0]);
        m[0].val[1] = vqtbl2q_u8(pairs[1], pack_shf.val[0]);
        m[0].val[2] = vqtbl2q_u8(pairs[2], pack_shf.val[0]);
        m[0].val[3] = vqtbl2q_u8(pairs[3], pack_shf.val[0]);

        m[1].val[0] = vqtbl2q_u8(pairs[0], pack_shf.val[1]);
        m[1].val[1] = vqtbl2q_u8(pairs[1], pack_shf.val[1]);
        m[1].val[2] = vqtbl2q_u8(pairs[2], pack_shf.val[1]);
        m[1].val[3] = vqtbl2q_u8(pairs[3], pack_shf.val[1]);
}

void gift_64_vec_sliced_bits_unpack(uint8x16x4_t m[restrict 2])
{
        const uint8x16x2_t pairs[4] = {
                { .val = { m[0].val[0], m[1].val[0] }},
                { .val = { m[0].val[1], m[1].val[1] }},
                { .val = { m[0].val[2], m[1].val[2] }},
                { .val = { m[0].val[3], m[1].val[3] }},
        };

        m[0].val[0] = vqtbl2q_u8(pairs[0], pack_shf_inv.val[0]);
        m[0].val[1] = vqtbl2q_u8(pairs[1], pack_shf_inv.val[0]);
        m[0].val[2] = vqtbl2q_u8(pairs[2], pack_shf_inv.val[0]);
        m[0].val[3] = vqtbl2q_u8(pairs[3], pack_shf_inv.val[0]);

        m[1].val[0] = vqtbl2q_u8(pairs[0], pack_shf_inv.val[1]);
        m[1].val[1] = vqtbl2q_u8(pairs[1], pack_shf_inv.val[1]);
        m[1].val[2] = vqtbl2q_u8(pairs[2], pack_shf_inv.val[1]);
        m[1].val[3] = vqtbl2q_u8(pairs[3], pack_shf_inv.val[1]);

        // take care not to shift mask bits out of the register
        gift_64_vec_sliced_swapmove(&m[0].val[0], &m[0].val[1], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m[0].val[2], &m[0].val[3], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m[1].val[0], &m[1].val[1], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m[1].val[2], &m[1].val[3], pack_mask_0, 1);

        gift_64_vec_sliced_swapmove(&m[0].val[0], &m[0].val[2], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m[0].val[1], &m[0].val[3], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m[1].val[0], &m[1].val[2], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m[1].val[1], &m[1].val[3], pack_mask_1, 2);

        // make bytes (a0 b0 c0 d0 a4 b4 c4 d4 -> a0 b0 c0 d0 e0 f0 g0 h0)
        gift_64_vec_sliced_swapmove(&m[0].val[0], &m[1].val[0], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m[0].val[2], &m[1].val[2], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m[0].val[1], &m[1].val[1], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m[0].val[3], &m[1].val[3], pack_mask_2, 4);
}

void gift_64_vec_sliced_subcells(uint8x16x4_t cs[restrict 2])
{
        cs[0].val[1] = veorq_u8(cs[0].val[1],
                                vandq_u8(cs[0].val[0], cs[0].val[2]));
        uint8x16_t t = veorq_u8(cs[0].val[0],
                                vandq_u8(cs[0].val[1], cs[0].val[3]));
        cs[0].val[2] = veorq_u8(cs[0].val[2], vorrq_u8(t, cs[0].val[1]));
        cs[0].val[0] = veorq_u8(cs[0].val[3], cs[0].val[2]);
        cs[0].val[1] = veorq_u8(cs[0].val[1], cs[0].val[0]);
        cs[0].val[0] = vmvnq_u8(cs[0].val[0]);
        cs[0].val[2] = veorq_u8(cs[0].val[2], vandq_u8(t, cs[0].val[1]));
        cs[0].val[3] = t;

        cs[1].val[1] = veorq_u8(cs[1].val[1],
                                vandq_u8(cs[1].val[0], cs[1].val[2]));
        t            = veorq_u8(cs[1].val[0],
                                vandq_u8(cs[1].val[1], cs[1].val[3]));
        cs[1].val[2] = veorq_u8(cs[1].val[2], vorrq_u8(t, cs[1].val[1]));
        cs[1].val[0] = veorq_u8(cs[1].val[3], cs[1].val[2]);
        cs[1].val[1] = veorq_u8(cs[1].val[1], cs[1].val[0]);
        cs[1].val[0] = vmvnq_u8(cs[1].val[0]);
        cs[1].val[2] = veorq_u8(cs[1].val[2], vandq_u8(t, cs[1].val[1]));
        cs[1].val[3] = t;
}

void gift_64_vec_sliced_subcells_inv(uint8x16x4_t cs[restrict 2])
{
        uint8x16_t t = cs[0].val[3];
        cs[0].val[2] = veorq_u8(cs[0].val[2], vandq_u8(t, cs[0].val[1]));
        cs[0].val[0] = vmvnq_u8(cs[0].val[0]);
        cs[0].val[1] = veorq_u8(cs[0].val[1], cs[0].val[0]);
        cs[0].val[3] = veorq_u8(cs[0].val[0], cs[0].val[2]);
        cs[0].val[2] = veorq_u8(cs[0].val[2], vorrq_u8(t, cs[0].val[1]));
        cs[0].val[0] = veorq_u8(t, vandq_u8(cs[0].val[1], cs[0].val[3]));
        cs[0].val[1] = veorq_u8(cs[0].val[1],
                                vandq_u8(cs[0].val[0], cs[0].val[2]));

        t            = cs[1].val[3];
        cs[1].val[2] = veorq_u8(cs[1].val[2], vandq_u8(t, cs[1].val[1]));
        cs[1].val[0] = vmvnq_u8(cs[1].val[0]);
        cs[1].val[1] = veorq_u8(cs[1].val[1], cs[1].val[0]);
        cs[1].val[3] = veorq_u8(cs[1].val[0], cs[1].val[2]);
        cs[1].val[2] = veorq_u8(cs[1].val[2], vorrq_u8(t, cs[1].val[1]));
        cs[1].val[0] = veorq_u8(t, vandq_u8(cs[1].val[1], cs[1].val[3]));
        cs[1].val[1] = veorq_u8(cs[1].val[1],
                                vandq_u8(cs[1].val[0], cs[1].val[2]));
}

void gift_64_vec_sliced_permute(uint8x16x4_t cs[restrict 2])
{
        cs[0].val[0] = vqtbl1q_u8(cs[0].val[0], perm.val[0]);
        cs[0].val[1] = vqtbl1q_u8(cs[0].val[1], perm.val[1]);
        cs[0].val[2] = vqtbl1q_u8(cs[0].val[2], perm.val[2]);
        cs[0].val[3] = vqtbl1q_u8(cs[0].val[3], perm.val[3]);

        cs[1].val[0] = vqtbl1q_u8(cs[1].val[0], perm.val[0]);
        cs[1].val[1] = vqtbl1q_u8(cs[1].val[1], perm.val[1]);
        cs[1].val[2] = vqtbl1q_u8(cs[1].val[2], perm.val[2]);
        cs[1].val[3] = vqtbl1q_u8(cs[1].val[3], perm.val[3]);
}

void gift_64_vec_sliced_permute_inv(uint8x16x4_t cs[restrict 2])
{
        cs[0].val[0] = vqtbl1q_u8(cs[0].val[0], perm_inv.val[0]);
        cs[0].val[1] = vqtbl1q_u8(cs[0].val[1], perm_inv.val[1]);
        cs[0].val[2] = vqtbl1q_u8(cs[0].val[2], perm_inv.val[2]);
        cs[0].val[3] = vqtbl1q_u8(cs[0].val[3], perm_inv.val[3]);

        cs[1].val[0] = vqtbl1q_u8(cs[1].val[0], perm_inv.val[0]);
        cs[1].val[1] = vqtbl1q_u8(cs[1].val[1], perm_inv.val[1]);
        cs[1].val[2] = vqtbl1q_u8(cs[1].val[2], perm_inv.val[2]);
        cs[1].val[3] = vqtbl1q_u8(cs[1].val[3], perm_inv.val[3]);
}

void gift_64_vec_sliced_generate_round_keys(uint8x16x4_t rks[restrict ROUNDS_GIFT_64][2],
                                            const uint64_t key[restrict 2])
{
        uint64_t key_state[] = {key[0], key[1]};
        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                const int v = (key_state[0] >> 0 ) & 0xffff;
                const int u = (key_state[0] >> 16) & 0xffff;

                // add round key (RK=U||V)
                // (slice 2 stays unused)
                uint64_t rk[6] = { 0x0UL };
                for (size_t i = 0; i < 8; i++) {
                        int key_bit_v   = (v >> (i + 0)) & 0x1;
                        int key_bit_u   = (u >> (i + 0)) & 0x1;
                        rk[0]           ^= (uint64_t)key_bit_v << (i * 8);
                        rk[2]           ^= (uint64_t)key_bit_u << (i * 8);

                        key_bit_v       = (v >> (i + 8)) & 0x1;
                        key_bit_u       = (u >> (i + 8)) & 0x1;
                        rk[1]           ^= (uint64_t)key_bit_v << (i * 8);
                        rk[3]           ^= (uint64_t)key_bit_u << (i * 8);
                }

                // add single bit
                rk[5] ^= 1UL << (7 * 8);

                // add round constants
                rk[4] ^= ((uint64_t)(round_const[round] >> 0) & 0x1) << (0 * 8);
                rk[4] ^= ((uint64_t)(round_const[round] >> 1) & 0x1) << (1 * 8);
                rk[4] ^= ((uint64_t)(round_const[round] >> 2) & 0x1) << (2 * 8);
                rk[4] ^= ((uint64_t)(round_const[round] >> 3) & 0x1) << (3 * 8);
                rk[4] ^= ((uint64_t)(round_const[round] >> 4) & 0x1) << (4 * 8);
                rk[4] ^= ((uint64_t)(round_const[round] >> 5) & 0x1) << (5 * 8);

                // extend bits to bytes
                for (size_t i = 0; i < 6; i++) {
                        rk[i] |= rk[i] << 1;
                        rk[i] |= rk[i] << 2;
                        rk[i] |= rk[i] << 4;
                }

                rks[round][0].val[0] = vsetq_lane_u64(rk[0], rks[round][0].val[0], 0);
                rks[round][0].val[0] = vsetq_lane_u64(rk[1], rks[round][0].val[0], 1);
                rks[round][0].val[1] = vsetq_lane_u64(rk[2], rks[round][0].val[1], 0);
                rks[round][0].val[1] = vsetq_lane_u64(rk[3], rks[round][0].val[1], 1);
                rks[round][0].val[2] = vdupq_n_u8(0);
                rks[round][0].val[3] = vsetq_lane_u64(rk[4], rks[round][0].val[3], 0);
                rks[round][0].val[3] = vsetq_lane_u64(rk[5], rks[round][0].val[3], 1);
                rks[round][1]        = rks[round][0];

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

void gift_64_vec_sliced_init(void)
{
        // bit packing shuffle
        pack_shf = vld1q_u8_x2((uint8_t*)&pack_shf_u64[0]);

        // inverse bit packing shuffle
        pack_shf_inv = vld1q_u8_x2((uint8_t*)&pack_shf_inv_u64[0]);

        // permutations
        perm = vld1q_u8_x4((uint8_t*)&perm_u64[0]);

        // inverse permutations
        perm_inv = vld1q_u8_x4((uint8_t*)&perm_inv_u64[0]);

        // packing masks
        pack_mask_0 = vdupq_n_u8(0x55);
        pack_mask_1 = vdupq_n_u8(0x33);
        pack_mask_2 = vdupq_n_u8(0x0f);
}

void gift_64_vec_sliced_encrypt(uint64_t c[restrict 16],
                                const uint64_t m[restrict 16],
                                const uint8x16x4_t rks[restrict ROUNDS_GIFT_64][2])
{
        uint8x16x4_t s[2];
        s[0] = vld1q_u8_x4((uint8_t*)&m[0]);
        s[1] = vld1q_u8_x4((uint8_t*)&m[8]);
        gift_64_vec_sliced_bits_pack(s);

        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                gift_64_vec_sliced_subcells(s);
                gift_64_vec_sliced_permute(s);

                s[0].val[0] = veorq_u8(s[0].val[0], rks[round][0].val[0]);
                s[0].val[1] = veorq_u8(s[0].val[1], rks[round][0].val[1]);
                s[0].val[2] = veorq_u8(s[0].val[2], rks[round][0].val[2]);
                s[0].val[3] = veorq_u8(s[0].val[3], rks[round][0].val[3]);
                s[1].val[0] = veorq_u8(s[1].val[0], rks[round][1].val[0]);
                s[1].val[1] = veorq_u8(s[1].val[1], rks[round][1].val[1]);
                s[1].val[2] = veorq_u8(s[1].val[2], rks[round][1].val[2]);
                s[1].val[3] = veorq_u8(s[1].val[3], rks[round][1].val[3]);
        }

        gift_64_vec_sliced_bits_unpack(s);
        vst1q_u8_x4((uint8_t*)&c[0], s[0]);
        vst1q_u8_x4((uint8_t*)&c[8], s[1]);
}

void gift_64_vec_sliced_decrypt(uint64_t m[restrict 16],
                                const uint64_t c[restrict 16],
                                const uint8x16x4_t rks[restrict ROUNDS_GIFT_64][2])
{
        uint8x16x4_t s[2];
        s[0] = vld1q_u8_x4((uint8_t*)&c[0]);
        s[1] = vld1q_u8_x4((uint8_t*)&c[8]);
        gift_64_vec_sliced_bits_pack(s);

        for (int round = ROUNDS_GIFT_64 - 1; round >= 0; round--) {
                s[0].val[0] = veorq_u8(s[0].val[0], rks[round][0].val[0]);
                s[0].val[1] = veorq_u8(s[0].val[1], rks[round][0].val[1]);
                s[0].val[2] = veorq_u8(s[0].val[2], rks[round][0].val[2]);
                s[0].val[3] = veorq_u8(s[0].val[3], rks[round][0].val[3]);
                s[1].val[0] = veorq_u8(s[1].val[0], rks[round][1].val[0]);
                s[1].val[1] = veorq_u8(s[1].val[1], rks[round][1].val[1]);
                s[1].val[2] = veorq_u8(s[1].val[2], rks[round][1].val[2]);
                s[1].val[3] = veorq_u8(s[1].val[3], rks[round][1].val[3]);

                gift_64_vec_sliced_permute_inv(s);
                gift_64_vec_sliced_subcells_inv(s);
        }

        gift_64_vec_sliced_bits_unpack(s);
        vst1q_u8_x4((uint8_t*)&m[0], s[0]);
        vst1q_u8_x4((uint8_t*)&m[8], s[1]);
}
