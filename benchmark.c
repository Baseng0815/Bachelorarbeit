#include "gift.h"
#include "gift_sliced.h"
#include "gift_neon.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
        uint64_t m[8] = {[1] = 0x2};
        gift_64_sliced_encrypt(m, NULL);
}
