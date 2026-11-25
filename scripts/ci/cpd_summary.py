#!/usr/bin/env python3
"""Summarize a PMD CPD XML report as Markdown (#2965).

Usage, from the repository root: cpd_summary.py cpd.xml cpd.log
"""

import os
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

NS = {"c": "https://pmd-code.org/schema/cpd-report"}
ISSUES = "https://github.com/SCIInstitute/SCIRun/issues"
TOP = 15


def is_test(path):
    return "/Tests/" in path or path.startswith("src/Testing/")


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    # Trusted input: CPD wrote this file earlier in the same job.
    root = ET.parse(sys.argv[1]).getroot()  # nosec B314
    scanned = len(root.findall("c:file", NS))
    # A wrong path or namespace would otherwise read as "no duplication".
    if scanned == 0:
        sys.exit("error: CPD report contains no scanned files")
    log = Path(sys.argv[2]).read_text(encoding="utf-8", errors="replace")
    skipped = log.count("Skipping file:")

    blocks = []
    for dup in root.findall("c:duplication", NS):
        # The XML renderer always writes absolute paths.
        sites = [(os.path.relpath(f.get("path")), f.get("line"))
                 for f in dup.findall("c:file", NS)]
        blocks.append((int(dup.get("lines")), sites))

    groups = {"Non-test": [], "Test-only": []}
    for block in blocks:
        test_only = all(is_test(path) for path, _ in block[1])
        groups["Test-only" if test_only else "Non-test"].append(block)

    print("## Duplicate code (CPD)\n")
    print("| | Blocks |\n|---|---|")
    for name, members in groups.items():
        print(f"| {name} | {len(members)} |")
    print(f"| **Total** | **{len(blocks)}** |\n")
    print(f"{scanned} files scanned, {skipped} skipped by the lexer. Blocks overlap, "
          "and the scan includes source that is not compiled. "
          f"Cleanup: [#30]({ISSUES}/30) (non-test), [#29]({ISSUES}/29) (tests).")

    for name, members in groups.items():
        print(f"\n### Largest: {name.lower()}\n")
        print("| Lines | Copies | Locations |\n|---|---|---|")
        for lines, sites in sorted(members, key=lambda b: b[0], reverse=True)[:TOP]:
            where = "<br>".join(f"`{path}:{line}`" for path, line in sites)
            print(f"| {lines} | {len(sites)} | {where} |")


if __name__ == "__main__":
    main()
