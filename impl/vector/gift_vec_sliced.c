#include "gift_vec_sliced.h"

#include <stddef.h>

static uint8x16_t perm_s0;
static uint8x16_t perm_s1;
static uint8x16_t perm_s2;
static uint8x16_t perm_s3;

static uint8x16_t perm_inv_s0;
static uint8x16_t perm_inv_s1;
static uint8x16_t perm_inv_s2;
static uint8x16_t perm_inv_s3;

static uint8x16_t pack_mask_0;
static uint8x16_t pack_mask_1;
static uint8x16_t pack_mask_2;

static const int round_constant[] = {
        // rounds 0-15
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F, 0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E,
        // rounds 16-31
        0x1D, 0x3A, 0x35, 0x2B, 0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E, 0x1C, 0x38,
        // rounds 32-47
        0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
};

uint8x16_t shl(uint8x16_t v, int n)
{
        // TODO check out vst
        uint64_t l0 = vgetq_lane_u64(v, 0);
        uint64_t l1 = vgetq_lane_u64(v, 1);
        l1 = l1 << n | (l0 >> (64 - n));
        l0 <<= n;
        uint8x16_t ret;
        ret = vsetq_lane_u64(l0, ret, 0);
        ret = vsetq_lane_u64(l1, ret, 1);
        return ret;
}

uint8x16_t shr(uint8x16_t v, int n)
{
        uint64_t l0 = vgetq_lane_u64(v, 0);
        uint64_t l1 = vgetq_lane_u64(v, 1);
        l0 = l0 >> n | (((l1 << (64 - n)) >> (64 - n)) << (64 - n));
        l1 >>= n;
        uint8x16_t ret;
        ret = vsetq_lane_u64(l0, ret, 0);
        ret = vsetq_lane_u64(l1, ret, 1);
        return ret;
}

void gift_64_vec_sliced_swapmove(uint8x16_t *a, uint8x16_t *b, uint8x16_t m, int n)
{

        uint8x16_t t = vandq_u8(veorq_u8(shr(*a, n), *b), m);
        // TODO use 16 words (8 registers) so a != b
        *b = veorq_u8(*b, t);
        *a = veorq_u8(*a, shl(t, n));
}

void gift_64_vec_sliced_bits_pack(uint8x16x4_t *m)
{
        // take care not to shift mask bits out of the register
        gift_64_vec_sliced_swapmove(&m->val[0], &m->val[1], pack_mask_0, 1);
        gift_64_vec_sliced_swapmove(&m->val[2], &m->val[3], pack_mask_0, 1);

        gift_64_vec_sliced_swapmove(&m->val[0], &m->val[2], pack_mask_1, 2);
        gift_64_vec_sliced_swapmove(&m->val[1], &m->val[3], pack_mask_1, 2);

        // make bytes (a0 b0 c0 d0 a4 b4 c4 d4 -> a0 b0 c0 d0 e0 f0 g0 h0)
        gift_64_vec_sliced_swapmove(&m->val[0], &m->val[0], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m->val[1], &m->val[1], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m->val[2], &m->val[2], pack_mask_2, 4);
        gift_64_vec_sliced_swapmove(&m->val[3], &m->val[3], pack_mask_2, 4);
}

void gift_64_vec_sliced_subcells(uint8x16x4_t cs)
{
        cs.val[1]       = veorq_u8(cs.val[1], vandq_u8(cs.val[0], cs.val[2]));
        uint8x16_t t    = veorq_u8(cs.val[0], vandq_u8(cs.val[1], cs.val[3]));
        cs.val[2]       = veorq_u8(cs.val[2], vorrq_u8(t, cs.val[1]));
        cs.val[0]       = veorq_u8(cs.val[3], cs.val[2]);
        cs.val[1]       = veorq_u8(cs.val[1], cs.val[0]);
        cs.val[0]       = vmvnq_u8(cs.val[0]);
        cs.val[2]       = veorq_u8(cs.val[2], vandq_u8(t, cs.val[1]));
        cs.val[3]       = t;
}

void gift_64_vec_sliced_subcells_inv(uint8x16x4_t cs)
{
        uint8x16_t t    = cs.val[3];
        cs.val[2]       = veorq_u8(cs.val[2], vandq_u8(t, cs.val[1]));
        cs.val[0]       = vmvnq_u8(cs.val[0]);
        cs.val[1]       = veorq_u8(cs.val[1], cs.val[0]);
        cs.val[3]       = veorq_u8(cs.val[0], cs.val[2]);
        cs.val[2]       = veorq_u8(cs.val[2], vorrq_u8(t, cs.val[1]));
        cs.val[0]       = veorq_u8(t, vandq_u8(cs.val[1], cs.val[3]));
        cs.val[1]       = veorq_u8(cs.val[1], vandq_u8(cs.val[0], cs.val[2]));
}

void gift_64_vec_sliced_permute(uint8x16x4_t cs)
{
        cs.val[0] = vqtbl1q_u8(cs.val[0], perm_s0);
        cs.val[1] = vqtbl1q_u8(cs.val[1], perm_s1);
        cs.val[2] = vqtbl1q_u8(cs.val[2], perm_s2);
        cs.val[3] = vqtbl1q_u8(cs.val[3], perm_s3);
}

void gift_64_vec_sliced_permute_inv(uint8x16x4_t cs)
{
        cs.val[0] = vqtbl1q_u8(cs.val[0], perm_inv_s0);
        cs.val[1] = vqtbl1q_u8(cs.val[1], perm_inv_s1);
        cs.val[2] = vqtbl1q_u8(cs.val[2], perm_inv_s2);
        cs.val[3] = vqtbl1q_u8(cs.val[3], perm_inv_s3);
}

void gift_64_vec_sliced_generate_round_keys(uint8x16x4_t round_keys[ROUNDS_GIFT_64],
                                            const uint64_t key[2])
{
        // TODO shift-and-or

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

                round_keys[round].val[0] = vsetq_lane_u64(round_key, round_keys[round].val[0], 0);
                round_keys[round].val[0] = vsetq_lane_u64(round_key, round_keys[round].val[0], 1);
                round_keys[round].val[1] = round_keys[round].val[0];
                round_keys[round].val[2] = round_keys[round].val[0];
                round_keys[round].val[3] = round_keys[round].val[0];

                gift_64_vec_sliced_bits_pack(&round_keys[round]);

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

        // 0, 4, 8, c, 3, 7, b, f, 2, 6, a, e, 1, 5, 9, d
        // 1, 5, 9, d, 0, 4, 8, c, 3, 7, b, f, 2, 6, a, e
        // 2, 6, a, e, 1, 5, 9, d, 0, 4, 8, c, 3, 7, b, f
        // 3, 7, b, f, 2, 6, a, e, 1, 5, 9, d, 0, 4, 8, c

        // permutations
        perm_s0 = vsetq_lane_u64(0x0f0b07030c080400UL, perm_s0, 0);
        perm_s0 = vsetq_lane_u64(0x0d0905010e0a0602UL, perm_s0, 1);
        perm_s1 = vsetq_lane_u64(0x0c0804000d090501UL, perm_s1, 0);
        perm_s1 = vsetq_lane_u64(0x0e0a06020f0b0703UL, perm_s1, 1);
        perm_s2 = vsetq_lane_u64(0x0d0905010e0a0602UL, perm_s2, 0);
        perm_s2 = vsetq_lane_u64(0x0f0b07030c080400UL, perm_s2, 1);
        perm_s3 = vsetq_lane_u64(0x0e0a06020f0b0703UL, perm_s3, 0);
        perm_s3 = vsetq_lane_u64(0x0c0804000d090501UL, perm_s3, 1);

        // inverse permutations
        perm_inv_s0 = vsetq_lane_u64(0x05090d0104080c00UL, perm_inv_s0, 0);
        perm_inv_s0 = vsetq_lane_u64(0x070b0f03060a0e02UL, perm_inv_s0, 1);
        perm_inv_s1 = vsetq_lane_u64(0x090d0105080c0004UL, perm_inv_s1, 0);
        perm_inv_s1 = vsetq_lane_u64(0x0b0f03070a0e0206UL, perm_inv_s1, 1);
        perm_inv_s2 = vsetq_lane_u64(0x0d0105090c000408UL, perm_inv_s2, 0);
        perm_inv_s2 = vsetq_lane_u64(0x0f03070b0e02060aUL, perm_inv_s2, 1);
        perm_inv_s3 = vsetq_lane_u64(0x0105090d0004080cUL, perm_inv_s3, 0);
        perm_inv_s3 = vsetq_lane_u64(0x03070b0f02060a0eUL, perm_inv_s3, 1);

        // packing masks
        pack_mask_0 = vsetq_lane_u64(0x5555555555555555UL, pack_mask_0, 0);
        pack_mask_0 = vsetq_lane_u64(0x5555555555555555UL, pack_mask_0, 1);
        pack_mask_1 = vsetq_lane_u64(0x3333333333333333UL, pack_mask_1, 0);
        pack_mask_1 = vsetq_lane_u64(0x3333333333333333UL, pack_mask_1, 1);
        pack_mask_2 = vsetq_lane_u64(0x0f0f0f0f0f0f0f0fUL, pack_mask_2, 0);
        pack_mask_2 = vsetq_lane_u64(0x0f0f0f0f0f0f0f0fUL, pack_mask_2, 1);
}

void gift_64_vec_sliced_encrypt(uint64_t c[8], const uint64_t m[8], const uint64_t key[2])
{
        uint8x16x4_t s;
        s.val[0] = vsetq_lane_u64(m[0], s.val[0], 0);
        s.val[0] = vsetq_lane_u64(m[1], s.val[0], 1);
        s.val[1] = vsetq_lane_u64(m[2], s.val[1], 0);
        s.val[1] = vsetq_lane_u64(m[3], s.val[1], 1);
        s.val[2] = vsetq_lane_u64(m[4], s.val[2], 0);
        s.val[2] = vsetq_lane_u64(m[5], s.val[2], 1);
        s.val[3] = vsetq_lane_u64(m[6], s.val[3], 0);
        s.val[3] = vsetq_lane_u64(m[7], s.val[3], 1);

        gift_64_vec_sliced_bits_pack(&s);
        printf("%lx %lx\n", vgetq_lane_u64(s.val[0], 1), vgetq_lane_u64(s.val[0], 0));
        printf("%lx %lx\n", vgetq_lane_u64(s.val[1], 1), vgetq_lane_u64(s.val[1], 0));
        printf("%lx %lx\n", vgetq_lane_u64(s.val[2], 1), vgetq_lane_u64(s.val[2], 0));
        printf("%lx %lx\n", vgetq_lane_u64(s.val[3], 1), vgetq_lane_u64(s.val[3], 0));

        uint8x16x4_t round_keys[ROUNDS_GIFT_64];
        gift_64_vec_sliced_generate_round_keys(round_keys, key);

        for (int round = 0; round < ROUNDS_GIFT_64; round++) {
                gift_64_vec_sliced_subcells(s);
                gift_64_vec_sliced_permute(s);

                s.val[0] = veorq_u8(s.val[0], round_keys[round].val[0]);
                s.val[1] = veorq_u8(s.val[1], round_keys[round].val[1]);
                s.val[2] = veorq_u8(s.val[2], round_keys[round].val[2]);
                s.val[3] = veorq_u8(s.val[3], round_keys[round].val[3]);
        }

        c[0] = vgetq_lane_u64(s.val[0], 0);
        c[1] = vgetq_lane_u64(s.val[0], 1);
        c[2] = vgetq_lane_u64(s.val[1], 0);
        c[3] = vgetq_lane_u64(s.val[1], 1);
        c[4] = vgetq_lane_u64(s.val[2], 0);
        c[5] = vgetq_lane_u64(s.val[2], 1);
        c[6] = vgetq_lane_u64(s.val[3], 0);
        c[7] = vgetq_lane_u64(s.val[3], 1);
}

void gift_64_vec_sliced_decrypt(uint64_t m[8], const uint64_t c[8], const uint64_t key[2])
{

}
