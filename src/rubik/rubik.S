/* Official bounded-output API. Search and tables are private to search_core.S.
 * 96-byte frame: state[14], moves[11], saved s0/s1/s2/ra. Nested search uses
 * 416 bytes, well within the runtime's 4 KiB stack. No output before validation.
 */
    .text
    .align 2
    .globl rubik_solve_asm
    .type rubik_solve_asm,@function
rubik_solve_asm:
    addi sp,sp,-96
    sw ra,92(sp)
    sw s0,88(sp)
    sw s1,84(sp)
    sw s2,80(sp)
    mv s0,a1
    mv s1,a2
    # Read sequentially: a NUL fails the digit range before any later byte
    # is read. This combines length validation with parsing safely.
    li t0,0
    li t1,0
    li t2,7
.Lapi_permutation:
    add t3,a0,t0
    lbu t4,0(t3)
    addi t4,t4,-49
    bgeu t4,t2,.Lapi_invalid
    li t5,1
    sll t5,t5,t4
    and t6,t1,t5
    bnez t6,.Lapi_invalid
    or t1,t1,t5
    add t5,sp,t0
    sb t4,0(t5)
    addi t0,t0,1
    bne t0,t2,.Lapi_permutation
    li t0,0
    li t2,0
    li t6,3
.Lapi_orientation:
    add t3,a0,t0
    lbu t4,7(t3)
    addi t4,t4,-49
    bgeu t4,t6,.Lapi_invalid
    add t5,sp,t0
    sb t4,7(t5)
    add t2,t2,t4
    addi t0,t0,1
    li t3,7
    bne t0,t3,.Lapi_orientation
    lbu t3,14(a0)
    bnez t3,.Lapi_invalid
    li t3,3
.Lapi_mod3:
    bltu t2,t3,.Lapi_sum
    addi t2,t2,-3
    j .Lapi_mod3
.Lapi_sum:
    bnez t2,.Lapi_invalid
    mv a0,sp
    addi a1,sp,16
    call rubik_asm_solve
    bltz a0,.Lapi_invalid
    mv s2,a0
    # At most 11 two-character moves, ten spaces, and the NUL: 33 bytes.
    la t3,.Lapi_names
    li t0,33
    bgeu s1,t0,.Lapi_write_init
    addi t0,sp,16
    mv t1,s2
    li t2,1
    beqz t1,.Lapi_capacity
.Lapi_measure:
    beq t1,s2,.Lapi_measure_token
    addi t2,t2,1
.Lapi_measure_token:
    lw t4,0(t0)
    addi t4,t4,-1
    slli t5,t4,1
    add t5,t5,t4
    add t5,t3,t5
    lbu t6,1(t5)
    snez t6,t6
    add t2,t2,t6
    addi t2,t2,1
    addi t0,t0,4
    addi t1,t1,-1
    bnez t1,.Lapi_measure
.Lapi_capacity:
    bltu s1,t2,.Lapi_short
.Lapi_write_init:
    addi t0,sp,16
    mv t1,s2
    mv t2,s0
    beqz t1,.Lapi_terminate
.Lapi_write:
    beq t1,s2,.Lapi_write_token
    li t4,32
    sb t4,0(t2)
    addi t2,t2,1
.Lapi_write_token:
    lw t4,0(t0)
    addi t4,t4,-1
    slli t5,t4,1
    add t5,t5,t4
    add t5,t3,t5
    lbu t4,0(t5)
    sb t4,0(t2)
    addi t2,t2,1
    lbu t4,1(t5)
    beqz t4,.Lapi_next
    sb t4,0(t2)
    addi t2,t2,1
.Lapi_next:
    addi t0,t0,4
    addi t1,t1,-1
    bnez t1,.Lapi_write
.Lapi_terminate:
    sb zero,0(t2)
    mv a0,s2
    j .Lapi_return
.Lapi_short:
    beqz s1,.Lapi_short_return
    sb zero,0(s0)
.Lapi_short_return:
    li a0,-2
    j .Lapi_return
.Lapi_invalid:
    li a0,-1
.Lapi_return:
    lw s2,80(sp)
    lw s1,84(sp)
    lw s0,88(sp)
    lw ra,92(sp)
    addi sp,sp,96
    ret
    .size rubik_solve_asm,.-rubik_solve_asm

    .section .rodata.rubik_api,"a",@progbits
.Lapi_names:
    .byte 82,0,0, 82,50,0, 82,39,0
    .byte 66,0,0, 66,50,0, 66,39,0
    .byte 68,0,0, 68,50,0, 68,39,0
