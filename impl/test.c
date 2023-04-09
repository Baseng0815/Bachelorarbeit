#include "naive/gift.h"
#include "naive/gift_sliced.h"
#include "table/gift_table.h"
#include "vector/gift_vec_sbox.h"
#include "vector/gift_vec_sliced.h"

#include "camellia/naive.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
                m[i] = rand();
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

        uint64_t rks[ROUNDS_GIFT_64];
        gift_64_generate_round_keys(rks, key);
        c = gift_64_encrypt(m, rks);
        ASSERT_EQUALS(c, c_expected);
        return;

        // test encrypt-decrypt
        printf("testing GIFT_64 encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand((uint8_t*)&m, 8);
                memset(&m, 0, sizeof(m));

                gift_64_generate_round_keys(rks, key);
                c = gift_64_encrypt(m, rks);
                uint64_t m_actual = gift_64_decrypt(c, rks);
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

        uint8_t rks[ROUNDS_GIFT_128][32];
        gift_128_generate_round_keys(rks, key);
        gift_128_encrypt(c, m, rks);
        ASSERT_TRUE(memcmp(c, c_expected, 16) == 0);

        // test encrypt-decrypt
        printf("testing GIFT_128 encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand(m, 16);

                gift_128_generate_round_keys(rks, key);
                gift_128_encrypt(c, m, rks);
                uint8_t m_actual[16];
                gift_128_decrypt(m_actual, c, rks);
                ASSERT_TRUE(memcmp(m, m_actual, 8) == 0);
        }
}

void test_gift_64_sliced(void)
{
        // test encrypt to known value (8 times the same)
        printf("testing GIFT_64_SLICED encryption to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m[8] = {
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL
        };
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c[8];
        gift_64_sliced_encrypt(c, m, key);
        for (size_t i = 0; i < 8; i++) {
                ASSERT_TRUE(memcmp(&c[i], &c_expected, 8) == 0);
        }

        // test encrypt-decrypt
        printf("testing GIFT_64_SLICED encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                for (size_t j = 0; j < 8; j++) {
                        m_rand((uint8_t*)&m, sizeof(m));
                }

                gift_64_sliced_encrypt(c, m, key);
                uint64_t m_actual[8];
                gift_64_sliced_decrypt(m_actual, c, key);
                ASSERT_TRUE(memcmp(m_actual, m, 8 * 8) == 0);
        }
}

void test_gift_64_table(void)
{
        return;
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
                m_rand((uint8_t*)&m, sizeof(m));

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

        uint8x16_t rks[ROUNDS_GIFT_64];
        gift_64_vec_sbox_generate_round_keys(rks, key);
        c = gift_64_vec_sbox_encrypt(m, rks);
        ASSERT_EQUALS(c, c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_VEC_SBOX encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand((uint8_t*)&m, sizeof(m));

                gift_64_vec_sbox_generate_round_keys(rks, key);
                c = gift_64_vec_sbox_encrypt(m, rks);
                // only encryption for table approach
                uint64_t m_actual = gift_64_vec_sbox_decrypt(c, rks);
                ASSERT_EQUALS(m, m_actual);
        }
}

void test_gift_64_vec_sliced(void)
{
        gift_64_vec_sliced_init();

        printf("testing GIFT_64_VEC_SLICED packing/unpacking...\n");
        uint64_t a[8] = { 0UL };
        uint8x16x4_t s[2] = {
                vld1q_u8_x4((uint8_t*)a), vld1q_u8_x4((uint8_t*)a),
        };
        s[0].val[0] = vdupq_n_u64(0x0123456789abcdefUL);
        s[0].val[1] = vdupq_n_u64(0x0123456789abcdefUL);
        s[0].val[2] = vdupq_n_u64(0x0123456789abcdefUL);
        s[0].val[3] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[0] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[1] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[2] = vdupq_n_u64(0x0123456789abcdefUL);
        s[1].val[3] = vdupq_n_u64(0x0123456789abcdefUL);
        gift_64_vec_sliced_bits_pack(s);
        gift_64_vec_sliced_bits_unpack(s);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[0], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[0], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[1], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[1], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[2], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[2], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[3], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[3], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[0], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[0], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[1], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[1], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[2], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[2], 1), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[3], 0), 0x0123456789abcdefUL);
        ASSERT_EQUALS(vgetq_lane_u64(s[1].val[3], 1), 0x0123456789abcdefUL);

        printf("testing GIFT_64_VEC_SLICED substitution...\n");
        gift_64_vec_sliced_bits_pack(s);
        gift_64_vec_sliced_subcells(s);
        gift_64_vec_sliced_bits_unpack(s);
        ASSERT_EQUALS(vgetq_lane_u64(s[0].val[0], 0), 0x1a4c6f392db7508eUL);

        // test encrypt to known value
        printf("testing GIFT_64_VEC_SLICED encrytion to known value...\n");
        uint64_t key[2] = { 0x5085772fe6916616UL, 0x3c9d8c18fdd20608UL };
        uint64_t m[16] = {
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
                0x4dcfd3bdd61810f0UL, 0x4dcfd3bdd61810f0UL,
        };
        uint64_t c_expected = 0xb11d30b8d39763e1UL;
        uint64_t c[16];
        uint8x16x4_t rks[ROUNDS_GIFT_64][2];
        gift_64_vec_sliced_generate_round_keys(rks, key);
        gift_64_vec_sliced_encrypt(c, m, rks);
        ASSERT_EQUALS(c[0], c_expected);
        ASSERT_EQUALS(c[1], c_expected);

        // test encrypt-decrypt
        printf("testing GIFT_64_VEC_SLICED encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                key_rand(key);
                m_rand((uint8_t*)m, sizeof(m));
                memset(m, 0, sizeof(m));

                gift_64_vec_sliced_generate_round_keys(rks, key);
                gift_64_vec_sliced_encrypt(c, m, rks);
                // only encryption for table approach
                uint64_t m_actual[16];
                gift_64_vec_sliced_decrypt(m_actual, c, rks);
                ASSERT_TRUE(memcmp(m, m_actual, sizeof(m_actual)) == 0);
        }
}

void test_camellia_naive(void)
{
        uint64_t m[2], c[2];
        uint64_t key[2];
        struct camellia_keytable rks;

        /* printf("testing CAMELLIA_NAIVE FL-FL_inv and feistel-feistel_inv...\n"); */
        /* for (int i = 0; i < 256; i++) { */
        /*         m_rand((uint8_t*)m, sizeof(m)); */
        /*         key_rand(key); */
        /*         ASSERT_EQUALS(camellia_naive_FL(camellia_naive_FL_inv(m[0], key[0]), key[0]), m[0]); */
        /*         ASSERT_EQUALS(camellia_naive_FL_inv(camellia_naive_FL(m[1], key[1]), key[1]), m[1]); */

        /*         uint64_t mc[2]; */
        /*         memcpy(mc, m, sizeof(mc)); */
        /*         camellia_naive_feistel_round(mc, key[0]); */
        /*         camellia_naive_feistel_round_inv(mc, key[0]); */
        /*         ASSERT_TRUE(memcmp(mc, m, sizeof(mc)) == 0); */
        /*         camellia_naive_feistel_round_inv(mc, key[1]); */
        /*         camellia_naive_feistel_round(mc, key[1]); */
        /*         ASSERT_TRUE(memcmp(mc, m, sizeof(mc)) == 0); */
        /* } */

        printf("testing CAMELLIA_NAIVE encrypt to known value...\n");
        key[0] = m[0] = 0x0123456789abcdefUL;
        key[1] = m[1] = 0xfedcba9876543210UL;
        uint64_t c_expected[2] = {
                0x7369965438316767UL, 0x43beea4856065708UL
        };

        camellia_naive_generate_round_keys(key, &rks);
        for (size_t i = 0; i < 18; i++) {
                printf("%lx\n", rks.ku[i]);
        }
        camellia_naive_encrypt(c, m, &rks);
        /* printf("%lx %lx\n", c[0], c[1]); */
        ASSERT_EQUALS(c[0], c_expected[0]);
        ASSERT_EQUALS(c[1], c_expected[1]);

        printf("testing CAMELLIA_NAIVE encrypt-decrypt...\n");
        for (int i = 0; i < 100; i++) {
                m_rand((uint8_t*)m, sizeof(m));
                key_rand(key);
                camellia_naive_generate_round_keys(key, &rks);

                uint64_t m_decr[2];
                camellia_naive_encrypt(c, m, &rks);
                camellia_naive_decrypt(m_decr, c, &rks);
                ASSERT_TRUE(memcmp(m_decr, m, sizeof(m_decr)) == 0);
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
        test_gift_64_vec_sliced();
        test_camellia_naive();
}
