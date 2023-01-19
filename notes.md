# ARM notes

- ARM 64/32-bit architecture families:
    - ARMv8, ARMv9

- ARM 32-bit architecture families:
    - ARMv6, ARMv7, ARMv8, ARMv9

- profiles (suffix to ARMv6{A/R/M})
    - A: application (GPOS with MMU, devices running Linux/Windows/iOS)
    - R: real-time (medical devices, avionics)
    - M: microcontroller (drones, SoCs)

- execution modes with instruction set:
    - 32-bit: AArch32/A32/T32
    - 64-bit: AArch64/A64

-data types:
    - (B)yte: 8 bits
    - (H)alf-word: 16 bits
    - (W)ord: 32 bits
    - (D)oubleword: 64 bits
    - (Q)uadword: 128 bits

- registers:
    - W[0-31]: 32-bit general-purpose
        - subset: X[0-31]: 64-bit general-purpose
    - V/Q[0-31]: 128-bit vector/quad-float
        - subset: D/S/H/B[0-31]
        - vector arithmetic: `FADD V0.2D, V1.2D, V2.2D`
            - treat elements as B/H/W/D
    - WZR/XZR: 32/64-bit zero register
    - SP: 64-bit stack pointer (must be 16 byte aligned on AArch64)

- calling convention:
    - X[0-7]: arguments and return value
    - X[8-18]: temporary registers
    - X[19-28]: callee-saved
    - X29: frame pointer
    - X30: link register (should be saved when calling subroutine)

- condition flags:
    - N: negative
    - Z: zero/equal
    - C: carry/overflow
    - V: overflow notes

- addressing modes:
    - base: [base]
    - base + offset (immediate): [base{, imm}]
    - base + offset (register): [base, Xm{, LSL imm}]
    - base + offset (extended register): [base, Xm, (S|U)XTW{#imm}]
    - pre-indexed: [base, imm]!
    - post-indexed (immediate): [base], imm
    - post-indexed (register): [base], Xm a
    - literal: label
