# Dependencies and recommended versions

## Overview

This document lists the key build and external dependency requirements introduced or updated by the CMake/dependency refactor ([PR #2463](https://github.com/SCIInstitute/SCIRun/pull/2463)).

## Minimum/recommended versions

| Dependency | Version | Source |
|---|---|---|
| CMake | 3.20 minimum | `Superbuild/CMakeLists.txt` |
| Boost | v1.90.0 | `Superbuild/BoostExternal.cmake` |
| Qt | 5.15.2 minimum; Qt 6.3.1+ and Qt 6.10 known to work | `Superbuild/Superbuild.cmake` |
| Qwt | wrapper v0.1.1 → Qwt v6.3.0 | `Superbuild/QwtExternal.cmake` |
| Eigen | 3.4.0 | `Superbuild/EigenExternal.cmake` |
| GLM | 0.9.9.8 | `Superbuild/GLMExternal.cmake` |
| spdlog | v1.10.0 | `Superbuild/SpdLogExternal.cmake` |
| TetGen | v1.6.1 | `Superbuild/TetgenExternal.cmake` |
| OSPRay | scirun-build-2.10 | `Superbuild/OsprayExternal.cmake` |
| Zlib | origin/1.3.1 | `Superbuild/ZlibExternal.cmake` |

Additional pinned externals: Cleaver2, Freetype, SQLite, Glew, Tny, Teem, LodePng — pinned to internal fork commits (see the manifest below).

## Single source of truth: `Superbuild/VERSIONS.cmake`

All dependency source URLs and pinned versions now live in one manifest,
[`Superbuild/VERSIONS.cmake`](../../Superbuild/VERSIONS.cmake). Each
`*External.cmake` file consumes those variables instead of hard-coding its own
repository and tag. To change a version, edit the manifest — not the individual
`*External.cmake` files.

For the full update workflow, pinning policy, and the automated freshness check,
see [DependencyUpdates.md](DependencyUpdates.md).

## How to find exact versions

1. Read [`Superbuild/VERSIONS.cmake`](../../Superbuild/VERSIONS.cmake) — every
   pin (a release tag or a full commit SHA) is listed there with the upstream
   branch it was captured from.
2. If using the Superbuild, the generated third-party directories and downloaded
   archives list concrete versions.

## Actions for release

- Add CI jobs testing Linux/macOS with ASAN/UBSAN builds.
- Document any breaking ABI or API changes affecting third-party plugins or downstream users.

## Reference

- PR: https://github.com/SCIInstitute/SCIRun/pull/2463
