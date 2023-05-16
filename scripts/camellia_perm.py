#!/usr/bin/env python3

import numpy as np

z1 = np.array([1, 0, 1, 1, 0, 1, 1, 1])
z2 = np.array([1, 1, 0, 1, 1, 0, 1, 1]) ^ z1
z3 = np.array([1, 1, 1, 0, 1, 1, 0, 1]) ^ z2
z4 = np.array([0, 1, 1, 1, 1, 1, 1, 0]) ^ z3
z5 = np.array([1, 1, 0, 0, 0, 1, 1, 1]) ^ z4
z6 = np.array([0, 1, 1, 0, 1, 0, 1, 1]) ^ z5
z7 = np.array([0, 0, 1, 1, 1, 1, 0, 1]) ^ z6
z8 = np.array([1, 0, 0, 1, 1, 1, 1, 0]) ^ z7

print(z1)
print(z2)
print(z3)
print(z4)
print(z5)
print(z6)
print(z7)
print(z8)
