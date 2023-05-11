#include "bytesliced.h"
#include "bytesliced.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint64_t keysched_const[] = {
        0xa09e667f3bcc908bUL,
        0xb67ae8584caa73b2UL,
        0xc6ef372fe94f82beUL,
        0x54ff53a5f1d36f1cUL,
        0x10e527fade682d1dUL,
        0xb05688c2b3e6c1fdUL
};


static uint8x16x4_t pack_group;
static uint8x16x4_t pack_single;

static uint8x16_t lower_4_bits_mask;
static uint8x16_t shiftrows_inv;

// store two for low ([0]) and high ([1]) bits of matrix multiplication
static uint8x16x2_t prefilter_0;
static uint8x16x2_t prefilter_3;  // s3(x) = s0(x <<< 1)
static uint8x16x2_t postfilter_0;
static uint8x16x2_t postfilter_1; // s1(x) = s0(x) <<< 1
static uint8x16x2_t postfilter_2; // s2(x) = s0(x) >>> 1

// rotates for values larger than 64 are done by swapping array elements
#define rol128(_a, _b, _n){\
        uint64_t _t = _b;\
        (_b) = ((_b) << (_n)) | ((_a) >> (64 - (_n)));\
        (_a) = ((_a) << (_n)) | ((_t) >> (64 - (_n)));\
}


#define expr_rol32_8(_a)\
        (((_a) << 8) | ((_a) >> 24))

static void rol32_1(uint8x16x4_t *a)
{
        // TODO implement
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
        X[0].val[0] = s(X[0].val[0], prefilter_0, postfilter_0);
        X[0].val[1] = s(X[0].val[1], prefilter_0, postfilter_1);
        X[0].val[2] = s(X[0].val[2], prefilter_0, postfilter_2);
        X[0].val[3] = s(X[0].val[3], prefilter_3, postfilter_0);
        X[1].val[0] = s(X[1].val[0], prefilter_0, postfilter_1);
        X[1].val[1] = s(X[1].val[1], prefilter_0, postfilter_2);
        X[1].val[2] = s(X[1].val[2], prefilter_3, postfilter_0);
        X[1].val[3] = s(X[1].val[3], prefilter_0, postfilter_0);
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

        camellia_sliced_F(F, kr);

        for (size_t i = 0; i < 2; i++) {
                for (size_t j = 0; j < 4; j++) {
                        F[i].val[j] = veorq_u8(state[i + 2].val[j], F[i].val[j]);
                }
        }

        state[2] = state[0];
        state[3] = state[1];
        state[0] = F[0];
        state[1] = F[1];
}

void camellia_sliced_feistel_round_inv(uint8x16x4_t state[restrict 4],
                                       const uint8x16x4_t kr[restrict 2])
{
        uint8x16x4_t F[2] = {
                state[2], state[3]
        };

        camellia_sliced_F(F, kr);

        for (size_t i = 0; i < 2; i++) {
                for (size_t j = 0; j < 4; j++) {
                        F[i].val[j] = veorq_u8(state[i].val[j], F[i].val[j]);
                }
        }

        state[0] = state[2];
        state[1] = state[3];
        state[2] = F[0];
        state[3] = F[1];
}

void camellia_sliced_generate_round_keys_128(const uint64_t key[restrict 2],
                                             struct camellia_rks_sliced_128 *restrict rks)
{
        /* uint64_t KL[2], KA[2]; */
        /* memcpy(KL, key, sizeof(KL)); */
        /* memcpy(KA, key, sizeof(KA)); */

        /* // compute KA */
        /* // TODO solve this in an adequate manner */
        /* camellia_naive_feistel_round(KA, keysched_const[0]); */
        /* camellia_naive_feistel_round(KA, keysched_const[1]); */
        /* KA[0] ^= KL[0], KA[1] ^= KL[1]; */
        /* camellia_naive_feistel_round(KA, keysched_const[2]); */
        /* camellia_naive_feistel_round(KA, keysched_const[3]); */

        /* // KB not needed for 128 bit */

        /* // pre-whitening */
        /* rks->kw[0] = KL[0]; rks->kw[1] = KL[1]; */

        /* // feistel rounds 0-5 */
        /* rks->ku[0] = KA[0]; rks->ku[1] = KA[1]; */
        /* rol128(KL[0], KL[1], 15); // KL << 15 */
        /* rks->ku[2] = KL[0]; rks->ku[3] = KL[1]; */
        /* rol128(KA[0], KA[1], 15); // KA << 15 */
        /* rks->ku[4] = KA[0]; rks->ku[5] = KA[1]; */

        /* // FL/FL-1 */
        /* rol128(KA[0], KA[1], 15); // KA << 30 */
        /* rks->kl[0] = KA[0]; rks->kl[1] = KA[1]; */

        /* // feistel rounds 6-11 */
        /* rol128(KL[0], KL[1], 30) // KL << 45 */
        /*         rks->ku[6] = KL[0]; rks->ku[7] = KL[1]; */
        /* rol128(KA[0], KA[1], 15) // KA << 45 */
        /*         rks->ku[8] = KA[0]; */
        /* rol128(KL[0], KL[1], 15) // KL << 60 */
        /*         rks->ku[9] = KL[1]; */
        /* rol128(KA[0], KA[1], 15) // KA << 60 */
        /*         rks->ku[10] = KA[0]; rks->ku[11] = KA[1]; */

        /* // FL/FL-1 */
        /* rol128(KL[0], KL[1], 17) // KL << 77 */
        /*         rks->kl[2] = KL[0]; rks->kl[3] = KL[1]; */

        /* // feistel rounds 12-17 */
        /* rol128(KL[0], KL[1], 17) // KL << 94 */
        /*         rks->ku[12] = KL[0]; rks->ku[13] = KL[1]; */
        /* rol128(KA[0], KA[1], 34) // KA << 94 */
        /*         rks->ku[14] = KA[0]; rks->ku[15] = KA[1]; */
        /* rol128(KL[0], KL[1], 17) // KL << 111 */
        /*         rks->ku[16] = KL[0]; rks->ku[17] = KL[1]; */

        /* // post-whitening */
        /* rol128(KA[0], KA[1], 17) // KA << 111 */
        /*         rks->kw[2] = KA[0]; rks->kw[3] = KA[1]; */
}

void camellia_sliced_pack(uint8x16x4_t packed[restrict 4],
                          const uint64_t x[restrict 16][2])
{
        for (size_t i = 0; i < 4; i++) {
                packed[i] = vld1q_u8_x4((uint8_t*)&x[0][i * 4]);
        }

        uint8x16x4_t packed_copy[4];
        memcpy(packed_copy, packed, 4 * sizeof(packed[0]));

        for (size_t i = 0; i < 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                        packed[i].val[j] = vqtbl4q_u8(packed_copy[j], pack_group.val[i]);
                }
        }

        memcpy(packed_copy, packed, 4 * sizeof(packed[0]));

        for (size_t i = 0; i < 4; i++) {
                for (size_t j = 0; j < 4; j++) {
                        packed[i].val[j] = vqtbl4q_u8(packed_copy[i], pack_single.val[j]);
                }
        }
}

void camellia_sliced_unpack(uint64_t x[restrict 16][2],
                            const uint8x16x4_t packed[restrict 4])
{

}


void camellia_sliced_init(void)
{
        const uint64_t pack_group_u64[4][2] = {
                { 0x1312111003020100UL, 0x3332313023222120UL },
                { 0x1716151407060504UL, 0x3736353427262524UL },
                { 0x1b1a19180b0a0908UL, 0x3b3a39382b2a2928UL },
                { 0x1f1e1d1c0f0e0d0cUL, 0x3f3e3d3c2f2e2d2cUL },
        };

        const uint64_t pack_single_u64[4][2] = {
                { 0x1c1814100c080400UL, 0x3c3834302c282420UL },
                { 0x1d1915110d090501UL, 0x3d3935312d292521UL },
                { 0x1e1a16120e0a0602UL, 0x3e3a36322e2a2622UL },
                { 0x1f1b17130f0b0703UL, 0x3f3b37332f2b2723UL },
        };

        const uint64_t shiftrows_inv_u64[2] = {
                0x0b0e0104070a0d00UL, 0x0306090c0f020508UL };

        const uint64_t prefilter_0_u64[][2] = {
                { 0x862b832eed40e845UL, 0x88258d20e34ee64bUL },
                { 0x2a7bdb8aa0f15100UL, 0x2372d283a9f85809UL } };

        const uint64_t prefilter_3_u64[][2] = {
                { 0x25204e4b2b2e4045UL, 0x74711f1a7a7f1114UL },
                { 0x7283f8097b8af100UL, 0xdf2e55a4d6275cadUL } };

        const uint64_t postfilter_0_u64[][2] = {
                { 0x31c1c2323fcfcc3cUL, 0xd12122d2df2f2cdcUL },
                { 0xa8512ed77f86f900UL, 0x0cf58a73db225da4UL } };

        const uint64_t postfilter_1_u64[][2] = {
                { 0x628385647e9f9978UL, 0xa34244a5bf5e58b9UL },
                { 0x51a25caffe0df300UL, 0x18eb15e6b744ba49UL } };

        const uint64_t postfilter_2_u64[][2] = {
                { 0x98e061199fe7661eUL, 0xe8901169ef97166eUL },
                { 0x54a817ebbf43fc00UL, 0x06fa45b9ed11ae52UL } };

        pack_group          = vld1q_u8_x4((uint8_t*)&pack_group_u64[0]);
        pack_single         = vld1q_u8_x4((uint8_t*)&pack_single_u64[0]);

        lower_4_bits_mask   = vdupq_n_u8(0x0f);
        shiftrows_inv       = vld1q_u8((uint8_t*)&shiftrows_inv_u64[0]);

        prefilter_0         = vld1q_u8_x2((uint8_t*)&prefilter_0_u64[0]);
        prefilter_3         = vld1q_u8_x2((uint8_t*)&prefilter_3_u64[0]);
        postfilter_0        = vld1q_u8_x2((uint8_t*)&postfilter_0_u64[0]);
        postfilter_1        = vld1q_u8_x2((uint8_t*)&postfilter_1_u64[0]);
        postfilter_2        = vld1q_u8_x2((uint8_t*)&postfilter_2_u64[0]);
}

void camellia_sliced_encrypt_128(uint64_t c[restrict 16][2],
                                 const uint64_t m[restrict 16][2],
                                 struct camellia_rks_sliced_128 *restrict rks);

void camellia_sliced_decrypt_128(uint64_t m[restrict 16][2],
                                 const uint64_t c[restrict 16][2],
                                 struct camellia_rks_sliced_128 *restrict rks);
