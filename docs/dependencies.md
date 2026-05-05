Dependencies and recommended versions

Overview

This document lists the key build and external dependency requirements introduced or changed by recent refactors (notably PR #2463).

Minimum/recommended versions

- CMake: minimum 3.20 (Superbuild/CMakeLists.txt)
- Boost: v1.90.0 (Superbuild/BoostExternal.cmake)
- Qt: minimum 5.15.2 (Superbuild/Superbuild.cmake). Superbuild examples reference Qt 6.10/6.11 paths; SCIRun supports Qt 5.15.2 and Qt6—set Qt_PATH accordingly.
- Qwt: wrapper v0.1.1 → Qwt v6.3.0 (Superbuild/QwtExternal.cmake)
- Eigen: 3.4.0 (Superbuild/EigenExternal.cmake)
- GLM: 0.9.9.8 (Superbuild/GLMExternal.cmake)
- spdlog: v1.10.0 (Superbuild/SpdLogExternal.cmake)
- TetGen: v1.6.1 (Superbuild/TetgenExternal.cmake)
- OSPRay: v2.10.1 (Superbuild/OsprayExternal.cmake)
- Zlib: origin/1.3.1 (Superbuild/ZlibExternal.cmake)
- Additional pinned externals: Cleaver2, Freetype, SQLite, Glew, Tny, Teem, LodePng — pinned to internal branches/tags (see Superbuild/*.cmake)


How to find exact versions

1. Inspect Superbuild files in the refactor branch (e.g. Superbuild/, Superbuild/*.cmake, Superbuild/*Config.cmake.in).
2. If using the superbuild, the generated third-party directories and downloaded archives list concrete versions.

Actions for release

- Update this file with exact Boost and Qt versions once verified.
- Add CI jobs testing Linux/macOS with the updated dependencies and ASAN/UBSAN builds.
- Document any breaking ABI or API changes affecting third-party plugins or downstream users.

Reference

- PR: https://github.com/SCIInstitute/SCIRun/pull/2463
