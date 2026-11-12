# Validation evidence

This directory is the compact public evidence for the four RV32I implementations. The published sources are identified by [source-sha256.json](source-sha256.json). The official framework remains at commit `fd2d7c17db223807c342e3f210c769d23d1865b1`; xPack GCC 15.2.0-1 and actual rv32emu revision `ffb041bab9d1513e5329f25f486378778dceed13` produced the target results.

## Results

The [make test log](make-test.log) and [clean-harness grading log](clean-grade.log) each report **100/100**. They cover O1/O2/O3/Os, the official micro-tests, C/ASM checksums, RV32I and symbol restrictions, instruction budgets, the best-C-plus-two comparison and footprint. The clean-harness run includes the host sanitizers. These are local public grading results; the files do not claim a GitHub Actions run or manual review.

| Artifact | Contents |
|---|---|
| [cycles.csv](cycles.csv), [cycles.md](cycles.md) | All benchmark inputs/implementations/compiler-level aggregates |
| [footprint.csv](footprint.csv) | Per-implementation section sizes |
| [cycles-per-call.svg](cycles-per-call.svg) | Benchmark instructions per call |
| [speedup.svg](speedup.svg) | Assembly compared with the best C build in the same ELF |
| [footprint.svg](footprint.svg) | Code/data section comparison |
| [summary.json](summary.json) | Selected optimizations, scope and instruction bounds |
| [independent-summary.json](independent-summary.json) | Additional local operand tests, coverage and counts |
| [rubik-api.log](rubik-api.log) | Guarded capacity and ABI checks over four API variants |
| [log-provenance.json](log-provenance.json) | Original and public-copy log hashes |
| [SHA256SUMS](SHA256SUMS) | Source, report and evidence checksums |

The Rubik assembly embeds its 24 KiB Bloom data as text constants; no external local file is needed to assemble it. This portability correction leaves the assembled object and target ELF unchanged. The CSVs retain the original measurements. Public log copies replace local absolute paths with placeholders and remove terminal colors; scores and test results are unchanged. The three SVGs are re-rendered from these CSVs using the unchanged official plotting functions. The 45-bar overview omits crowded value labels; the CSVs and tables retain exact figures. Their chart title uses the general term assembly; the Rubik source provenance remains disclosed in the report.

The instruction figures are executed guest instructions, not physical pipeline timing. Additional binary-operand testing is sampled rather than a full enumeration of every 2^64 pair. The independent sqrt check covers every 16-bit input. Large per-input records and complete exploratory/model archives remain local; this directory preserves their statistics, while the official tests below can be reproduced from this repository.

## Reproduction

From the repository root, with the prerequisites in the official README installed:

```sh
make toolchain
make test
make sanitize
make cycles
make size
scripts/grade.sh --submission . --out results/clean
shasum -a256 -c docs/evidence/SHA256SUMS
```

Fresh official output is written to results/. To regenerate these text SVGs from the published CSV snapshots, install matplotlib and run:

```sh
python3 docs/evidence/render_charts.py
```

The repository retains its original Git history and harness. New source commits must still go through the official hooks. This package does not claim that implementation commits, fork Actions, HackMD publication or the personal CodeTrial have already been completed.
