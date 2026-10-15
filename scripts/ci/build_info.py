#!/usr/bin/env python3
"""Record what a packaged SCIRun build actually is, and name it accordingly.

Emits build-info.json into the build directory so it travels with the installer
in the same run artifact. publish-release.yml reads it back to compose the
release asset name and the per-asset table in the release notes.

The point is that the OS floor is *measured*, not assumed. Nothing in this repo
sets CMAKE_OSX_DEPLOYMENT_TARGET, so a macOS binary's minimum is whatever the
runner image happened to be -- which is why the nightly SCIRunMacInstaller.pkg
shipped as macOS 26+ arm64-only under a name that implied "the mac build".
Reading LC_BUILD_VERSION out of the binary is the only way to know.

Run offline against any build tree to check what it would publish as:

    python3 scripts/ci/build_info.py --build-dir bin/SCIRun --dry-run
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


def run(cmd: list[str]) -> str:
    """Best-effort capture; a missing tool is not fatal, it just means less info."""
    try:
        return subprocess.run(
            cmd, capture_output=True, text=True, timeout=60
        ).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return ""


# --------------------------------------------------------------------------
# macOS
# --------------------------------------------------------------------------

def find_macho(build_dir: Path) -> Path | None:
    """The bundle main executable, else the unbundled SCIRun/SCIRun_test."""
    for pattern in (
        "**/SCIRun.app/Contents/MacOS/SCIRun",
        "SCIRun",
        "SCIRun_test",
    ):
        for candidate in sorted(build_dir.glob(pattern)):
            if candidate.is_file() and os.access(candidate, os.X_OK):
                return candidate
    return None


def macho_facts(binary: Path) -> dict:
    """minos/sdk from LC_BUILD_VERSION, architectures from lipo."""
    facts: dict = {}

    # vtool prints one platform block per slice; a fat binary repeats them.
    out = run(["vtool", "-show-build", str(binary)])
    if not out:
        out = run(["otool", "-l", str(binary)])
    minos = re.findall(r"^\s*minos\s+(\S+)", out, re.MULTILINE)
    sdk = re.findall(r"^\s*sdk\s+(\S+)", out, re.MULTILINE)
    if minos:
        # Widest reach wins: a fat binary can only run where every slice can.
        facts["os_floor"] = max(minos, key=version_key)
    if sdk:
        facts["sdk"] = max(sdk, key=version_key)

    archs = run(["lipo", "-archs", str(binary)]).split()
    if archs:
        facts["arch"] = "universal" if len(archs) > 1 else archs[0]
        facts["archs"] = archs
    return facts


def version_key(v: str) -> tuple:
    return tuple(int(p) if p.isdigit() else 0 for p in v.split("."))


# --------------------------------------------------------------------------
# Slug
# --------------------------------------------------------------------------

def build_slug(info: dict) -> str:
    """platform[floor]-arch[-flavor...] -- the stable part of the asset name.

    Every axis that distinguishes one installer from another is spelled out, so
    a reader can tell two assets apart without opening them. That is the whole
    job here: the names it replaces ("SCIRunMacInstaller", "...NPInstaller",
    "...-26-intel") encoded the builder image, not what the thing runs on.
    """
    parts: list[str] = []

    if info["platform"] == "macos":
        floor = info.get("os_floor", "")
        # Major only: Apple's minor versions never gate a launch on their own.
        parts.append(("macos" + floor.split(".")[0]) if floor else "macos")
    else:
        parts.append(info["platform"])

    parts.append(info.get("arch") or "unknown")

    qt = info.get("qt_version") or ""
    if qt:
        parts.append("qt" + qt.split(".")[0])
    if info.get("toolset"):
        parts.append(info["toolset"])
    # State it either way rather than leaving the default implicit: mac defaults
    # to python and windows historically did not, so silence means nothing.
    parts.append("python" if info.get("python") else "nopython")

    return "-".join(p for p in parts if p)


# --------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--build-dir", default="bin/SCIRun", type=Path)
    ap.add_argument("--artifact", default="", help="the workflow's artifact-name")
    ap.add_argument("--variant", default="")
    ap.add_argument("--runner-label", default="")
    ap.add_argument("--qt-version", default="")
    ap.add_argument("--generator", default="")
    ap.add_argument("--python", default="")
    ap.add_argument("--commit", default=os.environ.get("GITHUB_SHA", ""))
    ap.add_argument(
        "--max-os-floor",
        default="",
        help="warn if the measured floor is above this (e.g. 15.0). "
        "Warns only -- it must not fail a build it cannot fix.",
    )
    ap.add_argument("--dry-run", action="store_true", help="print, do not write")
    args = ap.parse_args()

    sysname = {"darwin": "macos", "win32": "windows"}.get(sys.platform, "linux")
    cmake_version = run(["cmake", "--version"]).splitlines()

    info: dict = {
        "artifact": args.artifact,
        "platform": sysname,
        "variant": args.variant,
        "runner_label": args.runner_label,
        "qt_version": args.qt_version,
        "python": args.python.lower() in ("true", "1", "yes", "on"),
        "commit": args.commit,
        "built_utc": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "cmake": cmake_version[0] if cmake_version else "",
    }

    if sysname == "macos":
        info["runner_os"] = "macOS {} ({})".format(
            run(["sw_vers", "-productVersion"]), run(["sw_vers", "-buildVersion"])
        )
        binary = find_macho(args.build_dir)
        if binary:
            info["measured_from"] = str(binary.relative_to(args.build_dir))
            info.update(macho_facts(binary))
        else:
            print(f"::warning::No SCIRun executable found under {args.build_dir}; "
                  "OS floor is unverified for this artifact", file=sys.stderr)
    elif sysname == "windows":
        info["runner_os"] = platform.platform()
        info["arch"] = "x64"
        # VS 2026 vs 2022 changes the CRT the installer needs, so it belongs in
        # the name. Derived from the generator rather than passed separately.
        if "18 2026" in args.generator:
            info["toolset"] = "vs2026"
    else:
        info["runner_os"] = platform.platform()
        info["arch"] = platform.machine()

    info["slug"] = build_slug(info)

    floor = info.get("os_floor")
    if floor and args.max_os_floor:
        # Majors only. A macOS minor never gates a launch on its own, and the
        # runner images sit on whatever point release they sit on, so comparing
        # 14.5 against an intended 14.0 would warn on a perfectly good build.
        if version_key(floor)[:1] > version_key(args.max_os_floor)[:1]:
            info["os_floor_exceeds_max"] = True
            info["max_os_floor"] = args.max_os_floor
            print(
                f"::warning title=Installer OS floor too high::{args.artifact or 'build'} "
                f"requires macOS {floor}+, above the intended {args.max_os_floor}. "
                "Users on older supported systems cannot launch it. "
                "Nothing sets CMAKE_OSX_DEPLOYMENT_TARGET, so the floor tracks "
                "whichever runner image built it.",
                file=sys.stderr,
            )

    text = json.dumps(info, indent=2, sort_keys=True)
    print(text)
    if not args.dry_run:
        args.build_dir.mkdir(parents=True, exist_ok=True)
        (args.build_dir / "build-info.json").write_text(text + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
