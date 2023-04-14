#ifndef CAMELLIA_KEYS
#define CAMELLIA_KEYS

#include <stdint.h>

struct camellia_keys_128 {
        uint64_t kw[4]; // whitening
        uint64_t ku[18]; // used in F function
        int64_t kl[4]; // used in FL layer
};

struct camellia_keys_256 {
        uint64_t kw[4]; // whitening
        uint64_t ku[24]; // used in F function
        uint64_t kl[6]; // used in FL layer
};


#endif
