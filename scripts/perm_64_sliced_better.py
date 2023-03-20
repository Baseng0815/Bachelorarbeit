#!/usr/bin/env python3

p0 = (0, 12, 8, 4, 1, 13, 9, 5, 2, 14, 10, 6, 3, 15, 11, 7)
p1 = (4, 0, 12, 8, 5, 1, 13, 9, 6, 2, 14, 10, 7, 3, 15, 11)
p2 = (8, 4, 0, 12, 9, 5, 1, 13, 10, 6, 2, 14, 11, 7, 3, 15)
p3 = (12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3)

print('0x', end='')
for i in range(15, -1, -1):
    for j, p0j in enumerate(p0):
        if p0j == i:
            print(f"{j:02x}", end='')

print('')

print('0x', end='')
for i in range(16):
    for j, p1j in enumerate(p1):
        if p1j == i:
            print(f"{j:02x}", end='')

print('')

print('0x', end='')
for i in range(16):
    for j, p2j in enumerate(p2):
        if p2j == i:
            print(f"{j:02x}", end='')


print('')

print('0x', end='')
for i in range(16):
    for j, p3j in enumerate(p3):
        if p3j == i:
            print(f"{j:02x}", end='')


print('\n0x', end='')

for i in range(16):
    print(f"{p0[15 - i]:02x}", end='')

print('\n0x', end='')

for i in range(16):
    print(f"{p1[15 - i]:02x}", end='')

print('\n0x', end='')

for i in range(16):
    print(f"{p2[15 - i]:02x}", end='')

print('\n0x', end='')

for i in range(16):
    print(f"{p3[15 - i]:02x}", end='')

print('')
