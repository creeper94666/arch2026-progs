# uint32_t gcd_u32_asm(uint32_t a, uint32_t b)
# a0 = a/result, a1 = b, t0 = common power of two.
# Leaf RV32I routine: only caller-saved registers, no stack or helper calls.
    .text
    .align 2
    .globl gcd_u32_asm
    .type gcd_u32_asm, @function
gcd_u32_asm:
    beqz a0, .Lreturn_b
    beqz a1, .Lreturn
    li t0, 0

    or t1, a0, a1
    andi t1, t1, 1
    bnez t1, .Lnormalize_a
.Lshared_twos:
    srli a0, a0, 1
    srli a1, a1, 1
    addi t0, t0, 1
    or t1, a0, a1
    andi t1, t1, 1
    beqz t1, .Lshared_twos

.Lnormalize_a:
    andi t1, a0, 1
    bnez t1, .Lnormalize_b
.Lshift_a:
    srli a0, a0, 1
    andi t1, a0, 1
    beqz t1, .Lshift_a

.Lnormalize_b:
    andi t1, a1, 1
    bnez t1, .Lodd_pair
.Lshift_b:
    srli a1, a1, 1
    andi t1, a1, 1
    beqz t1, .Lshift_b

.Lodd_pair:
    bgeu a1, a0, .Lordered
    mv t2, a0
    mv a0, a1
    mv a1, t2
.Lordered:
    sub a1, a1, a0
    # Both operands were odd: a nonzero difference is even.
    bnez a1, .Lshift_b

    sll a0, a0, t0
.Lreturn:
    ret
.Lreturn_b:
    mv a0, a1
    ret
    .size gcd_u32_asm, .-gcd_u32_asm
