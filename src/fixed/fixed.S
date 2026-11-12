    .text
    .align 2
    .globl op1_asm
    .type op1_asm, @function
op1_asm:
    li t0, 0
    beqz a1, .Lop1_done
.Lop1_carry:
    and t1, a0, a1
    xor a0, a0, a1
    slli a1, t1, 1
    addi t0, t0, 1
    bnez a1, .Lop1_carry
.Lop1_done:
    sw t0, 0(a2)
    ret
    .size op1_asm, .-op1_asm

    .align 2
    .globl op2_asm
    .type op2_asm, @function
op2_asm:
    sub a0, a0, a1
    ret
    .size op2_asm, .-op2_asm

    .align 2
    .globl op3_asm
    .type op3_asm, @function
op3_asm:
    xor t0, a0, a1
    and a0, a0, a1
    srli t0, t0, 1
    add a0, a0, t0
    ret
    .size op3_asm, .-op3_asm

    .align 2
    .globl q_mul_asm
    .type q_mul_asm, @function
q_mul_asm:
    li t0, 0                 # product low
    li t1, 0                 # product high
    mv t2, a0                # multiplicand low
    li t3, 0                 # multiplicand high
    mv t4, a1                # unsigned multiplier
    beqz t4, .Lq_sign
    # Process 4 bits per loop. Extra high zero bits neither add a term
    # nor change the accumulated product; multiplicand shifts wrap mod 2^64.
.Lq_product:
    andi t5, t4, 1
    beqz t5, .Lq_skip_bit0
    add t0, t0, t2
    sltu t5, t0, t2
    add t1, t1, t3
    add t1, t1, t5
.Lq_skip_bit0:
    srli t5, t2, 31
    slli t3, t3, 1
    or t3, t3, t5
    slli t2, t2, 1
    srli t4, t4, 1
    andi t5, t4, 1
    beqz t5, .Lq_skip_bit1
    add t0, t0, t2
    sltu t5, t0, t2
    add t1, t1, t3
    add t1, t1, t5
.Lq_skip_bit1:
    srli t5, t2, 31
    slli t3, t3, 1
    or t3, t3, t5
    slli t2, t2, 1
    srli t4, t4, 1
    andi t5, t4, 1
    beqz t5, .Lq_skip_bit2
    add t0, t0, t2
    sltu t5, t0, t2
    add t1, t1, t3
    add t1, t1, t5
.Lq_skip_bit2:
    srli t5, t2, 31
    slli t3, t3, 1
    or t3, t3, t5
    slli t2, t2, 1
    srli t4, t4, 1
    andi t5, t4, 1
    beqz t5, .Lq_skip_bit3
    add t0, t0, t2
    sltu t5, t0, t2
    add t1, t1, t3
    add t1, t1, t5
.Lq_skip_bit3:
    srli t5, t2, 31
    slli t3, t3, 1
    or t3, t3, t5
    slli t2, t2, 1
    srli t4, t4, 1
    bnez t4, .Lq_product
.Lq_sign:
    bgez a0, .Lq_a_positive
    sub t1, t1, a1
.Lq_a_positive:
    bgez a1, .Lq_round
    sub t1, t1, a0
.Lq_round:
    li t2, 0x80000           # Q11.20 half-unit, 2^19
    add t2, t0, t2
    sltu t3, t2, t0
    add t1, t1, t3
    srli t2, t2, 20
    slli t1, t1, 12
    or a0, t2, t1
    ret
    .size q_mul_asm, .-q_mul_asm
