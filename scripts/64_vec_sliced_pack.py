#!/usr/bin/env python3

p = list()

for i in range(16):
    src = (i % 2) * 16 + i // 2
    p.append(src)

print('0x', end='')
for i in range(15, -1, -1):
    print(f'{p[i]:02x}', end='')
print('')

print(p)

print('0x', end='')
for i in range(31, 15, -1):
    for isrc, src in enumerate(p):
        if src == i or (src > 7 and src - 8 == i):
            print(f'{isrc:02x}', end='')

print('')
