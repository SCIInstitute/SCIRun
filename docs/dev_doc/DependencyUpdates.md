# Updating SCIRun's external dependencies

SCIRun builds its third-party dependencies through a two-level CMake Superbuild.
Every dependency's **source URL and pinned version** live in a single manifest:

> [`Superbuild/VERSIONS.cmake`](../../Superbuild/VERSIONS.cmake)

Each `Superbuild/*External.cmake` file consumes the manifest variables (for
example `GIT_REPOSITORY ${ZLIB_GIT_URL}` / `GIT_TAG ${ZLIB_GIT_TAG}`) instead of
hard-coding its own repository and tag. **To change a dependency, edit the
manifest — not the individual `*External.cmake` file.**

## Pinning policy (reproducible builds)

Every pin must be an **immutable ref** so a clean build is byte-for-byte
reproducible:

- A **release tag** (`v1.2.3`, `3.4.0`) — preferred when the dependency tracks
  upstream releases.
- A **full 40-character commit SHA** — used for dependencies that track a
  maintenance *branch* of a fork (most `CIBC-Internal/*` repos).

**Do not pin to a branch ref** (`origin/master`, `origin/scirun-5.0.0-beta`).
A branch resolves to whatever its tip is at clone time, so two builds days apart
can silently differ. The manifest records, in each pin's doc string, which
branch a SHA was captured from, e.g. `"SQLite pinned commit (branch
scirun-5.0.0-beta)"`.

## How to update a dependency

1. **Pick the new ref.**
   - Release-tracked dep: choose the new upstream tag.
   - Branch-tracked fork: resolve the branch tip to a SHA —
     ```bash
     git ls-remote https://github.com/CIBC-Internal/<repo>.git refs/heads/<branch>
     ```
2. **Edit the pin** in `Superbuild/VERSIONS.cmake` (the `*_GIT_TAG`, `*_VERSION`,
   or `*_URL` value). Keep the `(branch <name>)` note in the doc string accurate.
3. **Rebuild from a clean build directory** and smoke-test the result
   (see the build instructions in the top-level `CLAUDE.md` / `README`).
4. **Note the change** in the PR description; if it is a notable version jump,
   also update the table in [dependencies.md](dependencies.md).

Because every pin is a cache variable, you can test a candidate version without
editing the file:

```bash
cmake ../Superbuild -DSPDLOG_GIT_TAG=v1.14.1 ...
```

## Automated freshness check

The [`Dependency version check`](../../.github/workflows/dependency-check.yml)
GitHub Actions workflow runs weekly (and on demand) and reports two conditions
via [`Superbuild/scripts/check_dependency_versions.py`](../../Superbuild/scripts/check_dependency_versions.py):

- **UPDATE** — a newer upstream release/tag exists than the one pinned.
- **DRIFT** — the upstream branch a SHA-pin tracks has advanced past our commit.

The job is report-only (it never fails the pipeline). Results appear in the run
summary, and the scheduled run opens or refreshes a single tracking issue titled
*"Dependency updates available"*. Run it locally with:

```bash
python3 Superbuild/scripts/check_dependency_versions.py            # full check
python3 Superbuild/scripts/check_dependency_versions.py --offline-upstream  # drift only, no API calls
```

## What the manifest does *not* cover

Some third-party code is pinned by other mechanisms and is intentionally left
out of `VERSIONS.cmake` (it documents them in a trailing comment block):

- **GoogleTest** — a git submodule (`src/Externals/submodules/googletest`,
  upstream `google/googletest`), pinned by its gitlink SHA in the tree.
- **CTK** — a curated **direct code copy** vendored in-tree at
  `src/Interface/Modules/Base/CustomWidgets/CTK/` and compiled into the
  Interface library; there is no external checkout. (`Superbuild/CtkExternal.cmake`
  is disabled/orphaned.)
- **Qt** — provided by the system or CI toolchain via `find_package`, not built
  by the Superbuild (minimum `SCIRUN_QT_MIN_VERSION` in `Superbuild.cmake`).
- **Test/sample data** — `SCIRunTestData` and `CIBCData` are fetched by
  `TestDataConfig.cmake` / `SCIRunDataExternal.cmake`; these are data fixtures,
  not code dependencies.

### Deprecated Superbuild files

These `*External.cmake` files are **not** wired into the build (no
`ADD_EXTERNAL`) and are not manifest-managed. They have been moved to
[`Superbuild/deprecated/`](../../Superbuild/deprecated/) — retained in case a
special build ever needs them, but not part of the normal build:

- `LibPNGExternal.cmake` — superseded by LodePNG (libpng removed from the build in 2021).
- `GoogleTestExternal.cmake` — superseded by the `googletest` submodule.
- `DataExternal.cmake` — Seg3D sample data, not a SCIRun dependency.
- `CtkExternal.cmake` — disabled; CTK is vendored as an in-tree code copy.

To re-enable one, move it back into `Superbuild/` and add its pin to
`VERSIONS.cmake` plus an `ADD_EXTERNAL` call in `Superbuild.cmake`.
