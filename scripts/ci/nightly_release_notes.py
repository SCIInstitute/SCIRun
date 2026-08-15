#!/usr/bin/env python3
"""Compose the body of the rolling `nightly` release.

Two workflows (mac.yml and windows.yml) publish to the SAME release on their own
schedules, so this has to merge rather than overwrite: whichever runs second
must not blank out the other platform's table, and must not regenerate an empty
changelog because the first one already moved the `nightly` tag to HEAD.

The merge is driven by HTML comment markers in the existing body:

    <!-- nightly-sha: <full sha> -->     the commit the changelog was cut to
    <!-- assets:macos --> ... <!-- /assets:macos -->
    <!-- changes --> ... <!-- /changes -->

Rules:
  * This run replaces only its OWN platform's asset block. Other platforms are
    carried over verbatim -- their assets are still on the release. Each block
    records the commit it was built from, so a stale one is visible as such.
  * The changelog is recomputed only when this run's commit differs from the
    recorded nightly-sha. Same commit (the usual mac/windows pair) preserves it.

Test offline against a directory of downloaded artifacts:

    python3 scripts/ci/nightly_release_notes.py --dist dist --sha $(git rev-parse HEAD)
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

SHA_MARKER = re.compile(r"<!--\s*nightly-sha:\s*([0-9a-f]{7,40})\s*-->")
REPO_URL = "https://github.com/SCIInstitute/SCIRun"
MAX_COMMITS = 50


def block(name: str, body: str) -> str:
    return f"<!-- {name} -->\n{body}\n<!-- /{name} -->"


def extract(name: str, text: str) -> str | None:
    m = re.search(
        rf"<!--\s*{re.escape(name)}\s*-->\n?(.*?)\n?<!--\s*/{re.escape(name)}\s*-->",
        text,
        re.DOTALL,
    )
    return m.group(1) if m else None


def git_log(base: str, head: str) -> list[str]:
    """One line per commit, newest first. Empty on any failure -- a shallow
    clone or a base that has been GC'd must not break the publish."""
    try:
        out = subprocess.run(
            ["git", "log", "--no-merges", "--pretty=format:%h %s", f"{base}..{head}"],
            capture_output=True, text=True, check=True, timeout=120,
        ).stdout
    except (OSError, subprocess.SubprocessError):
        return []
    return [ln for ln in out.splitlines() if ln.strip()]


def load_infos(dist: Path) -> list[dict]:
    infos = []
    for p in sorted(dist.rglob("build-info.json")):
        try:
            infos.append(json.loads(p.read_text()))
        except (OSError, ValueError) as e:
            print(f"::warning::Unreadable {p}: {e}", file=sys.stderr)
    return infos


def asset_table(infos: list[dict], asset_names: dict[str, str]) -> str:
    rows = [
        "| Installer | Arch | Runs on | Qt | Python |",
        "|---|---|---|---|---|",
    ]
    for i in sorted(infos, key=lambda x: x.get("slug", "")):
        slug = i.get("slug", "?")
        name = asset_names.get(slug, slug)
        floor = i.get("os_floor")
        if i["platform"] == "macos" and floor:
            runs = f"macOS {floor} or newer"
            if i.get("os_floor_exceeds_max"):
                runs += " &#9888;&#65039;"
        elif i["platform"] == "windows":
            runs = "Windows 10/11 x64"
        else:
            runs = i.get("runner_os", "?")
        rows.append(
            "| [`{}`]({}/releases/download/nightly/{}) | {} | {} | {} | {} |".format(
                name, REPO_URL, name, i.get("arch", "?"), runs,
                i.get("qt_version") or "--", "yes" if i.get("python") else "no",
            )
        )

    commits = sorted({i.get("commit", "")[:7] for i in infos if i.get("commit")})
    note = f"\nBuilt from `{', '.join(commits)}`." if commits else ""

    warned = [i for i in infos if i.get("os_floor_exceeds_max")]
    if warned:
        note += (
            "\n\n&#9888;&#65039; These are built on the newest runner image and "
            "inherit its minimum OS, so they will not launch on older macOS "
            "releases that SCIRun otherwise supports."
        )
    return "\n".join(rows) + note


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dist", type=Path, default=Path("dist"))
    ap.add_argument("--sha", required=True)
    ap.add_argument("--existing-body", type=Path)
    ap.add_argument("--asset-names", type=Path,
                    help="JSON map of slug -> final asset filename")
    args = ap.parse_args()

    existing = ""
    if args.existing_body and args.existing_body.exists():
        existing = args.existing_body.read_text()

    asset_names: dict[str, str] = {}
    if args.asset_names and args.asset_names.exists():
        asset_names = json.loads(args.asset_names.read_text())

    infos = load_infos(args.dist)
    if not infos:
        print("::warning::No build-info.json found; asset table will be omitted",
              file=sys.stderr)

    # --- per-platform asset blocks -----------------------------------------
    platforms = sorted({i["platform"] for i in infos})
    blocks: list[str] = []
    for plat in ("macos", "windows", "linux"):
        if plat in platforms:
            mine = [i for i in infos if i["platform"] == plat]
            blocks.append(block(f"assets:{plat}", asset_table(mine, asset_names)))
        else:
            carried = extract(f"assets:{plat}", existing)
            if carried:
                blocks.append(block(f"assets:{plat}", carried))

    # --- changelog ----------------------------------------------------------
    prev = SHA_MARKER.search(existing)
    prev_sha = prev.group(1) if prev else ""

    if prev_sha == args.sha:
        # The other platform already cut the changelog for this commit.
        changes = extract("changes", existing) or "_No changes recorded._"
    elif prev_sha:
        lines = git_log(prev_sha, args.sha)
        if lines:
            # A missed nightly (or a long-stale tag) can otherwise dump hundreds
            # of commits into the body; the compare link covers the remainder.
            shown, overflow = lines[:MAX_COMMITS], len(lines) - MAX_COMMITS
            changes = "\n".join(
                "- " + re.sub(r"^([0-9a-f]{7,40}) ", rf"[`\1`]({REPO_URL}/commit/\1) ", ln)
                for ln in shown
            )
            if overflow > 0:
                changes += f"\n- _...and {overflow} more; see the full compare link above._"
            changes = (
                f"[`{prev_sha[:7]}`...`{args.sha[:7]}`]"
                f"({REPO_URL}/compare/{prev_sha}...{args.sha}) "
                f"&mdash; {len(lines)} commit{'s' if len(lines) != 1 else ''}\n\n"
                + changes
            )
        else:
            changes = "_No new commits since the previous nightly._"
    else:
        changes = "_First nightly with generated notes; no previous build to compare against._"

    body = "\n\n".join([
        "Automated nightly build of `master`. Asset URLs are stable &mdash; each "
        "run overwrites them in place.",
        f"<!-- nightly-sha: {args.sha} -->",
        "## Downloads",
        *blocks,
        "## Changes since the previous nightly",
        block("changes", changes),
    ])
    print(body)
    return 0


if __name__ == "__main__":
    sys.exit(main())
