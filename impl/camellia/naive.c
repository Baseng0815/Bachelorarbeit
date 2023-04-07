#include "naive.h"

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
#define rol128(_b, _a, _n){\
        uint64_t _t = _a >> (64 - _n);\
        _a = (_a << _n) | (_b >> (64 - _n));\
        _b = (_b << _n) | _t;\
}

#define rol32expr(_a, _n)\
        ((_a << _n) | (_a >> (32 - _n)))

int main(int argc, char *argv[])
{

}

uint64_t camellia_naive_S(uint64_t X, uint64_t k)
{

}

uint64_t camellia_naive_P(uint64_t X, uint64_t k)
{

}

uint64_t camellia_naive_F(uint64_t X, uint64_t k)
{

}

uint64_t camellia_naive_FL(uint64_t X, uint64_t kl)
{
        uint32_t XL = (X >> 32);
        uint32_t XR = X & 0xffffffff;

        uint32_t klL = (kl >> 32);
        uint32_t klR = kl & 0xffffffff;

        uint32_t YR = rol32expr(XL & klL, 1) ^ XR;
        uint32_t YL = (YR | klR) ^ XL;

        return (uint64_t)YR | ((uint64_t)YL << 32);
}

uint64_t camellia_naive_FL_inv(uint64_t Y, uint64_t kl)
{
        uint32_t YL = (Y >> 32);
        uint32_t YR = Y & 0xffffffff;

        uint32_t klL = (kl >> 32);
        uint32_t klR = kl & 0xffffffff;

        uint32_t XL = (YR | klR) ^ YL;
        uint32_t XR = rol32expr(XL & klL, 1) ^ YR;

        return (uint64_t)XR | ((uint64_t)XL << 32);
}

void camellia_naive_feistel_round(uint64_t state[2], uint64_t kr)
{
        uint64_t Lr = state[0] ^ camellia_naive_F(state[1], kr);
        state[0] = state[1];
        state[1] = Lr;
}

void camellia_naive_generate_round_keys(const uint64_t key[restrict 2],
                                        struct camellia_keytable *restrict rks)
{
        uint64_t KL[2], KA[2], KB[2];
        memcpy(KL, key, sizeof(KL));
        memcpy(KA, key, sizeof(KA));

        // compute KA
        camellia_naive_feistel_round(KA, keysched_const[0]);
        camellia_naive_feistel_round(KA, keysched_const[1]);
        KA[0] ^= KL[0], KA[1] ^= KL[1];
        camellia_naive_feistel_round(KA, keysched_const[2]);
        camellia_naive_feistel_round(KA, keysched_const[3]);

        // compute KB
        memcpy(KB, KA, sizeof(KB));
        camellia_naive_feistel_round(KB, keysched_const[4]);
        camellia_naive_feistel_round(KB, keysched_const[5]);

        // pre-whitening
        rks->kw[0] = KL[1]; rks->kw[1] = KL[0];

        // feistel rounds 0-5
        rks->ku[0] = KA[1]; rks->ku[1] = KA[0];
        rol128(KL[1], KL[0], 15); // KL << 15
        rks->ku[2] = KL[1]; rks->ku[3] = KL[0];
        rol128(KA[1], KA[0], 15); // KA << 15
        rks->ku[4] = KA[1]; rks->ku[5] = KA[0];

        // FL/FL-1
        rol128(KA[1], KA[0], 15); // KA << 30
        rks->kl[0] = KA[1]; rks->kl[1] = KA[0];

        // feistel rounds 6-11
        rol128(KL[1], KL[0], 30) // KL << 45
        rks->ku[6] = KL[1]; rks->ku[7] = KL[0];
        rol128(KA[1], KA[0], 15) // KA << 45
        rks->ku[8] = KA[1];
        rol128(KL[1], KL[0], 15) // KL << 60
        rks->ku[9] = KL[0];
        rol128(KA[1], KA[0], 15) // KA << 60
        rks->ku[10] = KL[1]; rks->ku[11] = KL[0];

        // FL/FL-1
        rol128(KL[1], KL[0], 17) // KL << 77
        rks->kl[2] = KL[1]; rks->kl[3] = KL[0];

        // feistel rounds 12-17
        rol128(KL[1], KL[0], 17) // KL << 94
        rks->ku[12] = KL[1]; rks->ku[13] = KL[0];
        rol128(KA[1], KA[0], 34) // KA << 94
        rks->ku[14] = KA[1]; rks->ku[15] = KA[0];
        rol128(KL[1], KL[0], 17) // KL << 111
        rks->ku[16] = KL[1]; rks->ku[17] = KL[0];

        // post-whitening
        rol128(KA[1], KA[0], 17) // KA << 111
        rks->kw[2] = KL[1]; rks->kw[3] = KL[0];
}

void camellia_naive_encrypt(uint64_t m[restrict 2],
                            uint64_t key[restrict 2])
{

}
