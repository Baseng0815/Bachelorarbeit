#include "gift.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
        uint64_t key[] = { 0x12345678UL, 0x12319132UL };
        uint64_t m = 0xbadeaffeUL;
        uint64_t c = gift_64_encrypt(m, key);
        uint64_t m1 = gift_64_decrypt(c, key);
        printf("m=%lx, c=%lx, m1=%lx\n", m, c, m1);
}
