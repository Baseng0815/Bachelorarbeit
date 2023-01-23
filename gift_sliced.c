#include "gift_sliced.h"

#include <stdio.h>

static void swapmove(uint64_t *a, uint64_t *b, uint64_t m, int n)
{
        uint64_t t = ((*a >> n) ^ *b) & m;
        *b ^= t;
        *a ^= (t << n);
}

// this operation is its own inverse
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


uint64_t gift_64_sliced_encrypt(uint64_t m[8], const uint64_t key[2])
{
        uint64_t c[8] = {m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7]};
        bits_pack(m);
        gift_64_sliced_subcells(m);
        bits_pack(m);
        printf("%lx %lx %lx %lx %lx %lx %lx %lx\n",
               m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7]);
}

uint64_t gift_64_sliced_decrypt(uint64_t c_[8], const uint64_t key[2])
{

}
