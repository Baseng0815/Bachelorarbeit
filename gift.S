.arch armv8a
.cpu cortex-a73

.data

msg:
    .ascii  "Hello, ARM64!\n"
len = . - msg
.skip 64, 0xa

.text
