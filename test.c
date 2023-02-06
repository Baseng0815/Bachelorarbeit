#include "gift.h"
#include "gift_sliced.h"
#include "gift_neon.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// no testing framework necessary for this small project
#define ASSERT_EQUALS(x,y)\
        if ((x) != (y)) {\
                fprintf(stderr, "ASSERT_EQUALS failed on line %d", __LINE__);\
                exit(-1);\
        }

void test_gift_64()
{
        // test encrypt to known value

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                uint64_t key[2] = { rand(), rand() };
                uint64_t m = rand();
                uint64_t c = gift_64_encrypt(m, key);
                uint64_t m1 = gift_64_decrypt(c, key);
                ASSERT_EQUALS(m, m1);
        }
}

void test_gift_128()
{
        // test encrypt to known value

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                uint64_t key[2] = { rand(), rand() };
                uint64_t m[2] = { rand(), rand() };
                uint64_t c[2];
                gift_128_encrypt(c, m, key);
                uint64_t m1[2];
                gift_128_decrypt(m1, c, key);
                ASSERT_EQUALS(m[0], m1[0]);
                ASSERT_EQUALS(m[1], m1[1]);
        }
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        test_gift_64();
        test_gift_128();
}
