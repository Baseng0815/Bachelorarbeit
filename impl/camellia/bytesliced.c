#include "bytesliced.h"

#include "spec_opt.h" // need the spec_opt key schedule

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8x16x4_t pack_group;
static uint8x16x4_t pack_group_inv;
static uint8x16x4_t pack_single;

static uint8x16_t lower_4_bits_mask;
static uint8x16_t shiftrows_inv;
static uint8x16_t rol32_1_overflow;

// store two for low ([0]) and high ([1]) bits of matrix multiplication
static uint8x16x2_t prefilter_0;
static uint8x16x2_t prefilter_3;  // s3(x) = s0(x <<< 1)
static uint8x16x2_t postfilter_0;
static uint8x16x2_t postfilter_1; // s1(x) = s0(x) <<< 1
static uint8x16x2_t postfilter_2; // s2(x) = s0(x) >>> 1

void rol32_1(uint8x16x4_t *a)
{
        uint8x16_t a3 = a->val[3];

        for (size_t i = 3; i > 0; i--) {
                uint8x16_t *curr = &a->val[i];
                uint8x16_t *prev = &a->val[i - 1];

                uint8x16_t overflow = vshrq_n_u8(*prev, 7);
                *curr = vorrq_u8(vshlq_n_u8(*curr, 1), overflow);
        }

        uint8x16_t overflow = vshrq_n_u8(a3, 7);
        a->val[0] = vorrq_u8(vshlq_n_u8(a->val[0], 1), overflow);
}

static uint8x16_t s(const uint8x16_t X,
                    const uint8x16x2_t prefilter,
                    const uint8x16x2_t postfilter)
{
        // prefilter
        uint8x16_t pre_low  = vqtbl1q_u8(prefilter.val[0],
                                         vandq_u8(X, lower_4_bits_mask));
        uint8x16_t pre_high = vqtbl1q_u8(prefilter.val[1],
                                         vshrq_n_u8(X, 4));
        uint8x16_t pre = veorq_u8(pre_low, pre_high);

        // inverse ShiftRows
        pre = vqtbl1q_u8(pre, shiftrows_inv);

        // AES single round encryption (x <- AESSubBytes(AESShiftRows(x)))
        uint8x16_t aesd = vaeseq_u8(pre, vdupq_n_u8(0x0));

        // postfilter
        uint8x16_t post_low  = vqtbl1q_u8(postfilter.val[0],
                                          vandq_u8(aesd, lower_4_bits_mask));
        uint8x16_t post_high = vqtbl1q_u8(postfilter.val[1],
                                          vshrq_n_u8(aesd, 4));
        uint8x16_t post = veorq_u8(post_low, post_high);

        return post;
}

void camellia_sliced_F(uint8x16x4_t X[restrict 2],
                       const uint8x16x4_t k[restrict 2])
{
        // key additions
        for (size_t byte = 0; byte < 8; byte++) {
                uint8x16_t *reg = &X[byte / 4].val[byte % 4];

                *reg = veorq_u8(*reg, k[byte / 4].val[byte % 4]);
        }

        // S-boxes (beware of endianness)
        X[1].val[3] = s(X[1].val[3], prefilter_0, postfilter_0); // s0
        X[1].val[2] = s(X[1].val[2], prefilter_0, postfilter_1); // s1
        X[1].val[1] = s(X[1].val[1], prefilter_0, postfilter_2); // s2
        X[1].val[0] = s(X[1].val[0], prefilter_3, postfilter_0); // s3
        X[0].val[3] = s(X[0].val[3], prefilter_0, postfilter_1); // s1
        X[0].val[2] = s(X[0].val[2], prefilter_0, postfilter_2); // s2
        X[0].val[1] = s(X[0].val[1], prefilter_3, postfilter_0); // s3
        X[0].val[0] = s(X[0].val[0], prefilter_0, postfilter_0); // s0

        // permutation
        X[1].val[3] = veorq_u8(X[1].val[3], X[0].val[2]);
        X[1].val[2] = veorq_u8(X[1].val[2], X[0].val[1]);
        X[1].val[1] = veorq_u8(X[1].val[1], X[0].val[0]);
        X[1].val[0] = veorq_u8(X[1].val[0], X[0].val[3]);
        X[0].val[3] = veorq_u8(X[0].val[3], X[1].val[1]);
        X[0].val[2] = veorq_u8(X[0].val[2], X[1].val[0]);
        X[0].val[1] = veorq_u8(X[0].val[1], X[1].val[3]);
        X[0].val[0] = veorq_u8(X[0].val[0], X[1].val[2]);

        X[1].val[3] = veorq_u8(X[1].val[3], X[0].val[0]);
        X[1].val[2] = veorq_u8(X[1].val[2], X[0].val[3]);
        X[1].val[1] = veorq_u8(X[1].val[1], X[0].val[2]);
        X[1].val[0] = veorq_u8(X[1].val[0], X[0].val[1]);
        X[0].val[3] = veorq_u8(X[0].val[3], X[1].val[0]);
        X[0].val[2] = veorq_u8(X[0].val[2], X[1].val[3]);
        X[0].val[1] = veorq_u8(X[0].val[1], X[1].val[2]);
        X[0].val[0] = veorq_u8(X[0].val[0], X[1].val[1]);


        // X[0] and X[1] are swapped now; this is
        // taken into account in the feistel round
}

void camellia_sliced_FL(uint8x16x4_t X[restrict 2],
                        const uint8x16x4_t kl[restrict 2])
{
        const uint8x16x4_t XL = X[1];
        const uint8x16x4_t XR = X[0];

        const uint8x16x4_t klL = kl[1];
        const uint8x16x4_t klR = kl[0];

        uint8x16x4_t YR_prerotate = {
                .val = {
                        vandq_u8(XL.val[0], klL.val[0]),
                        vandq_u8(XL.val[1], klL.val[1]),
                        vandq_u8(XL.val[2], klL.val[2]),
                        vandq_u8(XL.val[3], klL.val[3])
                }
        };

        rol32_1(&YR_prerotate);

        const uint8x16x4_t YR = {
                .val = {
                        veorq_u8(YR_prerotate.val[0], XR.val[0]),
                        veorq_u8(YR_prerotate.val[1], XR.val[1]),
                        veorq_u8(YR_prerotate.val[2], XR.val[2]),
                        veorq_u8(YR_prerotate.val[3], XR.val[3])
                }
        };

        const uint8x16x4_t YL = {
                .val = {
                        veorq_u8(vorrq_u8(YR.val[0], klR.val[0]), XL.val[0]),
                        veorq_u8(vorrq_u8(YR.val[1], klR.val[1]), XL.val[1]),
                        veorq_u8(vorrq_u8(YR.val[2], klR.val[2]), XL.val[2]),
                        veorq_u8(vorrq_u8(YR.val[3], klR.val[3]), XL.val[3])
                }
        };

        X[0] = YR;
        X[1] = YL;
}

void camellia_sliced_FL_inv(uint8x16x4_t Y[restrict 2],
                            const uint8x16x4_t kl[restrict 2])
{
        const uint8x16x4_t YL = Y[1];
        const uint8x16x4_t YR = Y[0];

        const uint8x16x4_t klL = kl[1];
        const uint8x16x4_t klR = kl[0];

        const uint8x16x4_t XL = {
                .val = {
                        veorq_u8(vorrq_u8(YR.val[0], klR.val[0]), YL.val[0]),
                        veorq_u8(vorrq_u8(YR.val[1], klR.val[1]), YL.val[1]),
                        veorq_u8(vorrq_u8(YR.val[2], klR.val[2]), YL.val[2]),
                        veorq_u8(vorrq_u8(YR.val[3], klR.val[3]), YL.val[3])
                }
        };

        uint8x16x4_t XR_prerotate = {
                .val = {
                        vandq_u8(XL.val[0], klL.val[0]),
                        vandq_u8(XL.val[1], klL.val[1]),
                        vandq_u8(XL.val[2], klL.val[2]),
                        vandq_u8(XL.val[3], klL.val[3])
                }
        };

        rol32_1(&XR_prerotate);

        const uint8x16x4_t XR = {
                .val = {
                        veorq_u8(XR_prerotate.val[0], YR.val[0]),
                        veorq_u8(XR_prerotate.val[1], YR.val[1]),
                        veorq_u8(XR_prerotate.val[2], YR.val[2]),
                        veorq_u8(XR_prerotate.val[3], YR.val[3])
                }
        };

        Y[0] = XR;
        Y[1] = XL;
}

void camellia_sliced_feistel_round(uint8x16x4_t state[restrict 4],
                                   const uint8x16x4_t kr[restrict 2])
{
        uint8x16x4_t F[2] = {
                state[0], state[1]
        };

        // F function swaps result
        camellia_sliced_F(F, kr);

        for (size_t j = 0; j < 4; j++) {
                F[0].val[j] = veorq_u8(state[3].val[j], F[0].val[j]);
                F[1].val[j] = veorq_u8(state[2].val[j], F[1].val[j]);
        }

        state[2] = state[0];
        state[3] = state[1];
        state[0] = F[1];
        state[1] = F[0];
}

void camellia_sliced_feistel_round_inv(uint8x16x4_t state[restrict 4],
                                       const uint8x16x4_t kr[restrict 2])
{
        // TODO
}

void camellia_sliced_generate_round_keys_128(struct camellia_rks_sliced_128 *restrict rks,
                                             const uint64_t key[restrict 2])
{
        // use standard key derivation
        struct camellia_rks_128 rks_128;
        camellia_spec_opt_generate_round_keys_128(&rks_128, key);

        // now pack round keys by use of vdupq_n_u8 since all bytes are the same
        // in a bytesliced representation

        // whitening and FL layer keys
        for (size_t i = 0; i < 4; i++) {
                for (size_t byte = 0; byte < 8; byte++) {
                        uint8x16_t *reg_kw = &rks->kw[i][byte / 4].val[byte % 4];
                        uint8x16_t *reg_kl = &rks->kl[i][byte / 4].val[byte % 4];

                        *reg_kw = vdupq_n_u8((rks_128.kw[i] >> (8 * byte)) & 0xff);
                        *reg_kl = vdupq_n_u8((rks_128.kl[i] >> (8 * byte)) & 0xff);
                }
        }

        // F function keys
        for (size_t i = 0; i < 18; i++) {
                for (size_t byte = 0; byte < 8; byte++) {
                        uint8x16_t *reg_ku = &rks->ku[i][byte / 4].val[byte % 4];

                        *reg_ku = vdupq_n_u8((rks_128.ku[i] >> (8 * byte)) & 0xff);
                }
        }
}

void camellia_sliced_pack(uint8x16x4_t packed[restrict 4],
                          const uint64_t x[restrict 16][2])
{
        for (size_t i = 0; i < 4; i++) {
                packed[i] = vld1q_u8_x4((uint8_t*)&x[i * 4][0]);
        }

        uint8x16x4_t packed_0[4];

        for (size_t i = 0; i < 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                        packed_0[i].val[j] = vqtbl4q_u8(packed[j], pack_group.val[i]);
                }
        }

        for (size_t i = 0; i < 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                        packed[i].val[j] = vqtbl4q_u8(packed_0[i], pack_single.val[j]);
                }
        }
}

void camellia_sliced_unpack(uint64_t x[restrict 16][2],
                            const uint8x16x4_t packed[restrict 4])
{

        uint8x16x4_t unpacked[4];
        for (size_t i = 0; i < 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                        unpacked[i].val[j] = vqtbl4q_u8(packed[j], pack_group_inv.val[i]);
                }
        }

        uint8x16x4_t unpacked_0[4];

        // pack_single_inv = pack_group
        for (size_t i = 0; i < 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                        unpacked_0[i].val[j] = vqtbl4q_u8(unpacked[i], pack_group.val[j]);
                }
        }

        for (size_t i = 0; i < 4; i++) {
                vst1q_u8_x4((uint8_t*)&x[i * 4][0], unpacked_0[i]);
        }
}


void camellia_sliced_init(void)
{
        static const uint64_t pack_group_u64[4][2] = {
                { 0x1312111003020100UL, 0x3332313023222120UL },
                { 0x1716151407060504UL, 0x3736353427262524UL },
                { 0x1b1a19180b0a0908UL, 0x3b3a39382b2a2928UL },
                { 0x1f1e1d1c0f0e0d0cUL, 0x3f3e3d3c2f2e2d2cUL },
        };

        static const uint64_t pack_group_inv_u64[4][2] = {
                { 0x3121110130201000UL, 0x3323130332221202UL },
                { 0x3525150534241404UL, 0x3727170736261606UL },
                { 0x3929190938281808UL, 0x3b2b1b0b3a2a1a0aUL },
                { 0x3d2d1d0d3c2c1c0cUL, 0x3f2f1f0f3e2e1e0eUL }
        };

        static const uint64_t pack_single_u64[4][2] = {
                { 0x1c1814100c080400UL, 0x3c3834302c282420UL },
                { 0x1d1915110d090501UL, 0x3d3935312d292521UL },
                { 0x1e1a16120e0a0602UL, 0x3e3a36322e2a2622UL },
                { 0x1f1b17130f0b0703UL, 0x3f3b37332f2b2723UL },
        };

        static const uint64_t shiftrows_inv_u64[2] = {
                0x0b0e0104070a0d00UL, 0x0306090c0f020508UL };

        static const uint64_t rol32_1_overflow_u64[2] = {
                0x0605040302010010, 0x0e0d0c0b0a090807UL };

        static const uint64_t prefilter_0_u64[2][2] = {
                { 0x862b832eed40e845UL, 0x88258d20e34ee64bUL },
                { 0x2a7bdb8aa0f15100UL, 0x2372d283a9f85809UL } };

        static const uint64_t prefilter_3_u64[2][2] = {
                { 0x25204e4b2b2e4045UL, 0x74711f1a7a7f1114UL },
                { 0x7283f8097b8af100UL, 0xdf2e55a4d6275cadUL } };

        static const uint64_t postfilter_0_u64[2][2] = {
                { 0x31c1c2323fcfcc3cUL, 0xd12122d2df2f2cdcUL },
                { 0xa8512ed77f86f900UL, 0x0cf58a73db225da4UL } };

        static const uint64_t postfilter_1_u64[2][2] = {
                { 0x628385647e9f9978UL, 0xa34244a5bf5e58b9UL },
                { 0x51a25caffe0df300UL, 0x18eb15e6b744ba49UL } };

        static const uint64_t postfilter_2_u64[2][2] = {
                { 0x98e061199fe7661eUL, 0xe8901169ef97166eUL },
                { 0x54a817ebbf43fc00UL, 0x06fa45b9ed11ae52UL } };

        pack_group          = vld1q_u8_x4((uint8_t*)pack_group_u64);
        pack_group_inv      = vld1q_u8_x4((uint8_t*)pack_group_inv_u64);
        pack_single         = vld1q_u8_x4((uint8_t*)pack_single_u64);

        lower_4_bits_mask   = vdupq_n_u8(0x0f);
        shiftrows_inv       = vld1q_u8((uint8_t*)shiftrows_inv_u64);
        rol32_1_overflow    = vld1q_u8((uint8_t*)rol32_1_overflow_u64);

        prefilter_0         = vld1q_u8_x2((uint8_t*)prefilter_0_u64);
        prefilter_3         = vld1q_u8_x2((uint8_t*)prefilter_3_u64);
        postfilter_0        = vld1q_u8_x2((uint8_t*)postfilter_0_u64);
        postfilter_1        = vld1q_u8_x2((uint8_t*)postfilter_1_u64);
        postfilter_2        = vld1q_u8_x2((uint8_t*)postfilter_2_u64);
}

void camellia_sliced_encrypt_128(uint64_t c[restrict 16][2],
                                 const uint64_t m[restrict 16][2],
                                 struct camellia_rks_sliced_128 *restrict rks)
{
        uint8x16x4_t state[4];
        camellia_sliced_pack(state, m);

        // kw0/kw1
        for (size_t byte = 0; byte < 16; byte++) {
                uint8x16_t *reg     = &state[byte / 4].val[byte % 4];
                uint8x16x4_t *key   = &rks->kw[byte / 8 + 0][(byte % 8) / 4];

                *reg = veorq_u8(*reg, key->val[byte % 4]);
        }

        for (size_t i = 0; i < 6; i++) {
                camellia_sliced_feistel_round(state, rks->ku[i + 0]);
        }

        camellia_sliced_FL(&state[0], rks->kl[0]);
        camellia_sliced_FL_inv(&state[2], rks->kl[1]);

        for (size_t i = 0; i < 6; i++) {
                camellia_sliced_feistel_round(state, rks->ku[i + 6]);
        }

        camellia_sliced_FL(&state[0], rks->kl[2]);
        camellia_sliced_FL_inv(&state[2], rks->kl[3]);

        for (size_t i = 0; i < 6; i++) {
                camellia_sliced_feistel_round(state, rks->ku[i + 12]);
        }

        // swap state[0,1] and state[2,3] (concatenation of R||L)
        uint8x16x4_t tmp = state[0];
        state[0] = state[2];
        state[2] = tmp;
        tmp = state[1];
        state[1] = state[3];
        state[3] = tmp;

        // kw2/kw3
        for (size_t byte = 0; byte < 16; byte++) {
                uint8x16_t *reg     = &state[byte / 4].val[byte % 4];
                uint8x16x4_t *key   = &rks->kw[byte / 8 + 2][(byte % 8) / 4];

                *reg = veorq_u8(*reg, key->val[byte % 4]);
        }

        camellia_sliced_unpack(c, state);
}

void camellia_sliced_decrypt_128(uint64_t m[restrict 16][2],
                                 const uint64_t c[restrict 16][2],
                                 struct camellia_rks_sliced_128 *restrict rks);
