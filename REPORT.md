# Four Bare-Metal RV32I Applications

This report describes the local implementation and its reproducible validation as of November 8, 2026. The official framework is pinned to `sysprog21/arch2026-progs` commit `fd2d7c17db223807c342e3f210c769d23d1865b1`. All contract headers, tests, runtime sources, build rules and grading scripts remain unchanged. The implementation and report are prepared for publication in [creeper94666/arch2026-progs](https://github.com/creeper94666/arch2026-progs).

| Program | Portable C | Assembly |
|---|---|---|
| Rubik | [C](https://github.com/creeper94666/arch2026-progs/blob/main/src/rubik/rubik.c) | [ASM](https://github.com/creeper94666/arch2026-progs/blob/main/src/rubik/rubik.S) and [search core](https://github.com/creeper94666/arch2026-progs/blob/main/src/rubik/search_core.S) |
| GCD | [C](https://github.com/creeper94666/arch2026-progs/blob/main/src/gcd/gcd.c) | [ASM](https://github.com/creeper94666/arch2026-progs/blob/main/src/gcd/gcd.S) |
| BF16 | [C](https://github.com/creeper94666/arch2026-progs/blob/main/src/bf16/bf16.c) | [ASM](https://github.com/creeper94666/arch2026-progs/blob/main/src/bf16/bf16.S) |
| Fixed | [C](https://github.com/creeper94666/arch2026-progs/blob/main/src/fixed/fixed.c) | [ASM](https://github.com/creeper94666/arch2026-progs/blob/main/src/fixed/fixed.S) |


## Environment and measurement

The target compiler is xPack GNU RISC-V GCC 15.2.0-1, matching the version in the official CI configuration. The macOS ARM64 distribution was checked against the release's SHA256 digest, `6588e8351455fad8aca37551f0e5a5543f3346bfa9a837cf03cbd3bdd4989f8f`. The target flags are the framework's `-march=rv32i_zicsr -mabi=ilp32 -ffreestanding`, with separate O1, O2, O3 and Os builds. Implementation objects contain no counter reads or environment calls.

Actual rv32emu revision `ffb041b` runs the complete official programs in system interpreter mode with ELF loading enabled and the M, A, F and C extensions disabled. Its `cycle` and `instret` counters count executed guest instructions; these results do not represent physical pipeline timing. The runtime provides a 4 KiB stack.

Every benchmark uses identical input arrays for C and assembly and subtracts a loop baseline. The speed requirement compares assembly with the best compiler variant **inside that same ELF**, with two instructions per call of tolerance. A worst-case instruction budget and an average benchmark cost answer different questions and both must pass.

## Rubik: shortest-path search with a bounded text interface

The state consists of seven permutation digits and seven orientation digits. The implementation rejects incorrect length, out-of-range digits, duplicate cubies and an orientation sum not divisible by three before writing output. It returns the exact shortest distance and prints single-space-separated move tokens. Output capacity includes the terminating NUL. An insufficient buffer returns -2 and writes only an empty string when capacity is nonzero; capacity zero performs no output write.

The search is iterative deepening with an explicit state stack rather than recursion. Independent permutation and orientation distances provide an admissible lower bound. Short canonical suffix masks reject redundant move sequences. When at most five moves remain, an exact state-key hash table supplies either the shortest tail or a definite failure for that allowance. A key encodes six corners; permutation uniqueness and the twist-sum invariant determine the seventh corner.

A 24 KiB, three-bank Bloom filter rejects states outside the radius-six set before expanding their children. A positive result only permits further search: the exact lookup and normal search still establish the answer, so Bloom collisions cannot create an incorrect solution. Tables are generated offline; the target still searches the input state.

The assembly API is written explicitly in RV32I. The search core was ported from the previous verified bloom24 experiment: its compiler-derived control frame and the subsequent handwritten lookup, heuristic, rolling-mask, transition and Bloom optimizations are retained. It would be inaccurate to call the entire search core newly handwritten. Standalone startup, printing, ecalls and writable input/status globals were removed. The retained table ordering preserves anchor-relative offsets in the core. The 24 KiB Bloom data is embedded as assembly text constants, so a clean checkout has all build inputs. Replacing the earlier local-file reference produced byte-identical assembly objects and target ELF output.

The API frame is 96 bytes and the search frame is 416 bytes; its helper leaves do not allocate stack. Both frames preserve 16-byte alignment. Sequential parsing validates each input byte once, stopping at an invalid digit or NUL. A capacity of 33 bytes covers any possible 11-move output, so that path can skip the separate text-length pass. Smaller buffers still use exact measurement. The implementation footprint is 123,788 bytes for assembly; the largest C footprint is 129,999 bytes at O3, below the 131,072-byte limit. The latter has only 1,073 bytes of margin, so future table/code changes require another size check.

The official host tests passed 23/23. An additional sanitizer-enabled test checked 1,082 guarded output-capacity and invalid-input calls. A separate actual-rv32emu ABI probe checked all twelve saved registers, gp/tp, full/zero output capacity and invalid-state output preservation over the 59 valid and 13 invalid official states.

A previous standalone ELF was tested over all 3,674,160 legal states. That result is background evidence for the reused search, not a claim that the new official API ELF has been exhaustively rerun over every state.

## Binary GCD

`gcd_u32` and `gcd_u32_asm` implement Stein's algorithm on unsigned 32-bit integers. Zero inputs return the other operand, including gcd(0,0)=0. Both operands lose their common factors of two; each is made odd, ordered by an unsigned comparison, and the smaller is subtracted from the larger. A nonzero odd difference is even, so the next iteration removes at least one factor of two. The final common power of two is restored by a left shift.

This avoids division and remainder and correctly handles the high unsigned bit. The handwritten assembly is a 128-byte leaf with no stack or external call. The nonzero difference of the two odd operands is guaranteed even, so assembly jumps directly to the shifting loop after subtraction. This removes two redundant parity-test instructions per nonterminal subtraction. A conservative function-only instruction bound is 576 for all unsigned input pairs; this is distinct from the observed official performance samples. Binary GCD is useful for ratio reduction and integer arithmetic on processors without hardware division.

## BFloat16

The public interface uses integer bit patterns, avoiding host floating-point behavior and aliasing assumptions. Finite float32 narrowing adds `0x7fff + ((bits>>16)&1)` and retains the high half, implementing nearest-even rounding. Exponent 255 instead preserves the high half directly, as required by this course's contract.

Multiplication extracts sign, exponent and fraction, normalizes subnormal significands, forms an 8-by-8-bit product through shift/add, renormalizes and truncates the fraction. Special-value priority and underflow follow the header exactly, including its nonstandard rules: an infinite first operand can swallow a NaN second operand, and the hidden bit is removed before shifting an underflowing fraction. Replacing these with generic IEEE behavior would fail the assignment.

The portable C square root uses digit-by-digit restoring square root. The contract's truncating condition is:

```text
floor(root*root/128) <= mantissa
iff root*root <= 128*mantissa + 127.
```

Thus the exact integer answer is `isqrt((mantissa<<7)+127)`. The initial assembly loop passed correctness and its absolute budget but failed the official best-C benchmark rule: 70.4 versus 52.6 instructions per call in the comparison ELF. The final assembly stores only the 256 possible fraction/exponent-parity results, using 256 bytes of read-only data. An entry is `isqrt(((128+fraction) << (7+even_exponent))+127)&127`. Special values still use their original branches. This yields exactly the same result for every input rather than an approximation fitted to benchmark inputs.

The final implementation was checked against an independent reference for all 65,536 square-root bit patterns. Its function-only maximum is now 18 instructions. The final index is `(a^128)&255`: raw bit 7 already contains exponent parity, and the lower seven bits are the fraction. Fraction extraction for special values is deferred until that branch. The complete BF16 assembly occupies 548 bytes of code and 256 bytes of read-only data. Multiplication pre-adds the guaranteed leading significand bit, skips the remaining conditional additions when the lower seven bits are zero, and unrolls the other seven conditional adds. BF16 illustrates the tradeoff between precision, storage and arithmetic cost in machine-learning data formats; the assignment's particular special-case rules should not be assumed to match another BF16 implementation.

## Bitwise and Q11.20 fixed-point arithmetic

`op1` forms a sum through XOR and carry-word propagation and overwrites the exact pass count. With b=0 the count is zero; `op1(0xffff,1)` needs 17 passes; `op1(0xffffffff,1)` needs 32. Its assembly retires exactly `4+5*n` instructions, at most 164.

`op2` is unsigned subtraction modulo 2^32, compiled directly to `sub`. The quiz's construction from bitwise adders describes an equivalent arithmetic model; the official header asks for straight-line subtraction. `op3` computes the floor of the exact unsigned average without first overflowing a sum:

```c
return (a & b) + ((a ^ b) >> 1);
```

A stored signed Q11.20 value A represents A/2^20, with real range [-2048,2048-2^-20]. The specified product is:

```text
low32(floor((a*b + 2^19)/2^20)).
```

This rounds ties toward positive infinity and wraps on overflow. It does not saturate. The implementation forms an unsigned 64-bit product in two 32-bit words. If a is negative, subtract unsigned b from the high word; if b is negative, subtract unsigned a. This converts the raw product to the signed product modulo 2^64. Add the half-unit bias with carry and extract bits 20..51. The C return maps the result bits to int32 without an out-of-range signed cast or signed overflow.

The initial assembly also failed the benchmark comparison despite meeting its absolute budget. Updating the low word in place uses the equivalent carry test `new_low < added_low`, eliminating one instruction for each set multiplier bit. The final version processes four multiplier bits per loop. Any padded high zero bits add no term and leave the accumulated product unchanged. Its exact function count is `16+29*ceil(L/4)+4*P+sa+sb`, where L and P are unsigned multiplier bit length and popcount, and sa/sb indicate negative operands. Its global maximum is 378, reached at (-1,-1), including ret but excluding the caller. All Fixed assembly totals 316 code bytes and no data. Eight-bit expansion was slower and was rejected.

Independent validation of the final target ELF covered 230,400 input pairs and 1,843,200 C/ASM calls, checking results, carry pass counts, guards and ABI. Fixed-point arithmetic is useful for scaled sensor values and embedded processing where floating-point hardware is unavailable.

## Measured comparison

The following costs are instructions per call after baseline subtraction. Assembly is taken from the **same ELF as the best C build**. They are averages over the official benchmark arrays, not global worst-case counts.

| Function | Best C | C cost | Assembly cost |
|---|---|---:|---:|
| Rubik solve | O3 | 28,116.83 | 26,062.00 |
| GCD | O1 | 101.625 | 92.75 |
| BF16 from_f32 | O1 | 11.0625 | 11.25 |
| BF16 mul | O2 | 79.125 | 46.6875 |
| BF16 sqrt | O3 | 52.625 | 16.3125 |
| Fixed op1 | O2 | 22.0625 | 22.125 |
| Fixed op2 | O1 | 4.0 | 4.0 |
| Fixed op3 | O1 | 7.0 | 7.0 |
| Fixed q_mul | O1 | 216.6875 | 205.625 |

All costs satisfy the official best-C-plus-two rule. For example, from_f32 is slightly slower than C but remains within the allowed tolerance. The data above come from the final CSV, not from the older standalone experiments.

| Assembly implementation | .text | .rodata | .data | .bss |
|---|---:|---:|---:|---:|
| Rubik | 3,860 | 119,928 | 0 | 0 |
| GCD | 128 | 0 | 0 | 0 |
| BF16 | 548 | 256 | 0 | 0 |
| Fixed | 316 | 0 | 0 | 0 |

The first full official grading run scored 50/100 because the BF16 and Fixed speed comparisons failed; after the two targeted optimizations, the official driver scored 100/100. This is the public local automated score, not a promise about private vectors or manual grading.

## Reproduction and evidence

The checked-in [evidence directory](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/README.md) contains the official comparison CSVs, section sizes, SVG charts, normalized public logs and source hashes. With the prerequisites listed in the official README installed, run these commands from the repository root:

```sh
make toolchain
make test
make sanitize
make dump
make size
make cycles
scripts/grade.sh --submission . --out results/clean
```

The [comparison table](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/cycles.md), [raw benchmark CSV](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/cycles.csv) and [footprint CSV](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/footprint.csv) cover all four compiler levels and assembly. The published evidence is a snapshot; fresh output is generated under results/. The [source hashes](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/source-sha256.json) identify all implementation and contract files used for the snapshot. The [make test log](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/make-test.log) and [clean-harness grading log](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/clean-grade.log) each report 100/100. Absolute local paths in the public log copies are normalized, with original/copy hashes recorded in [log provenance](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/log-provenance.json).

![Official benchmark instructions per call](https://raw.githubusercontent.com/creeper94666/arch2026-progs/main/docs/evidence/cycles-per-call.svg)

![Assembly speedup relative to the best C build](https://raw.githubusercontent.com/creeper94666/arch2026-progs/main/docs/evidence/speedup.svg)

![Implementation section sizes](https://raw.githubusercontent.com/creeper94666/arch2026-progs/main/docs/evidence/footprint.svg)

The charts are text SVGs re-rendered from the published CSVs using [render_charts.py](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/render_charts.py), which reuses the unchanged official plotting code. The underlying measurements are unchanged. The [independent-test summary](https://github.com/creeper94666/arch2026-progs/blob/main/docs/evidence/independent-summary.json) records additional local tests; the large per-input binary records and complete exploratory archives remain local and are not part of this compact repository evidence package. No .env or persistent shell defaults are changed.

The official public grading checks correctness, checksums, RV32I restrictions, symbols, instruction budgets, footprint, benchmark speed, emulator configuration and the failing-test negative control. Additional BF16 validation made 2,113,536 target-model calls across 1,056,768 cases, including exhaustive sqrt, and the expanded Rubik ABI test ran 2,078 guarded capacity/invalid calls on actual rv32emu, in addition to its full-buffer calls. All C sources and private tables remained byte-identical during this assembly tuning. Neither test is presented as a full enumeration of all 2^64 binary operand pairs.

## Submission and independent understanding

The local public tests pass, and this report is prepared for the existing personal fork. The implementation changes are currently uncommitted on the local implement-applications branch. The existing upstream history verifies; implementation Change-Ids and a personal-fork Actions result still need to be established through the official hooks and a push. All GitHub links in this report target the fork's main branch. They become available after the prepared changes reach main through the required linear commit history. Publication to HackMD and completion of the personal CodeTrial recording remain separate submission steps. The compiler-derived portions of the Rubik search core are disclosed above and must not be described as wholly handwritten. The code and this explanation should be reviewed and understood before presenting them as coursework.
