.macro swapmove, a, b, mask, shift
    eor     v8.16B, \a\().16B, \b\().16B

.endm

.global gift_64_neon_encrypt, gift_64_neon_decrypt
.text
gift_64_neon_encrypt:
    swapmove v0, v1
    ret

gift_64_neon_decrypt:
    ret
