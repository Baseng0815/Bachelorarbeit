#include "gift.h"
#include "gift_sliced.h"
#include "gift_neon.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// no testing framework necessary for this small project
#define ASSERT_EQUALS(x,y)\
        if ((x) != (y)) {\
                fprintf(stderr, "ASSERT_EQUALS failed on line %d\n", __LINE__);\
                exit(-1);\
        }

void key_rand(uint64_t *k)
{
        k[0] = rand();
        k[1] = rand();
}

void m_rand(uint64_t *m, size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand() | ((uint64_t)rand() << 32);
        }
}

void test_gift_64()
{
        printf("testing GIFT_64...\n");

        // test encrypt to known value
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c = gift_64_encrypt(m, key);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(&m, 1);

                c = gift_64_encrypt(m, key);
                uint64_t m_actual = gift_64_decrypt(c, key);
                printf("m=%016lx, c=%016lx, m1=%016lx\n", m, c, m_actual);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_128()
{
        printf("testing GIFT_128...\n");

        // test encrypt to known value
        uint64_t key[2] = { 0x7d9a57f754a60169UL, 0x0e17c3f9eb2c96dcUL };
        uint64_t m[2] = { 0x82be2bde4907f80eUL, 0x1e8353722cede03dUL };
        uint64_t c_expected[2] = { 0x9bf471025027823cUL, 0x096a37a7d280df1cUL };
        uint64_t c[2];
        gift_128_encrypt(c, m, key);
        ASSERT_EQUALS(c[0], c_expected[0]);
        ASSERT_EQUALS(c[1], c_expected[1]);

        // test encrypt-decrypt
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(m, 2);

                gift_128_encrypt(c, m, key);
                uint64_t m_actual[2];
                gift_128_decrypt(m_actual, c, key);
                printf("m=[%016lx, %016lx], c=[%016lx, %016lx], m1=[%016lx, %016lx]\n",
                       m[0], m[1], c[0], c[1], m_actual[0], m_actual[1]);
                ASSERT_EQUALS(m[0], m_actual[0]);
                ASSERT_EQUALS(m[1], m_actual[1]);
        }
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        test_gift_64();
        test_gift_128();
}
