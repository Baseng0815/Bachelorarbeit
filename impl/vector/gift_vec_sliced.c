#include "gift_vec_sliced.h"

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

void swapmove(uint8x16_t *a, uint8x16_t *b, uint8x16_t m, int n)
{

}

void bits_pack(uint8x16x4_t m)
{
        // take care not to shift mask bits out of the register
        swapmove(m.val[0], m.val[1], 0x5555555555555555UL, 1);
        swapmove(m.val[2], m.val[3], 0x5555555555555555UL, 1);
        swapmove(m.val[4], m.val[5], 0x5555555555555555UL, 1);
        swapmove(m.val[6], m.val[7], 0x5555555555555555UL, 1);

        swapmove(m.val[0], m.val[2], 0x3333333333333333UL, 2);
        swapmove(m.val[1], m.val[3], 0x3333333333333333UL, 2);
        swapmove(m.val[4], m.val[6], 0x3333333333333333UL, 2);
        swapmove(m.val[5], m.val[7], 0x3333333333333333UL, 2);

        // make bytes (a0 b0 c0 d0 a4 b4 c4 d4 -> a0 b0 c0 d0 e0 f0 g0 h0)
        swapmove(m.val[0], m.val[4], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(m.val[2], m.val[6], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(m.val[1], m.val[5], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(m.val[3], m.val[7], 0x0f0f0f0f0f0f0f0fUL, 4);
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

void gift_64_vec_sbox_encrypt(uint64_t c[8], const uint64_t m[8], const uint64_t key[2])
{

}

void gift_64_vec_sbox_decrypt(uint64_t m[8], const uint64_t c[8], const uint64_t key[2])
{

}
