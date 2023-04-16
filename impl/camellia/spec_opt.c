#include "spec_opt.h"
#include "spec_opt_table.h"

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

uint64_t camellia_spec_opt_F(uint64_t X, const uint64_t k)
{
        X ^= k;

        // compute P(S(X)) through large 64-bit lookup table
        uint64_t result = 0UL;
        result ^= SP0[(X >> 56) & 0xff];
        result ^= SP1[(X >> 48) & 0xff];
        result ^= SP2[(X >> 40) & 0xff];
        result ^= SP3[(X >> 32) & 0xff];
        result ^= SP4[(X >> 24) & 0xff];
        result ^= SP5[(X >> 16) & 0xff];
        result ^= SP6[(X >> 8 ) & 0xff];
        result ^= SP7[(X >> 0 ) & 0xff];

        return result;
}

uint64_t camellia_spec_opt_FL(uint64_t X, const uint64_t kl)
{
        const uint32_t XL = (X >> 32);
        const uint32_t XR = (X >> 0);

        const uint32_t klL = (kl >> 32);
        const uint32_t klR = (kl >> 0);

        const uint32_t YR = expr_rol32_1(XL & klL) ^ XR;
        const uint32_t YL = (YR | klR) ^ XL;

        return ((uint64_t)YL << 32) | (uint64_t)YR;
}

uint64_t camellia_spec_opt_FL_inv(uint64_t Y, const uint64_t kl)
{
        const uint32_t YL = (Y >> 32);
        const uint32_t YR = (Y >> 0);

        const uint32_t klL = (kl >> 32);
        const uint32_t klR = (kl >> 0);

        const uint32_t XL = (YR | klR) ^ YL;
        const uint32_t XR = expr_rol32_1(XL & klL) ^ YR;

        return ((uint64_t)XL << 32) | (uint64_t)XR;
}

void camellia_spec_opt_feistel_round(uint64_t state[2], const uint64_t kr)
{
        const uint64_t Lr = state[1] ^ camellia_spec_opt_F(state[0], kr);
        state[1] = state[0];
        state[0] = Lr;
}

void camellia_spec_opt_feistel_round_inv(uint64_t state[2], const uint64_t kr)
{
        const uint64_t Rr = state[0] ^ camellia_spec_opt_F(state[1], kr);
        state[0] = state[1];
        state[1] = Rr;
}

void camellia_spec_opt_generate_round_keys_128(const uint64_t key[restrict 2],
                                        struct camellia_keys_128 *restrict rks)
{
        uint64_t KL[2], KA[2];

        // compute KL
        memcpy(KL, key, sizeof(KL));

        // compute KA
        memcpy(KA, KL, sizeof(KA));

        /* camellia_spec_opt_feistel_round(KA, keysched_const[0]); */
        /* camellia_spec_opt_feistel_round(KA, keysched_const[1]); */
        KA[0] = camellia_spec_opt_F(KL[1] ^ camellia_spec_opt_F(KL[0], keysched_const[0]),
                                    keysched_const[1]);
        KA[1] = camellia_spec_opt_F(KL[0], keysched_const[0]);
        camellia_spec_opt_feistel_round(KA, keysched_const[2]);
        camellia_spec_opt_feistel_round(KA, keysched_const[3]);

        struct camellia_keys_128 keys;

        // KL-dependent subkeys
        keys.kw[0] = KL[0]; keys.kw[1] = KL[1];
        rol128(KL[0], KL[1], 15); // KL << 15
        keys.ku[2] = KL[0]; keys.ku[3] = KL[1];
        rol128(KL[0], KL[1], 30) // KL << 45
                keys.ku[6] = KL[0]; keys.ku[7] = KL[1];
        rol128(KL[0], KL[1], 15) // KL << 60
                keys.ku[9] = KL[1];
        rol128(KL[0], KL[1], 17) // KL << 77
                keys.kl[2] = KL[0]; keys.kl[3] = KL[1];
        rol128(KL[0], KL[1], 17) // KL << 94
                keys.ku[12] = KL[0]; keys.ku[13] = KL[1];
        rol128(KL[0], KL[1], 17) // KL << 111
                keys.ku[16] = KL[0]; keys.ku[17] = KL[1];

        // KA-dependent subkeys
        keys.ku[0] = KA[0]; keys.ku[1] = KA[1];
        rol128(KA[0], KA[1], 15); // KA << 15
                keys.ku[4] = KA[0]; keys.ku[5] = KA[1];
        rol128(KA[0], KA[1], 15); // KA << 30
                keys.kl[0] = KA[0]; keys.kl[1] = KA[1];
        rol128(KA[0], KA[1], 15) // KA << 45
                keys.ku[8] = KA[0];
        rol128(KA[0], KA[1], 15) // KA << 60
                keys.ku[10] = KA[0]; keys.ku[11] = KA[1];
        rol128(KA[0], KA[1], 34) // KA << 94
                keys.ku[14] = KA[0]; keys.ku[15] = KA[1];
        rol128(KA[0], KA[1], 17) // KA << 111
                keys.kw[2] = KA[0]; keys.kw[3] = KA[1];

        memcpy(rks, &keys, sizeof(keys));

        // KB not needed for 128 bit
}

void camellia_spec_opt_encrypt_128(uint64_t c[restrict 2],
                            const uint64_t m[restrict 2],
                            struct camellia_keys_128 *restrict rks)
{
        memcpy(c, m, sizeof(c[0]) * 2);

        c[0] ^= rks->kw[0];
        c[1] ^= rks->kw[1];

        for (int i = 0; i < 6; i++) {
                camellia_spec_opt_feistel_round(c, rks->ku[i + 0]);
        }

        c[0] = camellia_spec_opt_FL(c[0], rks->kl[0]);
        c[1] = camellia_spec_opt_FL_inv(c[1], rks->kl[1]);

        for (int i = 0; i < 6; i++) {
                camellia_spec_opt_feistel_round(c, rks->ku[i + 6]);
        }

        c[0] = camellia_spec_opt_FL(c[0], rks->kl[2]);
        c[1] = camellia_spec_opt_FL_inv(c[1], rks->kl[3]);

        for (int i = 0; i < 6; i++) {
                camellia_spec_opt_feistel_round(c, rks->ku[i + 12]);
        }

        // swap c[0] and c[1] (concatenation of R||L)
        uint64_t t = c[0];
        c[0] = c[1]; c[1] = t;

        // kw[3] is absorped
        c[0] ^= rks->kw[2];
        c[1] ^= rks->kw[3];
}

void camellia_spec_opt_decrypt_128(uint64_t m[restrict 2],
                            const uint64_t c[restrict 2],
                            struct camellia_keys_128 *restrict rks)
{
        memcpy(m, c, sizeof(m[0]) * 2);

        m[0] ^= rks->kw[2]; m[1] ^= rks->kw[3];

        // swap c[0] and c[1] (concatenation of R||L)
        uint64_t t = m[0];
        m[0] = m[1]; m[1] = t;

        for (int i = 5; i >= 0; i--) {
                camellia_spec_opt_feistel_round_inv(m, rks->ku[i + 12]);
        }

        m[1] = camellia_spec_opt_FL(m[1], rks->kl[3]);
        m[0] = camellia_spec_opt_FL_inv(m[0], rks->kl[2]);

        for (int i = 5; i >= 0; i--) {
                camellia_spec_opt_feistel_round_inv(m, rks->ku[i + 6]);
        }

        m[1] = camellia_spec_opt_FL(m[1], rks->kl[1]);
        m[0] = camellia_spec_opt_FL_inv(m[0], rks->kl[0]);

        for (int i = 5; i >= 0; i--) {
                camellia_spec_opt_feistel_round_inv(m, rks->ku[i + 0]);
        }

        m[0] ^= rks->kw[0]; m[1] ^= rks->kw[1];

}
