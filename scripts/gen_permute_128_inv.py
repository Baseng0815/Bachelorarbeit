#!/usr/bin/env python3

a = [0 for _ in range(128)]
for i in range(128):
    pi = 4*(i//16) + 32*((3*(i%16//4) + i%4)%4) + i%4;
    a[pi] = i

print(a)
