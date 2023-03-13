#!/usr/bin/env python3

p = ((0, 4, 8, 12, 3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13),
     (1, 5, 9, 13, 0, 4, 8, 12, 3, 7, 11, 15, 2, 6, 10, 14),
     (2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12, 3, 7, 11, 15),
     (3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12))

p_inv = ((0, 12, 8, 4, 1, 13, 9, 5, 2, 14, 10, 6, 3, 15, 11, 7),
         (4, 0, 12, 8, 5, 1, 13, 9, 6, 2, 14, 10, 7, 3, 15, 11),
         (8, 4, 0, 12, 9, 5, 1, 13, 10, 6, 2, 14, 11, 7, 3, 15),
         (12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3))

regs = [list() for _ in range(8)]
for r in range(4):
    # r + 0, low
    for byte in range(14, -1, -2):
        byte_perm = p[r][byte]
        regs[r].append(byte_perm // 2 if byte_perm % 2 == 0 else byte_perm // 2 + 16)

    # r + 4, low
    for byte in range(15, -1, -2):
        byte_perm = p[r][byte]
        regs[r + 4].append(byte_perm // 2 if byte_perm % 2 == 0 else byte_perm // 2 + 16)

for r in range(8):
    for v in range(8):
        print(f"{regs[r][v]:02x}", end='')
    print('')

# inverse

regs = [list() for _ in range(8)]
for r in range(4):
    # r + 0, low
    for byte in range(14, -1, -2):
        byte_perm = p_inv[r][byte]
        regs[r].append(byte_perm // 2 if byte_perm % 2 == 0 else byte_perm // 2 + 16)

    # r + 4, low
    for byte in range(15, -1, -2):
        byte_perm = p_inv[r][byte]
        regs[r + 4].append(byte_perm // 2 if byte_perm % 2 == 0 else byte_perm // 2 + 16)

for r in range(8):
    for v in range(8):
        print(f"{regs[r][v]:02x}", end='')
    print('')
