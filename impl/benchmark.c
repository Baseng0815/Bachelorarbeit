#pragma clang optimize off

#include "naive/gift.h"
#include "naive/gift_sliced.h"
#include "table/gift_table.h"
#include "vector/gift_vec_sbox.h"
#include "vector/gift_vec_sliced.h"
#include "camellia/naive.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> // wall time via gettimeofday()

#include <stdint.h>
#include <arm_neon.h>

#define NL 10000 // latency measurement count
#define NT 1000000 // throughput measurement count

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

static void rand_bytes(uint8_t m[], size_t n)
{
        for (size_t i = 0; i < n; i++) {
                m[i] = rand();
        }
}

static double elapsed_seconds(const struct timeval *st, const struct timeval *et)
{
        return (et->tv_sec - st->tv_sec) + (et->tv_usec - st->tv_usec) / 1000000.0;
}

static void benchmark_gift_64(void)
{
        printf("benchmarking GIFT_64...\n");

        uint64_t key[2];
        uint64_t m;
        uint64_t round_keys[ROUNDS_GIFT_64];

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < NL; i++) {
                cycles[0] += TIME(gift_64_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_subcells(m));
                cycles[2] += TIME(gift_64_permute(m));
                cycles[3] += TIME(gift_64_encrypt(m, round_keys));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)NL / 8.0f,
               cycles[1] / (float)NL / 8.0f,
               cycles[2] / (float)NL / 8.0f,
               cycles[3] / (float)NL / 8.0f);

        struct timeval st, et;
        gettimeofday(&st, NULL);
        for (int i = 0; i < NT; i++) {
                gift_64_encrypt(m, round_keys);
        }
        gettimeofday(&et, NULL);
        double seconds = elapsed_seconds(&st, &et);
        double megs = NT * sizeof(m) / (1024 * 1024);
        printf("throughput: %f MiB/s\n", megs / seconds);
}

static void benchmark_gift_128(void)
{
        printf("benchmarking GIFT_128...\n");

        uint64_t key[2];
        uint8_t m[16], c[16];
        uint8_t round_keys[ROUNDS_GIFT_128][32];

        uint64_t cycles[4] = { 0UL };
        for (int i = 0; i < NL; i++) {
                cycles[0] += TIME(gift_128_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_128_subcells(m));
                cycles[2] += TIME(gift_128_permute(m));
                cycles[3] += TIME(gift_128_encrypt(c, m, round_keys));
        }

        printf("%f %f %f %f\n",
               cycles[0] / (float)NL / 16.0f,
               cycles[1] / (float)NL / 16.0f,
               cycles[2] / (float)NL / 16.0f,
               cycles[3] / (float)NL / 16.0f);

        struct timeval st, et;
        gettimeofday(&st, NULL);
        for (int i = 0; i < NT; i++) {
                gift_128_encrypt(c, m, round_keys);
        }
        gettimeofday(&et, NULL);
        double seconds = elapsed_seconds(&st, &et);
        double megs = NT * sizeof(m) / (1024 * 1024);
        printf("throughput: %f MiB/s\n", megs / seconds);
}

static void benchmark_gift_64_table(void)
{
        printf("benchmarking GIFT_64_TABLE...\n");

        uint64_t key[2];
        uint64_t m;
        uint64_t round_keys[ROUNDS_GIFT_64];

        uint64_t cycles[3] = { 0UL };
        for (int i = 0; i < NL; i++) {
                cycles[0] += TIME(gift_64_table_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_table_subperm(m));
                cycles[2] += TIME(gift_64_table_encrypt(m, round_keys));
        }

        printf("%f %f %f\n",
               cycles[0] / (float)NL / 8.0f,
               cycles[1] / (float)NL / 8.0f,
               cycles[2] / (float)NL / 8.0f);

        struct timeval st, et;
        gettimeofday(&st, NULL);
        for (int i = 0; i < NT; i++) {
                gift_64_table_encrypt(m, round_keys);
        }
        gettimeofday(&et, NULL);
        double seconds = elapsed_seconds(&st, &et);
        double megs = NT * sizeof(m) / (1024 * 1024);
        printf("throughput: %f MiB/s\n", megs / seconds);
}

static void benchmark_gift_64_vec_sbox(void)
{
        printf("benchmarking GIFT_64_VEC_SBOX...\n");
        gift_64_vec_sbox_init();

        uint64_t key[2];
        uint64_t m;
        uint8x16_t round_keys[ROUNDS_GIFT_64];
        uint8x16_t state;

        uint64_t cycles[6] = { 0UL };
        for (int i = 0; i < NL; i++) {
                cycles[0] += TIME(gift_64_vec_sbox_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_vec_sbox_subcells(state));
                cycles[2] += TIME(gift_64_vec_sbox_permute(state));
                cycles[3] += TIME(gift_64_vec_sbox_encrypt(m, round_keys));
                cycles[4] += TIME(gift_64_vec_sbox_bits_pack(m));
                cycles[5] += TIME(gift_64_vec_sbox_bits_unpack(state));
        }

        printf("%f %f %f %f %f %f\n",
               cycles[0] / (float)NL / 8.0f,
               cycles[1] / (float)NL / 8.0f,
               cycles[2] / (float)NL / 8.0f,
               cycles[3] / (float)NL / 8.0f,
               cycles[4] / (float)NL / 8.0f,
               cycles[5] / (float)NL / 8.0f);

        struct timeval st, et;
        gettimeofday(&st, NULL);
        for (int i = 0; i < NT; i++) {
                gift_64_vec_sbox_encrypt(m, round_keys);
        }
        gettimeofday(&et, NULL);
        double seconds = elapsed_seconds(&st, &et);
        double megs = NT * sizeof(m) / (1024 * 1024);
        printf("throughput: %f MiB/s\n", megs / seconds);
}

static void benchmark_gift_64_vec_sliced(void)
{
        printf("benchmarking GIFT_64_VEC_SLICED...\n");
        gift_64_vec_sbox_init();

        uint64_t key[2];
        uint64_t m[16], c[16];
        uint8x16x4_t round_keys[ROUNDS_GIFT_64][2];
        uint8x16x4_t state[2];

        uint64_t cycles[6] = { 0UL };
        for (int i = 0; i < NL; i++) {
                cycles[0] += TIME(gift_64_vec_sliced_generate_round_keys(round_keys, key));
                cycles[1] += TIME(gift_64_vec_sliced_subcells(state));
                cycles[2] += TIME(gift_64_vec_sliced_permute(state));
                cycles[3] += TIME(gift_64_vec_sliced_encrypt(c, m, round_keys));
                cycles[4] += TIME(gift_64_vec_sliced_bits_pack(state));
                cycles[5] += TIME(gift_64_vec_sliced_bits_unpack(state));
        }

        printf("%f %f %f %f %f %f\n",
               cycles[0] / (float)NL / 128.0f,
               cycles[1] / (float)NL / 128.0f,
               cycles[2] / (float)NL / 128.0f,
               cycles[3] / (float)NL / 128.0f,
               cycles[4] / (float)NL / 128.0f,
               cycles[5] / (float)NL / 128.0f);

        struct timeval st, et;
        gettimeofday(&st, NULL);
        for (int i = 0; i < NT; i++) {
                gift_64_vec_sliced_encrypt(c, m, round_keys);
        }
        gettimeofday(&et, NULL);
        double seconds = elapsed_seconds(&st, &et);
        double megs = NT * sizeof(m) / (1024 * 1024);
        printf("throughput: %f MiB/s\n", megs / seconds);
}

static void benchmark_camellia_naive(void)
{
        printf("Benchmaring CAMELLIA_NAIVE...\n");

        uint64_t key[2];
        uint64_t m[2], c[2];
        struct camellia_keys_128 rks;

        uint64_t cycles[2] = { 0UL };
        for (int i = 0; i < NL; i++) {
                cycles[0] += TIME(camellia_naive_generate_round_keys_128(key, &rks));
                cycles[1] += TIME(camellia_naive_encrypt_128(c, m, &rks));
        }

        printf("%f %f\n",
               cycles[0] / (float)NL / 16.0f,
               cycles[1] / (float)NL / 16.0f);
}

int main(int argc, char *argv[])
{
        srand(time(NULL));
        /* benchmark_gift_64(); */
        /* benchmark_gift_128(); */
        /* benchmark_gift_64_table(); */
        /* benchmark_gift_64_vec_sbox(); */
        /* benchmark_gift_64_vec_sliced(); */
        benchmark_camellia_naive();
}

#pragma clang optimize on
