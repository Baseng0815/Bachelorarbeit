#!/usr/bin/env python3

for i in range(64):
    pi = 4*(i//16) + 16*((3*(i%16//4) + i%4)%4) + i%4;
    # simple swap to get the inverse instead
    i, pi = pi, i
    # transpose (due to our bitslice representation)
    ri = pi // 8
    ci = pi % 8
    ti = ci * 8 + ri
    print(f"dst[{ti}] = src[{i%8}][{i//8}]")
