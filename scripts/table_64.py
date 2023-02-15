#!/usr/bin/env python3

sbox = (
    0x1, 0xa, 0x4, 0xc, 0x6, 0xf, 0x3, 0x9,
    0x2, 0xd, 0xb, 0x7, 0x5, 0x0, 0x8, 0xe
)

permute = (
    0, 17, 34, 51, 48, 1, 18, 35, 32, 49, 2, 19, 16, 33, 50, 3,
    4, 21, 38, 55, 52, 5, 22, 39, 36, 53, 6, 23, 20, 37, 54, 7,
    8, 25, 42, 59, 56, 9, 26, 43, 40, 57, 10, 27, 24, 41, 58, 11,
    12, 29, 46, 63, 60, 13, 30, 47, 44, 61, 14, 31, 28, 45, 62, 15
)

def perm(x):
    result = 0
    for i in range(64):
        bit = (x >> i) & 0x1
        result |= bit << permute[i]
    return result

tables = []
for sbox_index in range(16):
    table = []
    for sbox_input in range(16):
        output = sbox[sbox_input]
        output = perm(output << (4 * sbox_index))
        table.append(output)
    tables.append(table)

for x in tables[1]:
    print(hex(x), end=' ')
print('')
