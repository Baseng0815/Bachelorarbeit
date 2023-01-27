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
        uint64_t m[8] = {rand(), rand()};
        uint64_t c[8];
        gift_64_sliced_encrypt(c, m, key);
        printf("%lx %lx\n", c[0], c[1]);
        printf("%lx", gift_64_encrypt(m[0], key));
}
