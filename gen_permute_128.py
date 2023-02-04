#!/usr/bin/env python3

for i in range(128):
    if i % 4 == 0:
        print('')
    pi = 4*(i//16) + 32*((3*(i%16//4) + i%4)%4) + i%4;
    print(pi, end=', ')
