#!/usr/bin/env python3

for i in range(128):
    pi = 4*(i//16) + 32*((3*(i%16//4) + i%4)%4) + i%4;
    # transpose (due to our bitslice representation)
    ri = pi % 8
    ci = pi // 16
    si = (pi % 16) // 8
    ti = ri * 16 + ci * 2 + si
    print(f"dst[{ti}] = src[{i % 8}][{i // 16}][{(i % 16) // 8}]")
