#include "gift.h"
#include "gift_sliced.h"
#include "gift_neon.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
        srand(time(NULL));
        uint64_t key[2] = { 10000UL, 24000UL };
        uint64_t m[8] = {
                rand(), rand(), rand(), rand(),
                rand(), rand(), rand(), rand()
        };
        uint64_t c[8];

        // benchmark gift_64_encrypt
        uint64_t t0, t1;
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));
        gift_64_encrypt(m[0], key);
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));
        printf("gift_64_encrypt: took %ld cycles\n", t1 - t0);

        // benchmark gift_128_encrypt
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));
        gift_128_encrypt(c, m, key);
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));
        printf("gift_128_encrypt: took %ld cycles\n", t1 - t0);

        // benchmark gift_64_sliced_encrypt
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));
        gift_64_sliced_encrypt(c, m, key);
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));
        printf("gift_64_sliced_encrypt: took %ld cycles\n", t1 - t0);
}
