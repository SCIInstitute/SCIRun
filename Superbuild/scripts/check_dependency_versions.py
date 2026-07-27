#!/usr/bin/env python3
# For more information, please see: http://software.sci.utah.edu
#
# The MIT License
#
# Copyright (c) 2026 Scientific Computing and Imaging Institute,
# University of Utah.
# (see LICENSE for full text)

"""Check SCIRun's pinned external dependencies for available updates.

Reads Superbuild/VERSIONS.cmake (the single source of truth for dependency
pins) and performs two kinds of check:

  * DRIFT  — for deps pinned to a commit SHA that tracks an upstream *branch*,
             query the current tip of that branch. If the branch has advanced
             past our pinned SHA, our pin is behind the branch we follow.

  * UPDATE — for deps pinned to a release *tag*, query the upstream project's
             latest release/tag and compare. If a newer version exists, flag it.

The branch/tag a pin tracks is read from the doc string of each
sci_dep_version() entry, e.g. "... (branch scirun-5.0.0-beta)".

Network access is required (git ls-remote + GitHub/GitLab REST APIs). The
script is read-only: it never edits the manifest. It prints a Markdown report
to stdout and exits 0 when everything is current, or 1 when at least one
dependency is out of date (so CI can surface it). Pass --no-fail to always
exit 0 (e.g. for a report-only scheduled job).
"""

import argparse
import json
import os
import re
import subprocess
import sys
import urllib.request
import urllib.error

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_MANIFEST = os.path.normpath(os.path.join(HERE, "..", "VERSIONS.cmake"))

# Map each manifest dependency to how it should be checked.
#
#   url_var / tag_var : the sci_dep_version variables holding the repo + pin.
#   upstream          : GitHub "owner/repo" whose latest release/tag is the
#                       canonical version, or "gitlab:<project>" for GitLab,
#                       or None to skip the upstream-release check.
#
# The branch a SHA-pin tracks is derived from the tag_var's doc string, so it
# does not need to be repeated here.
DEPENDENCIES = [
    # name         url_var             tag_var                 upstream
    ("Eigen",     "EIGEN_URL",        "EIGEN_VERSION",        "gitlab:libeigen%2Feigen"),
    ("zlib",      "ZLIB_GIT_URL",     "ZLIB_GIT_TAG",         "madler/zlib"),
    ("LodePNG",   "LODEPNG_GIT_URL",  "LODEPNG_GIT_TAG",      "lvandeve/lodepng"),
    ("FreeType",  "FREETYPE_GIT_URL", "FREETYPE_GIT_TAG",     None),
    ("Teem",      "TEEM_GIT_URL",     "TEEM_GIT_TAG",         None),
    ("SQLite",    "SQLITE_GIT_URL",   "SQLITE_GIT_TAG",       None),
    ("GLM",       "GLM_GIT_URL",      "GLM_GIT_TAG",          "g-truc/glm"),
    ("GLEW",      "GLEW_GIT_URL",     "GLEW_GIT_TAG",         "nigels-com/glew"),
    ("OSPRay",    "OSPRAY_GIT_URL",   "OSPRAY_GIT_TAG",       "RenderKit/ospray"),
    ("Qwt",       "QWT_GIT_URL",      "QWT_WRAPPER_GIT_TAG",  None),
    ("Python",    "PYTHON_GIT_URL",   "PYTHON_VERSION",       "python/cpython"),
    ("Boost",     "BOOST_GIT_URL",    "BOOST_GIT_TAG",        "boostorg/boost"),
    ("TetGen",    "TETGEN_GIT_URL",   "TETGEN_GIT_TAG",       None),
    ("Cleaver2",  "CLEAVER2_GIT_URL", "CLEAVER2_GIT_TAG",     "SCIInstitute/Cleaver2"),
    ("spdlog",    "SPDLOG_GIT_URL",   "SPDLOG_GIT_TAG",       "gabime/spdlog"),
    ("Tny",       "TNY_GIT_URL",      "TNY_GIT_TAG",          None),
]

SHA_RE = re.compile(r"^[0-9a-f]{40}$")
BRANCH_DOC_RE = re.compile(r"\(branch\s+(?P<branch>[^)]+)\)")


def parse_manifest(path):
    """Return {VAR: (value, doc)} for every sci_dep_version() call."""
    text = open(path, encoding="utf-8").read()
    # sci_dep_version(NAME "value" "doc")   — value/doc may be quoted.
    pattern = re.compile(
        r"""sci_dep_version\(\s*
            (?P<var>[A-Z0-9_]+)\s+
            "(?P<value>[^"]*)"\s+
            "(?P<doc>[^"]*)"\s*\)""",
        re.VERBOSE,
    )
    out = {}
    for m in pattern.finditer(text):
        out[m.group("var")] = (m.group("value"), m.group("doc"))
    return out


def http_json(url):
    req = urllib.request.Request(url, headers={"User-Agent": "scirun-dep-check"})
    token = os.environ.get("GITHUB_TOKEN")
    if token and "api.github.com" in url:
        req.add_header("Authorization", f"Bearer {token}")
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.load(resp)


def latest_github(repo):
    """Latest release tag for a GitHub repo, falling back to newest tag."""
    try:
        data = http_json(f"https://api.github.com/repos/{repo}/releases/latest")
        if data.get("tag_name"):
            return data["tag_name"]
    except urllib.error.HTTPError as e:
        if e.code != 404:
            raise
    tags = http_json(f"https://api.github.com/repos/{repo}/tags?per_page=1")
    return tags[0]["name"] if tags else None


def latest_gitlab(project):
    """Newest release tag for a GitLab project id (URL-encoded path)."""
    data = http_json(
        f"https://gitlab.com/api/v4/projects/{project}/repository/tags?per_page=1"
    )
    return data[0]["name"] if data else None


def latest_upstream(upstream):
    if upstream is None:
        return None
    if upstream.startswith("gitlab:"):
        return latest_gitlab(upstream.split(":", 1)[1])
    return latest_github(upstream)


def remote_branch_sha(url, branch):
    """Tip SHA of `branch` in `url`, or None."""
    for ref in (f"refs/heads/{branch}", branch):
        out = subprocess.run(
            ["git", "ls-remote", url, ref],
            capture_output=True, text=True, timeout=60,
        )
        line = out.stdout.strip().splitlines()
        if line:
            return line[0].split()[0]
    return None


def norm(v):
    """Loose version normalization for comparison/display."""
    return v.lstrip("vV").strip() if v else v


def check(manifest, offline_upstream=False):
    """Return list of result dicts, one per dependency."""
    results = []
    for name, url_var, tag_var, upstream in DEPENDENCIES:
        if url_var not in manifest or tag_var not in manifest:
            results.append({"name": name, "status": "SKIP",
                            "detail": "not found in manifest"})
            continue
        url = manifest[url_var][0]
        pin, doc = manifest[tag_var]
        row = {"name": name, "pin": pin, "status": "OK", "detail": ""}

        # --- DRIFT check for SHA pins that track a branch ---------------------
        if SHA_RE.match(pin):
            bm = BRANCH_DOC_RE.search(doc)
            if bm:
                branch = bm.group("branch").strip()
                row["tracks"] = f"branch {branch}"
                try:
                    tip = remote_branch_sha(url, branch)
                except Exception as e:  # noqa: BLE001
                    row["status"], row["detail"] = "ERROR", f"ls-remote failed: {e}"
                    results.append(row); continue
                if tip is None:
                    row["status"], row["detail"] = "ERROR", f"branch '{branch}' not found"
                elif tip != pin:
                    row["status"] = "DRIFT"
                    row["detail"] = f"branch advanced to {tip[:12]} (pinned {pin[:12]})"
                else:
                    row["detail"] = f"up to date with branch tip {tip[:12]}"
            else:
                row["tracks"] = "commit (no tracked branch)"
                row["detail"] = "pinned to fixed commit"
        # --- UPDATE check for tag/version pins --------------------------------
        else:
            row["tracks"] = "release tag"
            if upstream and not offline_upstream:
                try:
                    latest = latest_upstream(upstream)
                except Exception as e:  # noqa: BLE001
                    row["status"], row["detail"] = "ERROR", f"upstream query failed: {e}"
                    results.append(row); continue
                row["latest"] = latest
                if latest and norm(latest) != norm(pin):
                    row["status"] = "UPDATE"
                    row["detail"] = f"upstream latest {latest} (pinned {pin})"
                else:
                    row["detail"] = f"matches upstream {latest}"
            else:
                row["detail"] = "no upstream mapping; pin is a fixed tag"
        results.append(row)
    return results


def render_markdown(results):
    order = {"UPDATE": 0, "DRIFT": 1, "ERROR": 2, "SKIP": 3, "OK": 4}
    emoji = {"OK": "✅", "UPDATE": "⬆️", "DRIFT": "🌱", "ERROR": "⚠️", "SKIP": "➖"}
    rows = sorted(results, key=lambda r: (order.get(r["status"], 9), r["name"]))
    actionable = [r for r in results if r["status"] in ("UPDATE", "DRIFT")]

    lines = ["## SCIRun dependency version check", ""]
    if actionable:
        lines.append(f"**{len(actionable)}** dependency(ies) may need attention.")
    else:
        lines.append("All pinned dependencies are current. 🎉")
    lines += ["", "| Dep | Status | Pinned | Tracks | Detail |",
              "| --- | --- | --- | --- | --- |"]
    for r in rows:
        lines.append("| {name} | {e} {status} | `{pin}` | {tracks} | {detail} |".format(
            e=emoji.get(r["status"], ""),
            name=r["name"], status=r["status"],
            pin=(r.get("pin", "") or "")[:16], tracks=r.get("tracks", ""),
            detail=r.get("detail", "")))
    lines += ["", "_🌱 DRIFT = tracked branch advanced past our pinned commit; "
              "⬆️ UPDATE = newer upstream release exists._"]
    return "\n".join(lines), actionable


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--manifest", default=DEFAULT_MANIFEST,
                    help="path to VERSIONS.cmake")
    ap.add_argument("--no-fail", action="store_true",
                    help="always exit 0, even when updates are found")
    ap.add_argument("--offline-upstream", action="store_true",
                    help="skip upstream release queries (drift checks only)")
    ap.add_argument("--json", action="store_true", help="emit raw JSON results")
    args = ap.parse_args(argv)

    manifest = parse_manifest(args.manifest)
    results = check(manifest, offline_upstream=args.offline_upstream)

    if args.json:
        print(json.dumps(results, indent=2))
    else:
        md, _ = render_markdown(results)
        print(md)

    actionable = [r for r in results if r["status"] in ("UPDATE", "DRIFT")]
    return 0 if (args.no_fail or not actionable) else 1


if __name__ == "__main__":
    sys.exit(main())
