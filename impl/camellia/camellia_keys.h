#ifndef CAMELLIA_KEYS
#define CAMELLIA_KEYS

#include <stdint.h>
#include <arm_neon.h>

struct camellia_rks_128 {
        uint64_t kw[4]; // whitening
        uint64_t ku[18]; // used in F function
        uint64_t kl[4]; // used in FL layer
};

struct camellia_rks_256 {
        uint64_t kw[4]; // whitening
        uint64_t ku[24]; // used in F function
        uint64_t kl[6]; // used in FL layer
};

struct camellia_rks_sliced_128 {
        uint8x16x4_t kw[4][2]; // whitening
        uint8x16x4_t ku[18][2]; // used in F function
        uint8x16x4_t kl[4][2]; // used in FL layer
};

#endif
