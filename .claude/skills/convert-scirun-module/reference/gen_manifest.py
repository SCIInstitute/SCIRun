#!/usr/bin/env python3
"""Regenerate the SCIRun v4->v5 conversion backlog manifest.
Run from repo root:  python3 .claude/skills/convert-scirun-module/reference/gen_manifest.py
Writes backlog_manifest.csv next to this script.
A module is a *dormant* port candidate iff it is commented out in its
Legacy CMakeLists AND its .cc exists on disk."""
import re
import os
import glob
import csv
import subprocess
from collections import Counter

REPO = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], text=True).strip()
os.chdir(REPO)
OUT = '.claude/skills/convert-scirun-module/reference/backlog_manifest.csv'


def loc(path):
    try:
        with open(path, errors='ignore') as f:
            return sum(1 for _ in f)
    except OSError:
        return 0


def find_one(*globs):
    for g in globs:
        hits = glob.glob(g, recursive=True)
        if hits:
            return hits[0]
    return ''


rows = []
for cml in glob.glob('src/Modules/Legacy/**/CMakeLists.txt', recursive=True):
    domain = cml[len('src/Modules/Legacy/'):-len('/CMakeLists.txt')]
    ddir = os.path.dirname(cml)
    for line in open(cml):
        m = re.match(r'\s*#\s*([A-Za-z0-9_]+)\.cc\s*$', line)
        if not m:
            continue
        mod = m.group(1)
        cc = os.path.join(ddir, mod + '.cc')
        present = os.path.exists(cc)
        # algorithm code already copied into tree?
        algo = find_one(f'src/Core/Algorithms/**/{mod}Algo.cc', f'src/Core/Algorithms/**/{mod}Algo.h')
        # v4 dialog / UI hint (rare for dormant, but check)
        ui = find_one(f'src/Interface/Modules/**/{mod}Dialog.cc')
        # config already exists?
        cfg = find_one(f'src/Modules/Factory/Config/{mod}.module')
        n = loc(cc) if present else 0
        diff = 'from-scratch' if not present else ('easy' if n < 200 else 'medium' if n < 450 else 'hard')
        rows.append(dict(domain=domain, module=mod, cc_present=int(present),
                         loc=n, has_legacy_algo=int(bool(algo)),
                         has_ui=int(bool(ui)), has_config=int(bool(cfg)),
                         difficulty=diff, cc_path=(cc if present else '')))

rows.sort(key=lambda r: (r['difficulty'] != 'easy', r['domain'], r['module']))
with open(OUT, 'w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=['domain', 'module', 'difficulty', 'cc_present', 'loc',
                                      'has_legacy_algo', 'has_ui', 'has_config', 'cc_path'],
                       lineterminator='\n')
    w.writeheader()
    w.writerows(rows)

present = [r for r in rows if r['cc_present']]
print(f"wrote {OUT}: {len(rows)} entries, {len(present)} dormant-with-source")
print("difficulty (dormant-with-source):", dict(Counter(r['difficulty'] for r in present)))
print("have legacy algo already:", sum(r['has_legacy_algo'] for r in present))
