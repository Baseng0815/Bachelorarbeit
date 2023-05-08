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

static uint64_t rol1_tbl_u64[] = {
        0x060504030201001fUL, 0x0e0d0c0b0a090807UL
};

static uint64_t rol8_tbl_u64[] = {
        0x1f1e1d1c1b1a1918UL, 0x0706050403020100UL
};

static uint64_t shiftrows_inv_u64[2] = {
        0x0b0e0104070a0d00UL, 0x0306090c0f020508UL
};

static uint64_t prefilter_0_u64[][2] = {
        { 0x862b832eed40e845UL, 0x88258d20e34ee64bUL },
        { 0x2a7bdb8aa0f15100UL, 0x2372d283a9f85809UL }
};

static uint64_t prefilter_3_u64[][2] = {
        { 0x25204e4b2b2e4045UL, 0x74711f1a7a7f1114UL },
        { 0x7283f8097b8af100UL, 0xdf2e55a4d6275cadUL }
};

static uint64_t postfilter_0_u64[][2] = {
        { 0x31c1c2323fcfcc3cUL, 0xd12122d2df2f2cdcUL },
        { 0xa8512ed77f86f900UL, 0x0cf58a73db225da4UL }
};

static uint64_t postfilter_1_u64[][2] = {
        { 0x628385647e9f9978UL, 0xa34244a5bf5e58b9UL },
        { 0x51a25caffe0df300UL, 0x18eb15e6b744ba49UL }
};

static uint64_t postfilter_2_u64[][2] = {
        { 0x98e061199fe7661eUL, 0xe8901169ef97166eUL },
        { 0x54a817ebbf43fc00UL, 0x06fa45b9ed11ae52UL }
};

static uint8x16_t rol1_tbl;
static uint8x16_t rol8_tbl;
static uint8x16_t lower_4_bits_mask;
static uint8x16_t shiftrows_inv;

// store two for low ([0]) and high ([1]) bits of matrix multiplication
static uint8x16x2_t prefilter_0;
static uint8x16_t   prefilter_0_xor;
static uint8x16x2_t prefilter_3;  // s3(x) = s0(x <<< 1)
static uint8x16x2_t postfilter_0;
static uint8x16_t   postfilter_0_xor;
static uint8x16x2_t postfilter_1; // s1(x) = s0(x) <<< 1
static uint8x16x2_t postfilter_2; // s2(x) = s0(x) >>> 1

// rotates for values larger than 64 are done by swapping array elements
#define rol128(_a, _b, _n){\
        uint64_t _t = _b;\
        (_b) = ((_b) << (_n)) | ((_a) >> (64 - (_n)));\
        (_a) = ((_a) << (_n)) | ((_t) >> (64 - (_n)));\
}

#define expr_rol32_1(_a)\
        (((_a) << 1) | ((_a) >> 31))

#define expr_rol32_8(_a)\
        (((_a) << 8) | ((_a) >> 24))

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
                        const uint8x16x4_t kl[restrict 2]);
void camellia_sliced_FL_inv(uint8x16x4_t X[restrict 2],
                            const uint8x16x4_t kl[restrict 2]);
void camellia_sliced_feistel_round(uint8x16x4_t state[restrict 2],
                                   const uint8x16x4_t kr[restrict 2]);
void camellia_sliced_feistel_round_inv(uint8x16x4_t state[restrict 2],
                                       const uint8x16x4_t kr[restrict 2]);

void camellia_sliced_generate_round_keys_128(const uint64_t key[restrict 2],
                                             struct camellia_rks_sliced_128 *restrict rks);

void camellia_sliced_init(void)
{
        rol1_tbl = vld1q_u8((uint8_t*)&rol1_tbl_u64[0]);
        rol8_tbl = vld1q_u8((uint8_t*)&rol8_tbl_u64[0]);
        lower_4_bits_mask = vdupq_n_u8(0x0f);
        shiftrows_inv       = vld1q_u8((uint8_t*)&shiftrows_inv_u64[0]);

        prefilter_0         = vld1q_u8_x2((uint8_t*)&prefilter_0_u64[0]);
        prefilter_0_xor     = vdupq_n_u8(0xa2);
        prefilter_3         = vld1q_u8_x2((uint8_t*)&prefilter_3_u64[0]);
        postfilter_0        = vld1q_u8_x2((uint8_t*)&postfilter_0_u64[0]);
        postfilter_0_xor    = vdupq_n_u8(0x3c);
        postfilter_1        = vld1q_u8_x2((uint8_t*)&postfilter_1_u64[0]);
        postfilter_2        = vld1q_u8_x2((uint8_t*)&postfilter_2_u64[0]);
}

void camellia_sliced_encrypt_128(uint64_t c[restrict 2][8],
                                 const uint64_t m[restrict 2][8],
                                 struct camellia_rks_sliced_128 *restrict rks);
