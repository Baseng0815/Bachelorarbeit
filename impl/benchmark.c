#pragma clang optimize off

#include "naive/gift.h"
#include "naive/gift_sliced.h"
#include "table/gift_table.h"
#include "vector/gift_vec_sbox.h"
#include "vector/gift_vec_sliced.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdint.h>
#include <arm_neon.h>

#define N 10000

#define MEASURE(code)\
        {\
        uint64_t t0, t1;\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));\
        code;\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));\
        printf(#code ": took %ld cycles\n", t1 - t0);\
        }

#define TIME(code)({\
        uint64_t t0, t1;\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t0));\
        code;\
        asm volatile("mrs %[c], PMCCNTR_EL0" : [c] "=r"(t1));\
        t1 - t0;\
        })

void rand_bytes(uint8_t m[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand();
        }
}

void benchmark_gift_64(void)
{
        printf("benchmarking GIFT_64...\n");

        uint64_t key[2];
        uint64_t m;
        uint64_t round_keys[ROUNDS_GIFT_64];

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < N; i++) {
                cycles[0] += TIME(gift_64_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_permute(m));
                cycles[2] += TIME(gift_64_subcells(m));
                cycles[3] += TIME(gift_64_encrypt(m, key));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)N,
               cycles[1] / (float)N,
               cycles[2] / (float)N,
               cycles[3] / (float)N);
}

void benchmark_gift_128(void)
{
        printf("benchmarking GIFT_128...\n");

        uint64_t key[2];
        uint8_t m[32], c[32];
        uint8_t round_keys[ROUNDS_GIFT_64][32];

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < N; i++) {
                cycles[0] += TIME(gift_128_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_128_permute(m));
                cycles[2] += TIME(gift_128_subcells(m));
                cycles[3] += TIME(gift_128_encrypt(c, m, key));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)N,
               cycles[1] / (float)N,
               cycles[2] / (float)N,
               cycles[3] / (float)N);
}

void benchmark_gift_64_table(void)
{
        printf("benchmarking GIFT_64_TABLE...\n");

        uint64_t key[2];
        uint64_t m;
        uint64_t round_keys[ROUNDS_GIFT_64];

        uint64_t cycles[3] = { 0UL };
        for (int i = 0; i < N; i++) {
                cycles[0] += TIME(gift_64_table_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_table_subperm(m));
                cycles[2] += TIME(gift_64_table_encrypt(m, key));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)N,
               cycles[1] / (float)N,
               cycles[2] / (float)N,
               cycles[3] / (float)N);
}

void benchmark_gift_64_sliced(void)
{
        printf("benchmarking GIFT_64_SLICED...\n");

        uint64_t key[2];
        uint64_t m[8], c[8];
        uint64_t round_keys[ROUNDS_GIFT_64][8];

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < N; i++) {
                cycles[0] += TIME(gift_64_sliced_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_sliced_subcells(m));
                cycles[2] += TIME(gift_64_sliced_permute(m));
                cycles[3] += TIME(gift_64_sliced_encrypt(c, m, key));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)N,
               cycles[1] / (float)N,
               cycles[2] / (float)N,
               cycles[3] / (float)N);
}

void benchmark_gift_64_vec_sbox(void)
{
        printf("benchmarking GIFT_64_VEC_SBOX...\n");
        gift_64_vec_sbox_init();

        uint64_t key[2];
        uint64_t m;
        uint8x16_t round_keys[ROUNDS_GIFT_64];
        uint8x16_t state;

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < N; i++) {
                cycles[0] += TIME(gift_64_vec_sbox_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_vec_sbox_subcells(state));
                cycles[2] += TIME(gift_64_vec_sbox_permute(state));
                cycles[3] += TIME(gift_64_vec_sbox_encrypt(m, key));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)N,
               cycles[1] / (float)N,
               cycles[2] / (float)N,
               cycles[3] / (float)N);
}

void benchmark_gift_64_vec_sliced(void)
{
        printf("benchmarking GIFT_64_VEC_SLICED...\n");
        gift_64_vec_sbox_init();

        uint64_t key[2];
        uint64_t m[16], c[16];
        uint8x16x4_t round_keys[ROUNDS_GIFT_64][2];
        uint8x16x4_t state[2];

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < N; i++) {
                cycles[0] += TIME(gift_64_vec_sliced_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_vec_sliced_subcells(state));
                cycles[2] += TIME(gift_64_vec_sliced_permute(state));
                cycles[3] += TIME(gift_64_vec_sliced_encrypt(c, m, key));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)N,
               cycles[1] / (float)N,
               cycles[2] / (float)N,
               cycles[3] / (float)N);
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        benchmark_gift_64();
        /* benchmark_gift_128(); */
        /* benchmark_gift_64_table(); */
        /* benchmark_gift_64_vec_sbox(); */
        /* benchmark_gift_64_vec_sliced(); */
}

#pragma clang optimize on
