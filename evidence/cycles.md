# Cycle analysis

- compiler: xpack/bin//riscv-none-elf-gcc 15.2.0, -march=rv32i_zicsr -mabi=ilp32 -ffreestanding
- rv32emu: ffb041b (system emulation, ELF_LOADER, interpreter)
- timed region: n calls on identical inputs; loop baseline subtracted

rv32emu has no timing model: `cycle` and `instret` are one counter of executed guest instructions. The figures below are instruction counts per call, with the loop control, input load and counter reads removed.

## Cycles per call

| Function | -O1 | -O2 | -O3 | -Os | asm | best C | asm vs best C |
|---|---|---|---|---|---|---|---|
| bf16.from_f32 | 11.1 | 11.2 | 12.8 | 11.2 | 11.2 | 11.1 | 0.98x |
| bf16.mul | 79.5 | 79.1 | 80.6 | 79.7 | 46.7 | 79.1 | 1.69x |
| bf16.sqrt | 79.2 | 72.5 | 52.6 | 74.1 | 14.9 | 52.6 | 3.23x |
| fixed.op1 | 24.8 | 22.1 | 24.2 | 24.7 | 22.1 | 22.1 | 1.00x |
| fixed.op2 | 4.0 | 4.0 | 5.9 | 4.0 | 4.0 | 4.0 | 1.00x |
| fixed.op3 | 7.0 | 7.0 | 8.9 | 7.0 | 7.0 | 7.0 | 1.00x |
| fixed.q_mul | 216.7 | 254.6 | 256.6 | 243.9 | 205.6 | 216.7 | 1.05x |
| gcd.gcd | 101.6 | 116.6 | 118.8 | 119.7 | 92.8 | 101.6 | 1.10x |
| rubik.solve | 42645.7 | 40349.3 | 28116.8 | 46605.7 | 26059.5 | 28116.8 | 1.08x |

## Ratio to -O2

| Function | -O1 | -O2 | -O3 | -Os | asm |
|---|---|---|---|---|---|
| bf16.from_f32 | 0.99 | 1.00 | 1.15 | 1.00 | 1.01 |
| bf16.mul | 1.00 | 1.00 | 1.02 | 1.01 | 0.59 |
| bf16.sqrt | 1.09 | 1.00 | 0.73 | 1.02 | 0.21 |
| fixed.op1 | 1.12 | 1.00 | 1.10 | 1.12 | 1.00 |
| fixed.op2 | 1.00 | 1.00 | 1.48 | 1.00 | 1.00 |
| fixed.op3 | 1.00 | 1.00 | 1.28 | 1.00 | 1.00 |
| fixed.q_mul | 0.85 | 1.00 | 1.01 | 0.96 | 0.81 |
| gcd.gcd | 0.87 | 1.00 | 1.02 | 1.03 | 0.80 |
| rubik.solve | 1.06 | 1.00 | 0.70 | 1.16 | 0.65 |

## Footprint (bytes, `size -A`, harness excluded)

| Program | Variant | .text | .rodata | .data | .bss |
|---|---|---|---|---|---|
| rubik | asm | 3860 | 119928 | 0 | 0 |
| rubik | -O1 | 6176 | 119843 | 0 | 0 |
| rubik | -O2 | 5452 | 119831 | 0 | 0 |
| rubik | -O3 | 10152 | 119847 | 0 | 0 |
| rubik | -Os | 5032 | 119835 | 0 | 0 |
| bf16 | asm | 548 | 256 | 0 | 0 |
| bf16 | -O1 | 668 | 0 | 0 | 0 |
| bf16 | -O2 | 612 | 0 | 0 | 0 |
| bf16 | -O3 | 844 | 0 | 0 | 0 |
| bf16 | -Os | 608 | 0 | 0 | 0 |
| gcd | asm | 128 | 0 | 0 | 0 |
| gcd | -O1 | 140 | 0 | 0 | 0 |
| gcd | -O2 | 140 | 0 | 0 | 0 |
| gcd | -O3 | 140 | 0 | 0 | 0 |
| gcd | -Os | 120 | 0 | 0 | 0 |
| fixed | asm | 316 | 0 | 0 | 0 |
| fixed | -O1 | 260 | 0 | 0 | 0 |
| fixed | -O2 | 256 | 0 | 0 | 0 |
| fixed | -O3 | 256 | 0 | 0 | 0 |
| fixed | -Os | 188 | 0 | 0 | 0 |
