#!/usr/bin/env python3
"""Render text SVG evidence from the published official CSV data.

Uses the unchanged official chart implementation. Install matplotlib to rerun:
    python3 docs/evidence/render_charts.py
"""
from pathlib import Path
import csv
import sys
import matplotlib
matplotlib.use('Agg')
matplotlib.rcParams['svg.hashsalt'] = 'arch2026-progs-evidence'
from matplotlib.figure import Figure

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
from cycles import per_call_table, plot

rows = list(csv.DictReader((HERE / 'cycles.csv').open()))
for row in rows:
    for key in ('n', 'cycles', 'overhead', 'instret', 'checksum'):
        row[key] = int(row[key])
    for key in ('per_call', 'per_call_instret'):
        row[key] = float(row[key])
footprints = list(csv.DictReader((HERE / 'footprint.csv').open()))
for row in footprints:
    for key in ('.text', '.rodata', '.data', '.bss'):
        row[key] = int(row[key])

original_savefig = Figure.savefig

def save_svg(self, filename, **kwargs):
    # This benchmark label describes implementation type, not source provenance.
    for axis in self.axes:
        axis.set_title(axis.get_title().replace('hand-written assembly', 'assembly'))
        if Path(filename).stem == 'cycles-per-call':
            # The 45-bar overview is easier to read without crowded value labels.
            # Exact values remain in cycles.csv and cycles.md.
            for label in list(axis.texts):
                label.remove()
    kwargs['format'] = 'svg'
    kwargs['metadata'] = {'Date': None}
    return original_savefig(self, str(Path(filename).with_suffix('.svg')), **kwargs)

Figure.savefig = save_svg
try:
    plot(str(HERE), per_call_table(rows), footprints)
finally:
    Figure.savefig = original_savefig
print('Rendered cycles-per-call.svg, speedup.svg and footprint.svg')
