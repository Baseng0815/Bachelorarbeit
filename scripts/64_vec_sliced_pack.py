#!/usr/bin/env python3

print('0x', end='')
for i in range(15, -1, -1):
    src = (i % 2) * 16 + i // 2 + 8
    print(f'{src:02x}', end='')

print('')
