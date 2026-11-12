    .text
    .align 2
    .globl bf16_from_f32_asm
    .type bf16_from_f32_asm, @function
bf16_from_f32_asm:
    li t0, 0x7f800000
    and t1, a0, t0
    beq t1, t0, .Lf32_keep
    srli t0, a0, 16
    andi t0, t0, 1
    li t1, 32767
    add a0, a0, t0
    add a0, a0, t1
.Lf32_keep:
    srli a0, a0, 16
    ret
    .size bf16_from_f32_asm, .-bf16_from_f32_asm

    .align 2
    .globl bf16_mul_asm
    .type bf16_mul_asm, @function
bf16_mul_asm:
    xor t0, a0, a1
    li t5, 32768
    and t0, t0, t5
    srli t1, a0, 7
    andi t1, t1, 255
    srli t2, a1, 7
    andi t2, t2, 255
    andi t3, a0, 127
    andi t4, a1, 127
    li t5, 255
    beq t1, t5, .Lmul_special_a
    beq t2, t5, .Lmul_special_b
    slli a2, a0, 17
    beqz a2, .Lmul_zero
    slli a2, a1, 17
    beqz a2, .Lmul_zero

    li t5, 128
    bnez t1, .Lmul_a_ready
    li t1, 1
.Lmul_normalize_a:
    slli t3, t3, 1
    addi t1, t1, -1
    bltu t3, t5, .Lmul_normalize_a
.Lmul_a_ready:
    ori t3, t3, 128
    bnez t2, .Lmul_b_ready
    li t2, 1
.Lmul_normalize_b:
    slli t4, t4, 1
    addi t2, t2, -1
    bltu t4, t5, .Lmul_normalize_b
.Lmul_b_ready:
    ori t4, t4, 128

    # Normalization guarantees bit 7, including normalized subnormals.
    slli t6, t3, 7
    andi t4, t4, 127
    beqz t4, .Lmul_product_done
    andi a2, t4, 1
    beqz a2, .Lmul_skip_bit0
    add t6, t6, t3
.Lmul_skip_bit0:
    slli t3, t3, 1
    andi a2, t4, 2
    beqz a2, .Lmul_skip_bit1
    add t6, t6, t3
.Lmul_skip_bit1:
    slli t3, t3, 1
    andi a2, t4, 4
    beqz a2, .Lmul_skip_bit2
    add t6, t6, t3
.Lmul_skip_bit2:
    slli t3, t3, 1
    andi a2, t4, 8
    beqz a2, .Lmul_skip_bit3
    add t6, t6, t3
.Lmul_skip_bit3:
    slli t3, t3, 1
    andi a2, t4, 16
    beqz a2, .Lmul_skip_bit4
    add t6, t6, t3
.Lmul_skip_bit4:
    slli t3, t3, 1
    andi a2, t4, 32
    beqz a2, .Lmul_skip_bit5
    add t6, t6, t3
.Lmul_skip_bit5:
    slli t3, t3, 1
    andi a2, t4, 64
    beqz a2, .Lmul_skip_bit6
    add t6, t6, t3
.Lmul_skip_bit6:
.Lmul_product_done:
    add t1, t1, t2
    addi t1, t1, -127
    srli a2, t6, 15
    beqz a2, .Lmul_fraction
    srli t6, t6, 1
    addi t1, t1, 1
.Lmul_fraction:
    srli t6, t6, 7
    andi t6, t6, 127
    li t5, 255
    bge t1, t5, .Lmul_inf
    blez t1, .Lmul_underflow
    slli t1, t1, 7
    or a0, t0, t1
    or a0, a0, t6
    ret
.Lmul_underflow:
    li t5, -6
    blt t1, t5, .Lmul_zero
    li t5, 1
    sub t5, t5, t1
    srl t6, t6, t5
    or a0, t0, t6
    ret
.Lmul_special_a:
    bnez t3, .Lmul_return
    slli a2, a1, 17
    beqz a2, .Lmul_nan
    j .Lmul_inf
.Lmul_special_b:
    bnez t4, .Lmul_return_b
    slli a2, a0, 17
    beqz a2, .Lmul_nan
.Lmul_inf:
    li a0, 0x7f80
    or a0, a0, t0
    ret
.Lmul_return_b:
    mv a0, a1
.Lmul_return:
    ret
.Lmul_nan:
    li a0, 0x7fc0
    ret
.Lmul_zero:
    mv a0, t0
    ret
    .size bf16_mul_asm, .-bf16_mul_asm

    .align 2
    .globl bf16_sqrt_asm
    .type bf16_sqrt_asm, @function
bf16_sqrt_asm:
    srli t0, a0, 7
    andi t0, t0, 255
    li t2, 255
    beq t0, t2, .Lsqrt_special
    # For zero-extended uint16 input, values above -0 are negative nonzero.
    # -0 itself reaches the exponent-zero return along with +0/subnormals.
    li t2, 0x8000
    bltu t2, a0, .Lsqrt_nan
    beqz t0, .Lsqrt_zero
    # Normal mantissa is 128..255, with only two exponent parities.
    # A 256-byte table stores the exact fraction after the course sqrt rule.
    # Index = raw fraction | ((!exponent_parity) << 7).
    # Raw bit7 is exponent parity; flipping it directly forms the table bank.
    xori t1, a0, 128
    andi t1, t1, 255
    la t2, .Lsqrt_fraction
    add t1, t1, t2
    lbu a0, 0(t1)
    addi t0, t0, 127
    srli t0, t0, 1
    slli t0, t0, 7
    or a0, a0, t0
    ret
.Lsqrt_special:
    andi t1, a0, 127
    bnez t1, .Lsqrt_return
    srli t2, a0, 15
    beqz t2, .Lsqrt_return
.Lsqrt_nan:
    li a0, 0x7fc0
.Lsqrt_return:
    ret
.Lsqrt_zero:
    li a0, 0
    ret
    .size bf16_sqrt_asm, .-bf16_sqrt_asm

    .section .rodata.bf16_sqrt,"a",@progbits
.Lsqrt_fraction:
    # isqrt(((128+fraction) << (7+even_exponent)) + 127) & 127
    .byte 0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7
    .byte 8,8,9,9,10,10,11,11,11,12,12,13,13,14,14,15
    .byte 15,15,16,16,17,17,18,18,19,19,19,20,20,21,21,22
    .byte 22,22,23,23,24,24,25,25,25,26,26,27,27,27,28,28
    .byte 29,29,29,30,30,31,31,31,32,32,33,33,33,34,34,35
    .byte 35,35,36,36,37,37,37,38,38,39,39,39,40,40,40,41
    .byte 41,42,42,42,43,43,43,44,44,45,45,45,46,46,46,47
    .byte 47,47,48,48,49,49,49,50,50,50,51,51,51,52,52,53
    .byte 53,54,54,55,56,56,57,58,58,59,60,60,61,62,62,63
    .byte 64,64,65,66,66,67,68,68,69,70,70,71,72,72,73,74
    .byte 74,75,75,76,77,77,78,79,79,80,80,81,82,82,83,83
    .byte 84,85,85,86,86,87,88,88,89,89,90,91,91,92,92,93
    .byte 93,94,95,95,96,96,97,97,98,99,99,100,100,101,101,102
    .byte 103,103,104,104,105,105,106,106,107,107,108,109,109,110,110,111
    .byte 111,112,112,113,113,114,114,115,115,116,117,117,118,118,119,119
    .byte 120,120,121,121,122,122,123,123,124,124,125,125,126,126,127,127
