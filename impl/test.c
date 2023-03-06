#include "naive/gift.h"
#include "naive/gift_sliced.h"
#include "table/gift_table.h"
#include "vector/gift_vec_sbox.h"
#include "vector/gift_vec_sliced.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// no testing framework necessary for this small project
#define ASSERT_EQUALS(x,y)\
        if (x != y) {\
                fprintf(stderr, "ASSERT_EQUALS failed on line %d: %lx != %lx\n", __LINE__, x, y);\
                exit(-1);\
        }

#define ASSERT_TRUE(x)\
        if (!(x)) {\
                fprintf(stderr, "ASSERT_TRUE failed on line %d", __LINE__);\
                exit(-1);\
        }

void key_rand(uint64_t k[])
{
        k[0] = rand();
        k[1] = rand();
}

void m_rand(uint8_t m[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand() & 0xf;
        }
}

void test_gift_64(void)
{
        // test encrypt to known value
        printf("testing GIFT_64 encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c;
        c = gift_64_encrypt(m, key);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64 encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand((uint8_t*)&m, 8);

                c = gift_64_encrypt(m, key);
                uint64_t m_actual = gift_64_decrypt(c, key);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_128(void)
{
        // test encrypt to known value
        printf("testing GIFT_128 encryption to known value...\n");
        uint64_t key[2] = { 0xe4cb97525e4e9ec7UL, 0xac22df007be1e75fUL };
        uint8_t m[16] = {
                0x52, 0xfa, 0x11, 0x56, 0xd7, 0x27, 0x67, 0xb5,
                0x33, 0x80, 0xce, 0xd7, 0x25, 0x85, 0x2d, 0xf2
        };
        uint8_t c_expected[16] = {
                0x87, 0x04, 0x63, 0xb0, 0x99, 0x08, 0x1e, 0x78,
                0x06, 0x7b, 0xc0, 0x97, 0xa6, 0x38, 0x1f, 0xe8
        };
        uint8_t c[16];
        gift_128_encrypt(c, m, key);
        ASSERT_TRUE(memcmp(c, c_expected, 16) == 0);

        // test encrypt-decrypt
        printf("testing GIFT_128 encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(m, 16);

                gift_128_encrypt(c, m, key);
                uint8_t m_actual[16];
                gift_128_decrypt(m_actual, c, key);
                ASSERT_TRUE(memcmp(m, m_actual, 8) == 0);
        }
}

void test_gift_64_sliced(void)
{
        // test encrypt to known value (8 times the same)
        printf("testing GIFT_64_SLICED encryption to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint8_t m[8][8] = {
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d },
                { 0xf0, 0x10, 0x18, 0xd6, 0xbd, 0xd3, 0xcf, 0x4d }
        };
        uint8_t c_expected[8] = {
                0xe1, 0x63, 0x97, 0xd3, 0xb8, 0x30, 0x1d, 0xb1
        };
        uint8_t c[8][8];
        gift_64_sliced_encrypt(c, m, key);
        for (size_t i = 0; i < 8; i++) {
                ASSERT_TRUE(memcmp(&c[i][0], c_expected, 8) == 0);
        }

        // test encrypt-decrypt
        printf("testing GIFT_64_SLICED encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                for (size_t j = 0; j < 8; j++) {
                        m_rand(&m[j][0], 8);
                }

                gift_64_sliced_encrypt(c, m, key);
                uint8_t m_actual[8][8];
                gift_64_sliced_decrypt(m_actual, c, key);
                ASSERT_TRUE(memcmp(m_actual, m, 8 * 8) == 0);
        }
}

void test_gift_64_table(void)
{
        // test encrypt to known value
        printf("testing GIFT_64_TABLE encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c;
        c = gift_64_table_encrypt(m, key);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_TABLE encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand((uint8_t*)&m, 8);

                c = gift_64_table_encrypt(m, key);
                // only encryption for table approach
                uint64_t m_actual = gift_64_decrypt(c, key);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_64_vec_sbox(void)
{
        gift_64_vec_sbox_init();

        // test encrypt to known value
        printf("testing GIFT_64_VEC_SBOX encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m = 0x4dcfd3bdd61810f0UL;
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c;
        c = gift_64_vec_sbox_encrypt(m, key);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_VEC_SBOX encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand((uint8_t*)&m, 8);

                c = gift_64_vec_sbox_encrypt(m, key);
                // only encryption for table approach
                uint64_t m_actual = gift_64_vec_sbox_decrypt(c, key);
                ASSERT_EQUALS(m, m_actual);
        }
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        test_gift_64();
        test_gift_128();
        test_gift_64_sliced();
        test_gift_64_table();
        test_gift_64_vec_sbox();
}
