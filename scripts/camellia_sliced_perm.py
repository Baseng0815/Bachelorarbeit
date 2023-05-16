#!/usr/bin/env python3

import numpy as np

first = np.array([
    (0, 1, 1, 1),
    (1, 0, 1, 1),
    (1, 1, 0, 1),
    (1, 1, 1, 0),
    (0, 1, 1, 1),
    (1, 0, 1, 1),
    (1, 1, 0, 1),
    (1, 1, 1, 0)
])

second = np.array([
    (1, 0, 0, 1),
    (1, 1, 0, 0),
    (0, 1, 1, 0),
    (0, 0, 1, 1),
    (1, 1, 1, 0),
    (0, 1, 1, 1),
    (1, 0, 1, 1),
    (1, 1, 0, 1)
])

for i in range(15, -1, -1):
    vec = np.array([(i >> 0) & 0x1, (i >> 1) & 0x1, (i >> 2) & 0x1, (i >> 3) & 0x1]).reshape(4, 1)
    result = ((first @ vec) % 2) * np.array([1, 2, 4, 8, 16, 32, 64, 128]).reshape(8, 1)
    print(hex(result.sum())[2:], end=' ')

print('')

for i in range(15, -1, -1):
    vec = np.array([(i >> 0) & 0x1, (i >> 1) & 0x1, (i >> 2) & 0x1, (i >> 3) & 0x1]).reshape(4, 1)
    result = ((second @ vec) % 2) * np.array([1, 2, 4, 8, 16, 32, 64, 128]).reshape(8, 1)
    print(hex(result.sum())[2:], end=' ')

print('')
