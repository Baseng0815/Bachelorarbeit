#include "gift_sliced.h"

#include <stdio.h>
#include <string.h>

static void swapmove(uint64_t *a, uint64_t *b, uint64_t m, int n)
{
        uint64_t t = ((*a >> n) ^ *b) & m;
        *b ^= t;
        *a ^= (t << n);
}

// this operation is its own inverse
// m[0] = a0 b0 c0 d0 e0 f0 g0 h0 a8 b8 c8 d8 e8 f8 g8 h8 ...
// m[1] = a1 b1 c1 d1 e1 f1 g1 h1 a9 b9 c9 d9 e9 f9 g9 h9 ...
// ...
static void bits_pack(uint64_t m[8])
{
        // take care not to shift mask bits out of the register
        swapmove(&m[0], &m[1], 0x5555555555555555UL, 1);
        swapmove(&m[2], &m[3], 0x5555555555555555UL, 1);
        swapmove(&m[4], &m[5], 0x5555555555555555UL, 1);
        swapmove(&m[6], &m[7], 0x5555555555555555UL, 1);

        swapmove(&m[0], &m[2], 0x3333333333333333UL, 2);
        swapmove(&m[1], &m[3], 0x3333333333333333UL, 2);
        swapmove(&m[4], &m[6], 0x3333333333333333UL, 2);
        swapmove(&m[5], &m[7], 0x3333333333333333UL, 2);

        // make bytes (a0 b0 c0 d0 a4 b4 c4 d4 -> a0 b0 c0 d0 e0 f0 g0 h0)
        swapmove(&m[0], &m[4], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(&m[2], &m[6], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(&m[1], &m[5], 0x0f0f0f0f0f0f0f0fUL, 4);
        swapmove(&m[3], &m[7], 0x0f0f0f0f0f0f0f0fUL, 4);
}

static void gift_64_sliced_subcells(uint64_t s[8])
{
        for (size_t i = 0; i < 2; i++) {
                size_t j = i * 4;
                s[j + 1]    ^= s[j + 0] & s[j + 1];
                uint64_t t  =  s[j + 0] ^ (s[j + 1] & s[j + 3]);
                s[j + 2]    ^= t        | s[j + 1];
                s[j + 0]    =  s[j + 3] ^ s[j + 2];
                s[j + 1]    ^= s[j + 0];
                s[j + 0]    = ~s[j + 0];
                s[j + 2]    ^= t        & s[j + 1];
                s[j + 3]    =  t;
        }
}

static void gift_64_permute(uint64_t s[8])
{
        uint8_t src[8][8];
        uint8_t *dst = (uint8_t*)s;
        memcpy(src, s, sizeof(src));

        dst[0]  = src[0][0]; dst[10] = src[1][0]; dst[20] = src[2][0];
        dst[30] = src[3][0]; dst[6]  = src[4][0]; dst[8]  = src[5][0];
        dst[18] = src[6][0]; dst[28] = src[7][0]; dst[4]  = src[0][1];
        dst[14] = src[1][1]; dst[16] = src[2][1]; dst[26] = src[3][1];
        dst[2]  = src[4][1]; dst[12] = src[5][1]; dst[22] = src[6][1];
        dst[24] = src[7][1]; dst[32] = src[0][2]; dst[42] = src[1][2];
        dst[52] = src[2][2]; dst[62] = src[3][2]; dst[38] = src[4][2];
        dst[40] = src[5][2]; dst[50] = src[6][2]; dst[60] = src[7][2];
        dst[36] = src[0][3]; dst[46] = src[1][3]; dst[48] = src[2][3];
        dst[58] = src[3][3]; dst[34] = src[4][3]; dst[44] = src[5][3];
        dst[54] = src[6][3]; dst[56] = src[7][3]; dst[1]  = src[0][4];
        dst[11] = src[1][4]; dst[21] = src[2][4]; dst[31] = src[3][4];
        dst[7]  = src[4][4]; dst[9]  = src[5][4]; dst[19] = src[6][4];
        dst[29] = src[7][4]; dst[5]  = src[0][5]; dst[15] = src[1][5];
        dst[17] = src[2][5]; dst[27] = src[3][5]; dst[3]  = src[4][5];
        dst[13] = src[5][5]; dst[23] = src[6][5]; dst[25] = src[7][5];
        dst[33] = src[0][6]; dst[43] = src[1][6]; dst[53] = src[2][6];
        dst[63] = src[3][6]; dst[39] = src[4][6]; dst[41] = src[5][6];
        dst[51] = src[6][6]; dst[61] = src[7][6]; dst[37] = src[0][7];
        dst[47] = src[1][7]; dst[49] = src[2][7]; dst[59] = src[3][7];
        dst[35] = src[4][7]; dst[45] = src[5][7]; dst[55] = src[6][7];
        dst[57] = src[7][7];
}

uint64_t gift_64_sliced_encrypt(uint64_t m[8], const uint64_t key[2])
{
        uint64_t c[8] = {m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7]};
        bits_pack(m);
        /* gift_64_sliced_subcells(m); */
        gift_64_permute(m);
        bits_pack(m);
        printf("%lx %lx %lx %lx %lx %lx %lx %lx\n",
               m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7]);
}

uint64_t gift_64_sliced_decrypt(uint64_t c_[8], const uint64_t key[2])
{

}
