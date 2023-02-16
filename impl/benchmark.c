#pragma clang optimize off

#include "naive/gift.h"
#include "naive/gift_sliced.h"
#include "naive/gift_neon.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MEASURE(code, t0, t1)\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));\
        code;\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));\
        printf(#code ": took %ld cycles\n", t1 - t0);

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

void benchmark_gift_64(void)
{
        uint64_t key[2];
        key_rand(key);

        uint64_t m;
        m_rand((uint8_t*)&m, 8);

        uint64_t round_keys[ROUNDS_GIFT_64];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_64_generate_round_keys(round_keys, key), t0, t1);
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_subcells(m), t0, t1);
        }
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_64_permute(m), t0, t1);
        }
        MEASURE(gift_64_encrypt(m, key), t0, t1);
}

void benchmark_gift_128(void)
{
        uint64_t key[2];
        key_rand(key);

        uint8_t m[16];
        m_rand(m, 8);

        uint8_t c[16];
        uint8_t round_keys[ROUNDS_GIFT_128][32];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_128_generate_round_keys(round_keys, key), t0, t1);
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_128_subcells(m), t0, t1);
        }
        for (int i = 0; i < 5; i++) {
                MEASURE(gift_128_permute(m), t0, t1);
        }
        MEASURE(gift_128_encrypt(c, m, key), t0, t1);
}

void benchmark_gift_64_sliced(void)
{
        uint64_t key[2];
        key_rand(key);

        uint8_t m[16];
        m_rand(m, 8);

        uint8_t c[16];
        uint8_t round_keys[ROUNDS_GIFT_128][32];

        uint64_t t0, t1;
        // TODO take into account repeated execution due to cache effects
        MEASURE(gift_64_sliced_generate_round_keys(round_keys, key), t0, t1);
        MEASURE(gift_64_sliced_subcells(m), t0, t1);
        MEASURE(gift_64_sliced_permute(m), t0, t1);
        MEASURE(gift_64_sliced_encrypt(c, m, key), t0, t1);
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        benchmark_gift_64();
        benchmark_gift_128();
}

#pragma clang optimize on
