#!/usr/bin/env python3

pl      = (0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23)
pl_inv  = (0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30)
pu      = [x + 8 for x in pl]
pu_inv  = [x + 1 for x in pl_inv]

print('0x', end='')
for i in range(15, -1, -1):
    print(f'{pl[i]:02x}', end='')
print('')

print('0x', end='')
for i in range(15, -1, -1):
    print(f'{pu[i]:02x}', end='')
print('')

print('0x', end='')
for i in range(15, -1, -1):
    print(f'{pl_inv[i]:02x}', end='')
print('')

print('0x', end='')
for i in range(15, -1, -1):
    print(f'{pu_inv[i]:02x}', end='')
print('')
