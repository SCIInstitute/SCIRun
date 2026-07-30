#!/usr/bin/env python3
"""Rescan the dormant v4 sources and print the frequency-ranked idiom table
that orders translation-table.md. Run from repo root:
    python3 .claude/skills/convert-scirun-module/reference/idiom_scan.py
Reads the dormant set from backlog_manifest.csv (cc_present==1)."""
import re, csv, os, subprocess
from collections import Counter

REPO = subprocess.check_output(['git','rev-parse','--show-toplevel'],text=True).strip()
os.chdir(REPO)
HERE = '.claude/skills/convert-scirun-module/reference'
rows = [r for r in csv.DictReader(open(os.path.join(HERE,'backlog_manifest.csv')))
        if r['cc_present'] == '1']
files = [r['cc_path'] for r in rows]

patterns = {
  'DECLARE_MAKER':                       r'\bDECLARE_MAKER\b',
  'class : public Module':               r'class\s+\w+\s*:\s*public\s+Module',
  'GuiVar/GuiInt/GuiDouble/GuiString':   r'\bGui(Int|Double|String|Var|Context)\b',
  '.get_rep()':                          r'\.get_rep\(\)',
  '->generation':                        r'->generation\b',
  'FieldHandle .detach()':               r'\.detach\(\)',
  'send_output_handle':                  r'\bsend_output_handle\b',
  'get_input_handle':                    r'\bget_input_handle\b',
  'error(/warning(/remark(':             r'\b(error|warning|remark)\s*\(',
  'update_state(':                       r'\bupdate_state\b',
  'gui->/ctx->':                         r'\b(gui|ctx)->',
  'Port headers include':                r'#include\s*<Dataflow/Network/Ports/',
  'TCLInterface/Tcl':                    r'\bTCL|\bTcl',
  'guiVarName.get()/set(':               r'\.(reset|set|get)\(\)\s*;',
  'dynamic_cast<...Field*>':             r'dynamic_cast<',
  'VMesh*/VField*':                      r'\bV(Mesh|Field)\s*\*',
  'FieldInformation':                    r'\bFieldInformation\b',
  'SCIRUN4 marker':                      r'SCIRUN4_',
  'dynamic compilation':                 r'module_dynamic_compile|DynamicCompilation|get_compile_info',
}
counts, filehits = Counter(), Counter()
for f in files:
    txt = open(f, errors='ignore').read()
    for name, pat in patterns.items():
        c = len(re.findall(pat, txt))
        if c:
            counts[name] += c; filehits[name] += 1

print(f"scanned {len(files)} dormant sources\n")
print(f"{'idiom':42s} {'files':>5s} {'total':>6s}")
for name, _ in counts.most_common():
    print(f"{name:42s} {filehits[name]:5d} {counts[name]:6d}")
